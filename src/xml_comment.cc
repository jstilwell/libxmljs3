// Copyright 2009, Squish Tech, LLC.
#include <cassert>
#include <string>

#include "xml_comment.h"
#include "xml_document.h"

namespace libxmljs {

Napi::FunctionReference XmlComment::constructor;

XmlComment::XmlComment(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<XmlComment>(info), XmlNode(nullptr) {
  // Zero-argument form: called from C++ factory. Caller sets xml_obj directly.
  if (info.Length() == 0) return;

  Napi::Env env = info.Env();

  // doc, content
  if (info.Length() < 1 || info[0].IsNull() || info[0].IsUndefined()) {
    Napi::Error::New(env, "document argument required")
        .ThrowAsJavaScriptException();
    return;
  }

  if (!info[0].As<Napi::Object>().InstanceOf(XmlDocument::constructor.Value())) {
    Napi::Error::New(env, "document argument must be an instance of Document")
        .ThrowAsJavaScriptException();
    return;
  }

  XmlDocument *document = XmlDocument::Unwrap(info[0].As<Napi::Object>());
  assert(document);

  const char *content = nullptr;
  std::string content_buf;
  if (info.Length() > 1 && info[1].IsString()) {
    content_buf = info[1].As<Napi::String>().Utf8Value();
    content = content_buf.c_str();
  }

  xmlNode *comm = xmlNewDocComment(document->xml_obj, (const xmlChar *)content);
  this->xml_obj = comm;
  comm->_private = static_cast<XmlNode *>(this);
  this->ancestor = nullptr;
  if (comm->doc && comm->doc->_private) {
    this->doc = comm->doc;
    static_cast<XmlDocument *>(comm->doc->_private)->Ref();
  }
  this->ref_wrapped_ancestor();
  this->attach(info.This().As<Napi::Object>());

  // Keep a reference to the document JS object to prevent GC
  info.This().As<Napi::Object>().Set("document", info[0]);
}

// static
Napi::Object XmlComment::New(Napi::Env env, xmlNode *node) {
  if (node->_private) {
    return static_cast<XmlNode *>(node->_private)->Value_();
  }

  Napi::Object obj = constructor.New({});
  XmlComment *wrapper = Napi::ObjectWrap<XmlComment>::Unwrap(obj);
  wrapper->xml_obj = node;
  node->_private = static_cast<XmlNode *>(wrapper);
  wrapper->ancestor = nullptr;
  if (node->doc && node->doc->_private) {
    wrapper->doc = node->doc;
    static_cast<XmlDocument *>(node->doc->_private)->Ref();
  }
  wrapper->ref_wrapped_ancestor();
  wrapper->attach(obj);
  return obj;
}

Napi::Value XmlComment::Text(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() == 0) {
    return get_content(env);
  }

  std::string content = info[0].As<Napi::String>().Utf8Value();
  set_content(content.c_str());
  return Value_();
}

void XmlComment::set_content(const char *content) {
  xmlNodeSetContent(xml_obj, (const xmlChar *)content);
}

Napi::Value XmlComment::get_content(Napi::Env env) {
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content) {
    Napi::String ret = Napi::String::New(env, reinterpret_cast<const char *>(content));
    xmlFree(content);
    return ret;
  }
  return Napi::String::New(env, "");
}

// static
void XmlComment::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "Comment", {
    // XmlNode base methods
    InstanceMethod("doc",         &XmlComment::Doc),
    InstanceMethod("parent",      &XmlComment::Parent),
    InstanceMethod("namespace",   &XmlComment::Namespace),
    InstanceMethod("namespaces",  &XmlComment::Namespaces),
    InstanceMethod("prevSibling", &XmlComment::PrevSibling),
    InstanceMethod("nextSibling", &XmlComment::NextSibling),
    InstanceMethod("line",        &XmlComment::LineNumber),
    InstanceMethod("type",        &XmlComment::Type),
    InstanceMethod("remove",      &XmlComment::Remove),
    InstanceMethod("clone",       &XmlComment::Clone),
    InstanceMethod("toString",    &XmlComment::ToString),
    // Own methods
    InstanceMethod("text", &XmlComment::Text),
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("Comment", func);
}

} // namespace libxmljs
