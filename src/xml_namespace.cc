// Copyright 2009, Squish Tech, LLC.

#include <string>

#include "xml_document.h"
#include "xml_namespace.h"
#include "xml_node.h"

namespace libxmljs {

Napi::FunctionReference XmlNamespace::constructor;

// JS constructor: new Namespace(node, prefix, href)
// Also invoked with zero arguments from the C++ factory path (XmlNamespace::New)
// followed immediately by initFromNative().
XmlNamespace::XmlNamespace(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<XmlNamespace>(info), xml_obj(nullptr), context(nullptr) {
  // Zero-argument form: C++ factory path — caller will call initFromNative().
  if (info.Length() == 0) {
    return;
  }

  Napi::Env env = info.Env();

  if (!info[0].IsObject()) {
    Napi::Error::New(env, "You must provide a node to attach this namespace to")
        .ThrowAsJavaScriptException();
    return;
  }

  // Unwrap the XmlNode* from the JS node object.
  // XmlNode::Unwrap reads xml_obj->_private (set by XmlNode::XmlNode())
  // without requiring knowledge of the concrete subclass type.
  Napi::Object node_obj = info[0].As<Napi::Object>();
  XmlNode *node = XmlNode::Unwrap(env, node_obj);
  if (!node) {
    Napi::Error::New(env, "You must provide a valid node object")
        .ThrowAsJavaScriptException();
    return;
  }

  const char *prefix_str = nullptr;
  const char *href_str = nullptr;

  std::string prefix_buf, href_buf;

  if (info.Length() > 1 && info[1].IsString()) {
    prefix_buf = info[1].As<Napi::String>().Utf8Value();
    prefix_str = prefix_buf.c_str();
  }

  if (info.Length() > 2 && info[2].IsString()) {
    href_buf = info[2].As<Napi::String>().Utf8Value();
    href_str = href_buf.c_str();
  }

  if (!href_str) {
    Napi::Error::New(env, "You must provide an href for the namespace")
        .ThrowAsJavaScriptException();
    return;
  }

  xmlNs *ns = xmlNewNs(node->xml_obj,
                       reinterpret_cast<const xmlChar *>(href_str),
                       prefix_str
                           ? reinterpret_cast<const xmlChar *>(prefix_str)
                           : nullptr);

  initFromNative(ns);
}

XmlNamespace::~XmlNamespace() {
  // xml_obj may have been nulled by xmlDeregisterNodeCallback when the xmlNs
  // was freed along with its attached node or document.
  if (xml_obj != nullptr) {
    xml_obj->_private = nullptr;
  }

  // The context pointer is only set if this wrapper incremented the refcount.
  if (this->context != nullptr) {
    if (this->context->_private != nullptr) {
      XmlDocument *doc = static_cast<XmlDocument *>(this->context->_private);
      doc->Unref();
    }
    this->context = nullptr;
  }

  // We do not free the xmlNs here — it may still be part of a document and
  // will be freed when the document is freed.
}

void XmlNamespace::initFromNative(xmlNs *ns) {
  xml_obj = ns;
  xml_obj->_private = this;

  // If a context document is present and wrapped, increment its refcount so
  // that it stays alive for as long as this namespace wrapper is alive.
  if ((xml_obj->context != nullptr) && (xml_obj->context->_private != nullptr)) {
    this->context = xml_obj->context;
    XmlDocument *doc = static_cast<XmlDocument *>(xml_obj->context->_private);
    doc->Ref();
  } else {
    this->context = nullptr;
  }
}

// static
Napi::Object XmlNamespace::New(Napi::Env env, xmlNs *ns) {
  if (ns->_private) {
    // Already has a JS wrapper — return it directly.
    return static_cast<XmlNamespace *>(ns->_private)->Value();
  }

  // Create a new JS wrapper via the zero-argument constructor, then attach
  // the native pointer.
  Napi::Object obj = constructor.New({});
  XmlNamespace *wrapper = XmlNamespace::Unwrap(obj);
  wrapper->initFromNative(ns);
  return obj;
}

Napi::Value XmlNamespace::Href(const Napi::CallbackInfo& info) {
  return get_href(info.Env());
}

Napi::Value XmlNamespace::Prefix(const Napi::CallbackInfo& info) {
  return get_prefix(info.Env());
}

Napi::Value XmlNamespace::get_href(Napi::Env env) {
  if (xml_obj->href) {
    return Napi::String::New(env,
        reinterpret_cast<const char *>(xml_obj->href),
        xmlStrlen(xml_obj->href));
  }
  return env.Null();
}

Napi::Value XmlNamespace::get_prefix(Napi::Env env) {
  if (xml_obj->prefix) {
    return Napi::String::New(env,
        reinterpret_cast<const char *>(xml_obj->prefix),
        xmlStrlen(xml_obj->prefix));
  }
  return env.Null();
}

// static
void XmlNamespace::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "Namespace", {
    InstanceMethod("href",   &XmlNamespace::Href),
    InstanceMethod("prefix", &XmlNamespace::Prefix),
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Namespace", func);
}

} // namespace libxmljs
