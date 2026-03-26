// Copyright 2009, Squish Tech, LLC.
#include <cassert>
#include <string>

#include "xml_document.h"
#include "xml_pi.h"

namespace libxmljs {

Napi::FunctionReference XmlProcessingInstruction::constructor;

XmlProcessingInstruction::XmlProcessingInstruction(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<XmlProcessingInstruction>(info), XmlNode(nullptr) {
  // Zero-argument form: called from C++ factory. Caller sets xml_obj directly.
  if (info.Length() == 0) return;

  Napi::Env env = info.Env();

  // doc, name[, content]
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

  if (info.Length() < 2 || !info[1].IsString()) {
    Napi::Error::New(env, "name argument must be of type string")
        .ThrowAsJavaScriptException();
    return;
  }

  if (info.Length() > 2 && !info[2].IsString() &&
      !info[2].IsNull() && !info[2].IsUndefined()) {
    Napi::Error::New(env, "content argument must be of type string")
        .ThrowAsJavaScriptException();
    return;
  }

  XmlDocument *document = XmlDocument::Unwrap(info[0].As<Napi::Object>());
  assert(document);

  std::string name_buf = info[1].As<Napi::String>().Utf8Value();

  const char *content = nullptr;
  std::string content_buf;
  if (info.Length() > 2 && info[2].IsString()) {
    content_buf = info[2].As<Napi::String>().Utf8Value();
    content = content_buf.c_str();
  }

  xmlNode *pi = xmlNewDocPI(document->xml_obj,
                            (const xmlChar *)name_buf.c_str(),
                            (const xmlChar *)content);
  this->xml_obj = pi;
  pi->_private = static_cast<XmlNode *>(this);
  this->ancestor = nullptr;
  if (pi->doc && pi->doc->_private) {
    this->doc = pi->doc;
    static_cast<XmlDocument *>(pi->doc->_private)->Ref();
  }
  this->ref_wrapped_ancestor();
  this->attach(info.This().As<Napi::Object>());

  // Keep a reference to the document JS object to prevent GC
  info.This().As<Napi::Object>().Set("document", info[0]);
}

// static
Napi::Object XmlProcessingInstruction::New(Napi::Env env, xmlNode *node) {
  if (node->_private) {
    return static_cast<XmlNode *>(node->_private)->Value_();
  }

  Napi::Object obj = constructor.New({});
  XmlProcessingInstruction *wrapper =
      Napi::ObjectWrap<XmlProcessingInstruction>::Unwrap(obj);
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

Napi::Value XmlProcessingInstruction::Name(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() == 0) {
    return get_name(env);
  }

  std::string name = info[0].As<Napi::String>().Utf8Value();
  set_name(name.c_str());
  return Value_();
}

Napi::Value XmlProcessingInstruction::Text(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() == 0) {
    return get_content(env);
  }

  std::string content = info[0].As<Napi::String>().Utf8Value();
  set_content(content.c_str());
  return Value_();
}

void XmlProcessingInstruction::set_name(const char *name) {
  xmlNodeSetName(xml_obj, (const xmlChar *)name);
}

Napi::Value XmlProcessingInstruction::get_name(Napi::Env env) {
  if (xml_obj->name) {
    return Napi::String::New(env, reinterpret_cast<const char *>(xml_obj->name));
  }
  return env.Undefined();
}

void XmlProcessingInstruction::set_content(const char *content) {
  xmlNodeSetContent(xml_obj, (const xmlChar *)content);
}

Napi::Value XmlProcessingInstruction::get_content(Napi::Env env) {
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content) {
    Napi::String ret = Napi::String::New(env, reinterpret_cast<const char *>(content));
    xmlFree(content);
    return ret;
  }
  return Napi::String::New(env, "");
}

// static
void XmlProcessingInstruction::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "ProcessingInstruction", {
    // XmlNode base methods
    InstanceMethod("doc",         &XmlProcessingInstruction::Doc),
    InstanceMethod("parent",      &XmlProcessingInstruction::Parent),
    InstanceMethod("namespace",   &XmlProcessingInstruction::Namespace),
    InstanceMethod("namespaces",  &XmlProcessingInstruction::Namespaces),
    InstanceMethod("prevSibling", &XmlProcessingInstruction::PrevSibling),
    InstanceMethod("nextSibling", &XmlProcessingInstruction::NextSibling),
    InstanceMethod("line",        &XmlProcessingInstruction::LineNumber),
    InstanceMethod("type",        &XmlProcessingInstruction::Type),
    InstanceMethod("remove",      &XmlProcessingInstruction::Remove),
    InstanceMethod("clone",       &XmlProcessingInstruction::Clone),
    InstanceMethod("toString",    &XmlProcessingInstruction::ToString),
    // Own methods
    InstanceMethod("name", &XmlProcessingInstruction::Name),
    InstanceMethod("text", &XmlProcessingInstruction::Text),
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("ProcessingInstruction", func);
}

} // namespace libxmljs
