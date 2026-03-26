// Copyright 2009, Squish Tech, LLC.
#include <cassert>
#include <cstring>
#include <string>

#include "xml_document.h"
#include "xml_node.h"
#include "xml_text.h"

namespace libxmljs {

Napi::FunctionReference XmlText::constructor;

XmlText::XmlText(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<XmlText>(info), XmlNode(nullptr) {
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

  if (info.Length() < 2 || (!info[1].IsString() && !info[1].IsNull() && !info[1].IsUndefined())) {
    Napi::Error::New(env, "content argument must be of type string")
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

  xmlNode *textNode = xmlNewDocText(document->xml_obj, (const xmlChar *)content);
  this->xml_obj = textNode;
  textNode->_private = static_cast<XmlNode *>(this);
  this->ancestor = nullptr;
  if (textNode->doc && textNode->doc->_private) {
    this->doc = textNode->doc;
    static_cast<XmlDocument *>(textNode->doc->_private)->Ref();
  }
  this->ref_wrapped_ancestor();
  this->attach(info.This().As<Napi::Object>());

  // Keep a reference to the document JS object to prevent GC
  info.This().As<Napi::Object>().Set("document", info[0]);
}

// static
Napi::Object XmlText::New(Napi::Env env, xmlNode *node) {
  if (node->_private) {
    return static_cast<XmlNode *>(node->_private)->Value_();
  }

  Napi::Object obj = constructor.New({});
  XmlText *wrapper = Napi::ObjectWrap<XmlText>::Unwrap(obj);
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

Napi::Value XmlText::Text(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() == 0) {
    return get_content(env);
  }

  std::string content = info[0].As<Napi::String>().Utf8Value();
  set_content(content.c_str());
  return Value_();
}

Napi::Value XmlText::AddPrevSibling(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  XmlNode *new_sibling = XmlNode::Unwrap(env, info[0].As<Napi::Object>());
  assert(new_sibling);

  xmlNode *imported_sibling = this->import_node(new_sibling->xml_obj);
  if (imported_sibling == nullptr) {
    Napi::Error::New(env, "Could not add sibling. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return env.Null();
  } else if ((new_sibling->xml_obj == imported_sibling) &&
             prev_sibling_will_merge(imported_sibling)) {
    imported_sibling = xmlCopyNode(imported_sibling, 0);
  }
  add_prev_sibling(imported_sibling);

  return info[0];
}

Napi::Value XmlText::AddNextSibling(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  XmlNode *new_sibling = XmlNode::Unwrap(env, info[0].As<Napi::Object>());
  assert(new_sibling);

  xmlNode *imported_sibling = this->import_node(new_sibling->xml_obj);
  if (imported_sibling == nullptr) {
    Napi::Error::New(env, "Could not add sibling. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return env.Null();
  } else if ((new_sibling->xml_obj == imported_sibling) &&
             next_sibling_will_merge(imported_sibling)) {
    imported_sibling = xmlCopyNode(imported_sibling, 0);
  }
  add_next_sibling(imported_sibling);

  return info[0];
}

Napi::Value XmlText::Replace(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info[0].IsString()) {
    std::string content = info[0].As<Napi::String>().Utf8Value();
    replace_text(content.c_str());
  } else {
    XmlNode *new_node = XmlNode::Unwrap(env, info[0].As<Napi::Object>());
    assert(new_node);

    xmlNode *imported_sibling = this->import_node(new_node->xml_obj);
    if (imported_sibling == nullptr) {
      Napi::Error::New(env, "Could not replace. Failed to copy node to new Document.")
          .ThrowAsJavaScriptException();
      return env.Null();
    }
    replace_element(imported_sibling);
  }

  return info[0];
}

Napi::Value XmlText::Path(const Napi::CallbackInfo& info) {
  return get_path(info.Env());
}

Napi::Value XmlText::Name(const Napi::CallbackInfo& info) {
  if (info.Length() == 0) {
    return get_name(info.Env());
  }
  return Value_();
}

Napi::Value XmlText::NextElement(const Napi::CallbackInfo& info) {
  return get_next_element(info.Env());
}

Napi::Value XmlText::PrevElement(const Napi::CallbackInfo& info) {
  return get_prev_element(info.Env());
}

void XmlText::set_content(const char *content) {
  xmlChar *encoded =
      xmlEncodeSpecialChars(xml_obj->doc, (const xmlChar *)content);
  xmlNodeSetContent(xml_obj, encoded);
  xmlFree(encoded);
}

Napi::Value XmlText::get_content(Napi::Env env) {
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content) {
    Napi::String ret = Napi::String::New(env, reinterpret_cast<const char *>(content));
    xmlFree(content);
    return ret;
  }
  return Napi::String::New(env, "");
}

Napi::Value XmlText::get_name(Napi::Env env) {
  if (xml_obj->name) {
    return Napi::String::New(env, reinterpret_cast<const char *>(xml_obj->name));
  }
  return env.Undefined();
}

Napi::Value XmlText::get_path(Napi::Env env) {
  xmlChar *path = xmlGetNodePath(xml_obj);
  const char *return_path = path ? reinterpret_cast<char *>(path) : "";
  int str_len = xmlStrlen((const xmlChar *)return_path);
  Napi::String js_obj = Napi::String::New(env, return_path, str_len);
  xmlFree(path);
  return js_obj;
}

Napi::Value XmlText::get_next_element(Napi::Env env) {
  xmlNode *sibling = xml_obj->next;
  if (!sibling) return env.Null();

  while (sibling && sibling->type != XML_ELEMENT_NODE)
    sibling = sibling->next;

  if (sibling) {
    return XmlNode::New(env, sibling);
  }
  return env.Null();
}

Napi::Value XmlText::get_prev_element(Napi::Env env) {
  xmlNode *sibling = xml_obj->prev;
  if (!sibling) return env.Null();

  while (sibling && sibling->type != XML_ELEMENT_NODE) {
    sibling = sibling->prev;
  }

  if (sibling) {
    return XmlNode::New(env, sibling);
  }
  return env.Null();
}

void XmlText::add_prev_sibling(xmlNode *element) {
  xmlAddPrevSibling(xml_obj, element);
}

void XmlText::add_next_sibling(xmlNode *element) {
  xmlAddNextSibling(xml_obj, element);
}

void XmlText::replace_element(xmlNode *element) {
  xmlReplaceNode(xml_obj, element);
}

void XmlText::replace_text(const char *content) {
  xmlNodePtr txt = xmlNewDocText(xml_obj->doc, (const xmlChar *)content);
  xmlReplaceNode(xml_obj, txt);
}

bool XmlText::next_sibling_will_merge(xmlNode *child) {
  return (child->type == XML_TEXT_NODE);
}

bool XmlText::prev_sibling_will_merge(xmlNode *child) {
  return (child->type == XML_TEXT_NODE);
}

// static
void XmlText::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "Text", {
    // XmlNode base methods
    InstanceMethod("doc",         &XmlText::Doc),
    InstanceMethod("parent",      &XmlText::Parent),
    InstanceMethod("namespace",   &XmlText::Namespace),
    InstanceMethod("namespaces",  &XmlText::Namespaces),
    InstanceMethod("prevSibling", &XmlText::PrevSibling),
    InstanceMethod("nextSibling", &XmlText::NextSibling),
    InstanceMethod("line",        &XmlText::LineNumber),
    InstanceMethod("type",        &XmlText::Type),
    InstanceMethod("remove",      &XmlText::Remove),
    InstanceMethod("clone",       &XmlText::Clone),
    InstanceMethod("toString",    &XmlText::ToString),
    // Own methods
    InstanceMethod("nextElement",    &XmlText::NextElement),
    InstanceMethod("prevElement",    &XmlText::PrevElement),
    InstanceMethod("text",           &XmlText::Text),
    InstanceMethod("replace",        &XmlText::Replace),
    InstanceMethod("path",           &XmlText::Path),
    InstanceMethod("name",           &XmlText::Name),
    InstanceMethod("addPrevSibling", &XmlText::AddPrevSibling),
    InstanceMethod("addNextSibling", &XmlText::AddNextSibling),
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("Text", func);
}

} // namespace libxmljs
