// Copyright 2009, Squish Tech, LLC.

#include <cassert>

#include <napi.h>

#include <libxml/parserInternals.h>

#include "libxmljs.h"
#include "xml_sax_parser.h"

// ---------------------------------------------------------------------------
// Helper: recover the SaxParserBase* from a libxml parser context.
// The _private field is set to `this` by initializeContext().
// ---------------------------------------------------------------------------
static libxmljs::SaxParserBase *
LXJS_GET_PARSER_FROM_CONTEXT(void *context) {
  _xmlParserCtxt *the_context = static_cast<_xmlParserCtxt *>(context);
  return static_cast<libxmljs::SaxParserBase *>(the_context->_private);
}

#ifdef WIN32
// https://github.com/json-c/json-c/blob/master/printbuf.c
// Copyright (c) 2004, 2005 Metaparadigm Pte Ltd
static int vasprintf(char **buf, const char *fmt, va_list ap) {
  int chars;
  char *b;

  if (!buf) {
    return -1;
  }

  chars = _vscprintf(fmt, ap) + 1;

  b = (char *)malloc(sizeof(char) * chars);
  if (!b) {
    return -1;
  }

  if ((chars = vsprintf(b, fmt, ap)) < 0) {
    free(b);
  } else {
    *buf = b;
  }

  return chars;
}
#endif /* WIN32 */

namespace libxmljs {

// ---------------------------------------------------------------------------
// SaxParserBase
// ---------------------------------------------------------------------------

SaxParserBase::SaxParserBase() : context_(nullptr), env_(nullptr) {
  xmlSAXHandler tmp = {
      0,                              // internalSubset
      0,                              // isStandalone
      0,                              // hasInternalSubset
      0,                              // hasExternalSubset
      0,                              // resolveEntity
      0,                              // getEntity
      0,                              // entityDecl
      0,                              // notationDecl
      0,                              // attributeDecl
      0,                              // elementDecl
      0,                              // unparsedEntityDecl
      0,                              // setDocumentLocator
      SaxParserBase::start_document,  // startDocument
      SaxParserBase::end_document,    // endDocument
      0,                              // startElement
      0,                              // endElement
      0,                              // reference
      SaxParserBase::characters,      // characters
      0,                              // ignorableWhitespace
      0,                              // processingInstruction
      SaxParserBase::comment,         // comment
      SaxParserBase::warning,         // warning
      SaxParserBase::error,           // error
      0,  // fatalError (unused — error() gets all errors)
      0,  // getParameterEntity
      SaxParserBase::cdata_block,     // cdataBlock
      0,                              // externalSubset
      XML_SAX2_MAGIC,                 // force SAX2
      this,                           // _private
      SaxParserBase::start_element_ns, // startElementNs
      SaxParserBase::end_element_ns,   // endElementNs
      0                               // serror
  };

  sax_handler_ = tmp;
}

SaxParserBase::~SaxParserBase() {
  releaseContext();
}

void SaxParserBase::initializeContext() {
  assert(context_);
  context_->validate = 0;
  context_->_private = this;
}

void SaxParserBase::releaseContext() {
  if (context_) {
    context_->_private = nullptr;
    if (context_->myDoc != nullptr) {
      xmlFreeDoc(context_->myDoc);
      context_->myDoc = nullptr;
    }
    xmlFreeParserCtxt(context_);
    context_ = nullptr;
  }
}

void SaxParserBase::Callback(const char *what, int argc, Napi::Value argv[]) {
  assert(env_ != nullptr);
  Napi::Env env = *env_;

  Napi::Object self = SelfObject();

  // Build the argument list: [eventName, ...argv]
  std::vector<napi_value> args(argc + 1);
  args[0] = Napi::String::New(env, what);
  for (int i = 0; i < argc; ++i) {
    args[i + 1] = argv[i];
  }

  Napi::Value emit_v = self.Get("emit");
  if (!emit_v.IsFunction()) {
    return;
  }

  emit_v.As<Napi::Function>().Call(self, args);
}

void SaxParserBase::parse_string(const char *str, unsigned int size) {
  context_ = xmlCreateMemoryParserCtxt(str, static_cast<int>(size));
  initializeContext();
  context_->replaceEntities = 1;
  xmlSAXHandler *old_sax = context_->sax;
  context_->sax = &sax_handler_;
  xmlParseDocument(context_);
  context_->sax = old_sax;
  releaseContext();
}

void SaxParserBase::initialize_push_parser() {
  context_ = xmlCreatePushParserCtxt(&sax_handler_, nullptr, nullptr, 0, "");
  context_->replaceEntities = 1;
  initializeContext();
}

void SaxParserBase::push(const char *str, unsigned int size, bool terminate) {
  xmlParseChunk(context_, str, static_cast<int>(size), terminate ? 1 : 0);
}

// ---------------------------------------------------------------------------
// SAX callback statics
// ---------------------------------------------------------------------------

void SaxParserBase::start_document(void *context) {
  SaxParserBase *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);
  parser->Callback("startDocument");
}

void SaxParserBase::end_document(void *context) {
  SaxParserBase *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);
  parser->Callback("endDocument");
}

void SaxParserBase::start_element_ns(void *context,
                                     const xmlChar *localname,
                                     const xmlChar *prefix,
                                     const xmlChar *uri,
                                     int nb_namespaces,
                                     const xmlChar **namespaces,
                                     int nb_attributes,
                                     int nb_defaulted,
                                     const xmlChar **attributes) {
  SaxParserBase *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);
  assert(parser->env_ != nullptr);
  Napi::Env env = *parser->env_;

  const int argc = 5;
  Napi::Value argv[argc];

  argv[0] = Napi::String::New(env, (const char *)localname);

  // Build attributes list — each entry is [localname, prefix, URI, value]
  Napi::Array attrList = Napi::Array::New(env, nb_attributes);
  if (attributes) {
    for (int i = 0, j = 0; j < nb_attributes; i += 5, j++) {
      const xmlChar *attrLocal = attributes[i + 0];
      const xmlChar *attrPref  = attributes[i + 1];
      const xmlChar *attrUri   = attributes[i + 2];
      const xmlChar *attrVal   = attributes[i + 3];

      Napi::Array elem = Napi::Array::New(env, 4);
      elem.Set(0u, Napi::String::New(env, (const char *)attrLocal,
                                    xmlStrlen(attrLocal)));
      elem.Set(1u, Napi::String::New(env, (const char *)attrPref,
                                    xmlStrlen(attrPref)));
      elem.Set(2u, Napi::String::New(env, (const char *)attrUri,
                                    xmlStrlen(attrUri)));
      elem.Set(3u, Napi::String::New(env, (const char *)attrVal,
                                    attributes[i + 4] - attrVal));
      attrList.Set((uint32_t)j, elem);
    }
  }
  argv[1] = attrList;

  argv[2] = prefix ? Napi::Value(Napi::String::New(env, (const char *)prefix))
                   : env.Null();

  argv[3] = uri ? Napi::Value(Napi::String::New(env, (const char *)uri))
                : env.Null();

  // Build namespace list — each entry is [prefix, uri]
  Napi::Array nsList = Napi::Array::New(env, nb_namespaces);
  if (namespaces) {
    for (int i = 0, j = 0; j < nb_namespaces; j++) {
      const xmlChar *nsPref = namespaces[i++];
      const xmlChar *nsUri  = namespaces[i++];

      Napi::Array elem = Napi::Array::New(env, 2);
      if (xmlStrlen(nsPref) == 0) {
        elem.Set(0u, env.Null());
      } else {
        elem.Set(0u, Napi::String::New(env, (const char *)nsPref,
                                      xmlStrlen(nsPref)));
      }
      elem.Set(1u, Napi::String::New(env, (const char *)nsUri,
                                    xmlStrlen(nsUri)));
      nsList.Set((uint32_t)j, elem);
    }
  }
  argv[4] = nsList;

  parser->Callback("startElementNS", argc, argv);
}

void SaxParserBase::end_element_ns(void *context,
                                   const xmlChar *localname,
                                   const xmlChar *prefix,
                                   const xmlChar *uri) {
  SaxParserBase *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);
  assert(parser->env_ != nullptr);
  Napi::Env env = *parser->env_;

  Napi::Value argv[3];
  argv[0] = Napi::String::New(env, (const char *)localname);
  argv[1] = prefix ? Napi::Value(Napi::String::New(env, (const char *)prefix))
                   : env.Null();
  argv[2] = uri ? Napi::Value(Napi::String::New(env, (const char *)uri))
                : env.Null();

  parser->Callback("endElementNS", 3, argv);
}

void SaxParserBase::characters(void *context, const xmlChar *ch, int len) {
  SaxParserBase *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);
  assert(parser->env_ != nullptr);
  Napi::Env env = *parser->env_;

  Napi::Value argv[1] = {Napi::String::New(env, (const char *)ch, len)};
  parser->Callback("characters", 1, argv);
}

void SaxParserBase::comment(void *context, const xmlChar *value) {
  SaxParserBase *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);
  assert(parser->env_ != nullptr);
  Napi::Env env = *parser->env_;

  Napi::Value argv[1] = {Napi::String::New(env, (const char *)value)};
  parser->Callback("comment", 1, argv);
}

void SaxParserBase::cdata_block(void *context, const xmlChar *value, int len) {
  SaxParserBase *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);
  assert(parser->env_ != nullptr);
  Napi::Env env = *parser->env_;

  Napi::Value argv[1] = {Napi::String::New(env, (const char *)value, len)};
  parser->Callback("cdata", 1, argv);
}

void SaxParserBase::warning(void *context, const char *msg, ...) {
  SaxParserBase *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);
  assert(parser->env_ != nullptr);
  Napi::Env env = *parser->env_;

  char *message = nullptr;
  va_list args;
  va_start(args, msg);
  if (vasprintf(&message, msg, args) >= 0) {
    Napi::Value argv[1] = {Napi::String::New(env, message)};
    parser->Callback("warning", 1, argv);
  }
  va_end(args);
  free(message);
}

void SaxParserBase::error(void *context, const char *msg, ...) {
  SaxParserBase *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);
  assert(parser->env_ != nullptr);
  Napi::Env env = *parser->env_;

  char *message = nullptr;
  va_list args;
  va_start(args, msg);
  if (vasprintf(&message, msg, args) >= 0) {
    Napi::Value argv[1] = {Napi::String::New(env, message)};
    parser->Callback("error", 1, argv);
  }
  va_end(args);
  free(message);
}

// ---------------------------------------------------------------------------
// XmlSaxParser (pull parser)
// ---------------------------------------------------------------------------

Napi::FunctionReference XmlSaxParser::constructor;

XmlSaxParser::XmlSaxParser(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<XmlSaxParser>(info), SaxParserBase() {
  // The sax_handler_._private was set to `this` (SaxParserBase*) in the
  // SaxParserBase constructor.  That pointer is still valid because both base
  // sub-objects live inside the same derived object.
  sax_handler_._private = static_cast<SaxParserBase *>(this);
}

Napi::Value XmlSaxParser::ParseString(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  LIBXMLJS_ARGUMENT_TYPE_CHECK(info[0], IsString,
                               "Bad Argument: parseString requires a string");

  std::string parsable = info[0].As<Napi::String>().Utf8Value();

  Napi::Env local_env = env;
  env_ = &local_env;
  parse_string(parsable.c_str(), static_cast<unsigned int>(parsable.size()));
  env_ = nullptr;

  return Napi::Boolean::New(env, true);
}

void XmlSaxParser::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "SaxParser", {
    InstanceMethod("parseString", &XmlSaxParser::ParseString),
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("SaxParser", func);
}

// ---------------------------------------------------------------------------
// XmlSaxPushParser (push parser)
// ---------------------------------------------------------------------------

Napi::FunctionReference XmlSaxPushParser::constructor;

XmlSaxPushParser::XmlSaxPushParser(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<XmlSaxPushParser>(info), SaxParserBase() {
  sax_handler_._private = static_cast<SaxParserBase *>(this);
  initialize_push_parser();
}

Napi::Value XmlSaxPushParser::Push(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  LIBXMLJS_ARGUMENT_TYPE_CHECK(info[0], IsString,
                               "Bad Argument: parseString requires a string");

  std::string parsable = info[0].As<Napi::String>().Utf8Value();

  bool terminate = (info.Length() > 1) ? info[1].As<Napi::Boolean>().Value()
                                       : false;

  Napi::Env local_env = env;
  env_ = &local_env;
  push(parsable.c_str(), static_cast<unsigned int>(parsable.size()), terminate);
  env_ = nullptr;

  return Napi::Boolean::New(env, true);
}

void XmlSaxPushParser::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "SaxPushParser", {
    InstanceMethod("push", &XmlSaxPushParser::Push),
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("SaxPushParser", func);
}

} // namespace libxmljs
