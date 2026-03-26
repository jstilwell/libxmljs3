// Copyright 2009, Squish Tech, LLC.

#include <napi.h>

#include <libxml/xmlmemory.h>

#include "libxmljs.h"
#include "xml_document.h"
#include "xml_namespace.h"
#include "xml_node.h"
#include "xml_sax_parser.h"
#include "xml_textwriter.h"

namespace libxmljs {

// ensure destruction at exit time
// v8 doesn't cleanup its resources
LibXMLJS LibXMLJS::instance;

// track how much memory libxml2 is using
int xml_memory_used = 0;

// How often we report memory usage changes back to the JS engine.
const int nan_adjust_external_memory_threshold = 1024 * 1024;

// track how many nodes haven't been freed
int nodeCount = 0;

// Global napi_env stored during module Init so that memory wrappers
// (called from libxml2's allocator hooks) can report memory changes.
// Set to nullptr during module cleanup to guard against post-shutdown calls.
static napi_env global_env = nullptr;

void adjustExternalMemory() {
  const int diff = xmlMemUsed() - xml_memory_used;

  if (abs(diff) > nan_adjust_external_memory_threshold) {
    xml_memory_used += diff;
    if (global_env != nullptr) {
      int64_t adjusted_value = 0;
      napi_adjust_external_memory(global_env, static_cast<int64_t>(diff),
                                  &adjusted_value);
    }
  }
}

// wrapper for xmlMemMalloc to update the engine's knowledge of memory used
// the GC relies on this information
void *xmlMemMallocWrap(size_t size) {
  void *res = xmlMemMalloc(size);

  // no need to update memory if we didn't allocate
  if (!res) {
    return res;
  }

  adjustExternalMemory();
  return res;
}

// wrapper for xmlMemFree to update the engine's knowledge of memory used
// the GC relies on this information
void xmlMemFreeWrap(void *p) {
  xmlMemFree(p);

  // if the JS engine is no longer running, don't try to adjust memory.
  // this happens when the VM is shutting down and cleanup routines for
  // libxml2 are called (freeing memory) but the engine is already offline.
  // trying to adjust after shutdown will result in a fatal error.
  if (global_env == nullptr) {
    return;
  }

  adjustExternalMemory();
}

// wrapper for xmlMemRealloc to update the engine's knowledge of memory used
void *xmlMemReallocWrap(void *ptr, size_t size) {
  void *res = xmlMemRealloc(ptr, size);

  // if realloc fails, no need to update memory state
  if (!res) {
    return res;
  }

  adjustExternalMemory();
  return res;
}

// wrapper for xmlMemoryStrdupWrap to update the engine's knowledge of memory
// used
char *xmlMemoryStrdupWrap(const char *str) {
  char *res = xmlMemoryStrdup(str);

  // if strdup fails, no need to update memory state
  if (!res) {
    return res;
  }

  adjustExternalMemory();
  return res;
}

void deregisterNsList(xmlNs *ns) {
  while (ns != NULL) {
    if (ns->_private != NULL) {
      XmlNamespace *wrapper = static_cast<XmlNamespace *>(ns->_private);
      wrapper->xml_obj = NULL;
      ns->_private = NULL;
    }
    ns = ns->next;
  }
}

void deregisterNodeNamespaces(xmlNode *xml_obj) {
  xmlNs *ns = NULL;
  if ((xml_obj->type == XML_DOCUMENT_NODE) ||
#ifdef LIBXML_DOCB_ENABLED
      (xml_obj->type == XML_DOCB_DOCUMENT_NODE) ||
#endif
      (xml_obj->type == XML_HTML_DOCUMENT_NODE)) {
    ns = reinterpret_cast<xmlDoc *>(xml_obj)->oldNs;
  } else if ((xml_obj->type == XML_ELEMENT_NODE) ||
             (xml_obj->type == XML_XINCLUDE_START) ||
             (xml_obj->type == XML_XINCLUDE_END)) {
    ns = xml_obj->nsDef;
  }
  if (ns != NULL) {
    deregisterNsList(ns);
  }
}

/*
 * Before libxmljs nodes are freed, they are passed to the deregistration
 * callback, (configured by `xmlDeregisterNodeDefault`).
 *
 * In deregistering each node, we update any wrapper (e.g. `XmlElement`,
 * `XmlAttribute`) to ensure that when it is destroyed, it doesn't try to
 * access the freed memory.
 *
 * Because namespaces (`xmlNs`) attached to nodes are also freed and may be
 * wrapped, it is necessary to update any wrappers (`XmlNamespace`) which have
 * been created for attached namespaces.
 */
void xmlDeregisterNodeCallback(xmlNode *xml_obj) {
  nodeCount--;
  deregisterNodeNamespaces(xml_obj);
  if (xml_obj->_private != NULL) {
    static_cast<XmlNode *>(xml_obj->_private)->xml_obj = NULL;
    xml_obj->_private = NULL;
  }
  return;
}

// this is called for any created nodes
void xmlRegisterNodeCallback(xmlNode *xml_obj) { nodeCount++; }

LibXMLJS::LibXMLJS() {
  // set the callback for when a node is created
  xmlRegisterNodeDefault(xmlRegisterNodeCallback);

  // set the callback for when a node is about to be freed
  xmlDeregisterNodeDefault(xmlDeregisterNodeCallback);

  // populated debugMemSize (see xmlmemory.h/c) and makes the call to
  // xmlMemUsed work, this must happen first!
  xmlMemSetup(xmlMemFreeWrap, xmlMemMallocWrap, xmlMemReallocWrap,
              xmlMemoryStrdupWrap);

  // initialize libxml
  LIBXML_TEST_VERSION;

  // initial memory usage
  xml_memory_used = xmlMemUsed();
}

LibXMLJS::~LibXMLJS() { xmlCleanupParser(); }

Napi::Object listFeatures(Napi::Env env) {
  Napi::Object target = Napi::Object::New(env);
#define FEAT(x)                                                                \
  target.Set(#x, Napi::Boolean::New(env, xmlHasFeature(XML_WITH_##x)))
  // See enum xmlFeature in parser.h
  FEAT(THREAD);
  FEAT(TREE);
  FEAT(OUTPUT);
  FEAT(PUSH);
  FEAT(READER);
  FEAT(PATTERN);
  FEAT(WRITER);
  FEAT(SAX1);
  FEAT(FTP);
  FEAT(HTTP);
  FEAT(VALID);
  FEAT(HTML);
  FEAT(LEGACY);
  FEAT(C14N);
  FEAT(CATALOG);
  FEAT(XPATH);
  FEAT(XPTR);
  FEAT(XINCLUDE);
  FEAT(ICONV);
  FEAT(ISO8859X);
  FEAT(UNICODE);
  FEAT(REGEXP);
  FEAT(AUTOMATA);
  FEAT(EXPR);
  FEAT(SCHEMAS);
  FEAT(SCHEMATRON);
  FEAT(MODULES);
  FEAT(DEBUG);
  FEAT(DEBUG_MEM);
  FEAT(DEBUG_RUN);
  FEAT(ZLIB);
  FEAT(ICU);
  FEAT(LZMA);
#undef FEAT
  return target;
}

Napi::Value XmlMemUsed(const Napi::CallbackInfo &info) {
  return Napi::Number::New(info.Env(), xmlMemUsed());
}

Napi::Value XmlNodeCount(const Napi::CallbackInfo &info) {
  return Napi::Number::New(info.Env(), nodeCount);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  // Store the env for use in memory wrapper callbacks.
  // Cleared during module teardown via the cleanup hook below.
  global_env = env;

  // Register a cleanup hook so that global_env is nulled out before the
  // env is torn down, preventing post-shutdown calls to napi_adjust_external_memory.
  napi_add_env_cleanup_hook(env, [](void *) { global_env = nullptr; }, nullptr);

  XmlDocument::Init(env, exports);
  XmlSaxParser::Initialize(env, exports);
  XmlSaxPushParser::Initialize(env, exports);
  XmlTextWriter::Initialize(env, exports);

  exports.Set("libxml_version",
              Napi::String::New(env, LIBXML_DOTTED_VERSION));

  exports.Set("libxml_parser_version",
              Napi::String::New(env, xmlParserVersion));

  exports.Set("libxml_debug_enabled",
              Napi::Boolean::New(env, debugging));

  exports.Set("features", listFeatures(env));

  // self-reference so callers can do require('libxmljs').libxml
  exports.Set("libxml", exports);

  exports.Set("xmlMemUsed",
              Napi::Function::New(env, XmlMemUsed, "xmlMemUsed"));

  exports.Set("xmlNodeCount",
              Napi::Function::New(env, XmlNodeCount, "xmlNodeCount"));

  return exports;
}

} // namespace libxmljs

static Napi::Object InitModule(Napi::Env env, Napi::Object exports) {
  return libxmljs::Init(env, exports);
}

NODE_API_MODULE(xmljs, InitModule)
