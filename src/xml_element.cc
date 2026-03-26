// Copyright 2009, Squish Tech, LLC.

#include <cstring>

#include "libxmljs.h"

#include "xml_attribute.h"
#include "xml_document.h"
#include "xml_element.h"
#include "xml_xpath_context.h"

namespace libxmljs {

Napi::FunctionReference XmlElement::constructor;

// doc, name, content
XmlElement::XmlElement(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<XmlElement>(info), XmlNode(nullptr) {
  Napi::Env env = info.Env();

  // Called from C++ factory New(env, node) — will be initialized after.
  if (info.Length() == 0) {
    return;
  }

  // Called from JS: new Element(doc, name, content)
  if (info.Length() < 2 || !info[0].IsObject() || !info[1].IsString()) {
    Napi::TypeError::New(env, "Element constructor requires (doc, name[, content])")
        .ThrowAsJavaScriptException();
    return;
  }

  XmlDocument *document = XmlDocument::Unwrap(info[0].As<Napi::Object>());
  if (!document) {
    Napi::TypeError::New(env, "first argument must be a Document")
        .ThrowAsJavaScriptException();
    return;
  }

  std::string name = info[1].As<Napi::String>().Utf8Value();

  const char *content = nullptr;
  std::string content_str;
  if (info.Length() >= 3 && info[2].IsString()) {
    content_str = info[2].As<Napi::String>().Utf8Value();
    content = content_str.c_str();
  }

  xmlChar *encodedContent =
      content
          ? xmlEncodeSpecialChars(document->xml_obj, (const xmlChar *)content)
          : nullptr;
  xmlNode *elem = xmlNewDocNode(document->xml_obj, nullptr,
                                (const xmlChar *)name.c_str(), encodedContent);
  if (encodedContent) {
    xmlFree(encodedContent);
  }

  // Initialize the XmlNode base now that we have the xmlNode*.
  this->xml_obj = elem;
  elem->_private = static_cast<XmlNode *>(this);
  this->ancestor = nullptr;
  if (elem->doc && elem->doc->_private) {
    this->doc = elem->doc;
    static_cast<XmlDocument *>(elem->doc->_private)->Ref();
  }
  this->ref_wrapped_ancestor();

  // Store the document reference on this JS object to prevent GC.
  info.This().As<Napi::Object>().Set("document", info[0]);

  this->attach(info.This().As<Napi::Object>());
}

void XmlElement::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "Element", {
    // XmlNode base methods
    InstanceMethod("doc",          &XmlElement::Doc),
    InstanceMethod("parent",       &XmlElement::Parent),
    InstanceMethod("namespace",    &XmlElement::Namespace),
    InstanceMethod("namespaces",   &XmlElement::Namespaces),
    InstanceMethod("prevSibling",  &XmlElement::PrevSibling),
    InstanceMethod("nextSibling",  &XmlElement::NextSibling),
    InstanceMethod("line",         &XmlElement::LineNumber),
    InstanceMethod("type",         &XmlElement::Type),
    InstanceMethod("remove",       &XmlElement::Remove),
    InstanceMethod("clone",        &XmlElement::Clone),
    InstanceMethod("toString",     &XmlElement::ToString),
    // Own methods
    InstanceMethod("name",         &XmlElement::Name),
    InstanceMethod("_attr",        &XmlElement::Attr),
    InstanceMethod("attrs",        &XmlElement::Attrs),
    InstanceMethod("find",         &XmlElement::Find),
    InstanceMethod("text",         &XmlElement::Text),
    InstanceMethod("path",         &XmlElement::Path),
    InstanceMethod("child",        &XmlElement::Child),
    InstanceMethod("childNodes",   &XmlElement::ChildNodes),
    InstanceMethod("addChild",     &XmlElement::AddChild),
    InstanceMethod("cdata",        &XmlElement::AddCData),
    InstanceMethod("nextElement",  &XmlElement::NextElement),
    InstanceMethod("prevElement",  &XmlElement::PrevElement),
    InstanceMethod("addPrevSibling", &XmlElement::AddPrevSibling),
    InstanceMethod("addNextSibling", &XmlElement::AddNextSibling),
    InstanceMethod("replace",      &XmlElement::Replace),
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("Element", func);
}

Napi::Object XmlElement::New(Napi::Env env, xmlNode *node) {
  if (node->_private) {
    return static_cast<XmlNode *>(node->_private)->Value_();
  }

  Napi::Object obj = constructor.New({});
  XmlElement *elem = Napi::ObjectWrap<XmlElement>::Unwrap(obj);

  // Initialize the XmlNode base.
  elem->xml_obj = node;
  node->_private = static_cast<XmlNode *>(elem);
  elem->ancestor = nullptr;
  if (node->doc && node->doc->_private) {
    elem->doc = node->doc;
    static_cast<XmlDocument *>(node->doc->_private)->Ref();
  }
  elem->ref_wrapped_ancestor();
  elem->attach(obj);

  return obj;
}

Napi::Value XmlElement::Name(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() == 0) {
    return get_name(env);
  }

  std::string name = info[0].As<Napi::String>().Utf8Value();
  set_name(name.c_str());
  return info.This();
}

Napi::Value XmlElement::Attr(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  // getter
  if (info.Length() == 1) {
    std::string name = info[0].As<Napi::String>().Utf8Value();
    return get_attr(env, name.c_str());
  }

  // setter
  std::string name = info[0].As<Napi::String>().Utf8Value();
  std::string value = info[1].As<Napi::String>().Utf8Value();
  set_attr(env, name.c_str(), value.c_str());

  return info.This();
}

Napi::Value XmlElement::Attrs(const Napi::CallbackInfo& info) {
  return get_attrs(info.Env());
}

Napi::Value XmlElement::AddChild(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  XmlNode *child = XmlNode::Unwrap(env, info[0].As<Napi::Object>());
  if (!child) {
    Napi::TypeError::New(env, "argument must be an XmlNode")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  xmlNode *imported_child = import_node(child->xml_obj);
  if (imported_child == nullptr) {
    Napi::Error::New(env, "Could not add child. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  bool will_merge = child_will_merge(imported_child);
  if ((child->xml_obj == imported_child) && will_merge) {
    // merged child will be freed, so ensure we operate on a copy
    imported_child = xmlCopyNode(imported_child, 0);
  }

  add_child(imported_child);

  if (!will_merge && (imported_child->_private != nullptr)) {
    static_cast<XmlNode *>(imported_child->_private)->ref_wrapped_ancestor();
  }

  return info.This();
}

Napi::Value XmlElement::AddCData(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  const char *content = nullptr;
  std::string content_str;
  if (info.Length() >= 1 && info[0].IsString()) {
    content_str = info[0].As<Napi::String>().Utf8Value();
    content = content_str.c_str();
  }

  xmlNode *cdata =
      xmlNewCDataBlock(xml_obj->doc, (const xmlChar *)content,
                       content ? (int)content_str.length() : 0);

  add_cdata(cdata);
  return info.This();
}

Napi::Value XmlElement::Find(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  std::string xpath_str = info[0].As<Napi::String>().Utf8Value();

  XmlXpathContext ctxt(xml_obj);

  if (info.Length() == 2) {
    if (info[1].IsString()) {
      std::string uri = info[1].As<Napi::String>().Utf8Value();
      ctxt.register_ns((const xmlChar *)"xmlns", (const xmlChar *)uri.c_str());
    } else if (info[1].IsObject()) {
      Napi::Object namespaces = info[1].As<Napi::Object>();
      Napi::Array properties = namespaces.GetPropertyNames();
      for (uint32_t i = 0; i < properties.Length(); i++) {
        Napi::Value prop_name = properties.Get(i);
        std::string prefix = prop_name.As<Napi::String>().Utf8Value();
        std::string uri = namespaces.Get(prop_name).As<Napi::String>().Utf8Value();
        ctxt.register_ns((const xmlChar *)prefix.c_str(),
                         (const xmlChar *)uri.c_str());
      }
    }
  }

  return ctxt.evaluate(env, (const xmlChar *)xpath_str.c_str());
}

Napi::Value XmlElement::NextElement(const Napi::CallbackInfo& info) {
  return get_next_element(info.Env());
}

Napi::Value XmlElement::PrevElement(const Napi::CallbackInfo& info) {
  return get_prev_element(info.Env());
}

Napi::Value XmlElement::Text(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() == 0) {
    return get_content(env);
  }

  std::string content = info[0].As<Napi::String>().Utf8Value();
  set_content(content.c_str());
  return info.This();
}

Napi::Value XmlElement::Child(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() != 1 || !info[0].IsNumber()) {
    Napi::Error::New(env, "Bad argument: must provide #child() with a number")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  int32_t idx = info[0].As<Napi::Number>().Int32Value();
  return get_child(env, idx);
}

Napi::Value XmlElement::ChildNodes(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() >= 1 && info[0].IsNumber()) {
    return get_child(env, info[0].As<Napi::Number>().Int32Value());
  }

  return get_child_nodes(env);
}

Napi::Value XmlElement::Path(const Napi::CallbackInfo& info) {
  return get_path(info.Env());
}

Napi::Value XmlElement::AddPrevSibling(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  XmlNode *new_sibling = XmlNode::Unwrap(env, info[0].As<Napi::Object>());
  if (!new_sibling) {
    Napi::TypeError::New(env, "argument must be an XmlNode")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  xmlNode *imported_sibling = import_node(new_sibling->xml_obj);
  if (imported_sibling == nullptr) {
    Napi::Error::New(env, "Could not add sibling. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  add_prev_sibling(imported_sibling);

  if (imported_sibling->_private != nullptr) {
    static_cast<XmlNode *>(imported_sibling->_private)->ref_wrapped_ancestor();
  }

  return info[0];
}

Napi::Value XmlElement::AddNextSibling(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  XmlNode *new_sibling = XmlNode::Unwrap(env, info[0].As<Napi::Object>());
  if (!new_sibling) {
    Napi::TypeError::New(env, "argument must be an XmlNode")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  xmlNode *imported_sibling = import_node(new_sibling->xml_obj);
  if (imported_sibling == nullptr) {
    Napi::Error::New(env, "Could not add sibling. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  add_next_sibling(imported_sibling);

  if (imported_sibling->_private != nullptr) {
    static_cast<XmlNode *>(imported_sibling->_private)->ref_wrapped_ancestor();
  }

  return info[0];
}

Napi::Value XmlElement::Replace(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info[0].IsString()) {
    std::string content = info[0].As<Napi::String>().Utf8Value();
    replace_text(content.c_str());
  } else {
    XmlNode *new_node = XmlNode::Unwrap(env, info[0].As<Napi::Object>());
    if (!new_node) {
      Napi::TypeError::New(env, "argument must be a string or XmlNode")
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    xmlNode *imported_sibling = import_node(new_node->xml_obj);
    if (imported_sibling == nullptr) {
      Napi::Error::New(env, "Could not replace. Failed to copy node to new Document.")
          .ThrowAsJavaScriptException();
      return env.Null();
    }
    replace_element(imported_sibling);
  }

  return info[0];
}

// ---------------------------------------------------------------------------
// Private helper implementations
// ---------------------------------------------------------------------------

void XmlElement::set_name(const char *name) {
  xmlNodeSetName(xml_obj, (const xmlChar *)name);
}

Napi::Value XmlElement::get_name(Napi::Env env) {
  if (xml_obj->name) {
    return Napi::String::New(env, (const char *)xml_obj->name);
  }
  return env.Undefined();
}

// TODO(sprsquish) make these work with namespaces
Napi::Value XmlElement::get_attr(Napi::Env env, const char *name) {
  xmlAttr *attr = xmlHasProp(xml_obj, (const xmlChar *)name);
  if (attr) {
    return XmlAttribute::New(env, attr);
  }
  return env.Null();
}

// TODO(sprsquish) make these work with namespaces
void XmlElement::set_attr(Napi::Env env, const char *name, const char *value) {
  XmlAttribute::New(env, xml_obj, (const xmlChar *)name, (const xmlChar *)value);
}

Napi::Value XmlElement::get_attrs(Napi::Env env) {
  xmlAttr *attr = xml_obj->properties;

  Napi::Array attributes = Napi::Array::New(env);
  if (!attr) {
    return attributes;
  }

  uint32_t i = 0;
  do {
    attributes.Set(i++, XmlAttribute::New(env, attr));
  } while ((attr = attr->next));

  return attributes;
}

void XmlElement::add_cdata(xmlNode *cdata) {
  xmlAddChild(xml_obj, cdata);
}

Napi::Value XmlElement::get_child(Napi::Env env, int32_t idx) {
  xmlNode *child = xml_obj->children;

  int32_t i = 0;
  while (child && i < idx) {
    child = child->next;
    ++i;
  }

  if (!child) {
    return env.Null();
  }

  return XmlNode::New(env, child);
}

Napi::Value XmlElement::get_child_nodes(Napi::Env env) {
  xmlNode *child = xml_obj->children;

  if (!child) {
    return Napi::Array::New(env, 0);
  }

  uint32_t len = 0;
  do {
    ++len;
  } while ((child = child->next));

  Napi::Array children = Napi::Array::New(env, len);
  child = xml_obj->children;

  uint32_t i = 0;
  do {
    children.Set(i++, XmlNode::New(env, child));
  } while ((child = child->next) && i < len);

  return children;
}

Napi::Value XmlElement::get_path(Napi::Env env) {
  xmlChar *path = xmlGetNodePath(xml_obj);
  const char *return_path = path ? reinterpret_cast<char *>(path) : "";
  int str_len = xmlStrlen((const xmlChar *)return_path);
  Napi::String js_obj = Napi::String::New(env, return_path, str_len);
  xmlFree(path);
  return js_obj;
}

void XmlElement::unlink_children() {
  xmlNode *cur = xml_obj->children;
  while (cur != nullptr) {
    xmlNode *next = cur->next;
    if (cur->_private != nullptr) {
      static_cast<XmlNode *>(cur->_private)->unref_wrapped_ancestor();
    }
    xmlUnlinkNode(cur);
    cur = next;
  }
}

void XmlElement::set_content(const char *content) {
  xmlChar *encoded =
      xmlEncodeSpecialChars(xml_obj->doc, (const xmlChar *)content);
  this->unlink_children();
  xmlNodeSetContent(xml_obj, encoded);
  xmlFree(encoded);
}

Napi::Value XmlElement::get_content(Napi::Env env) {
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content) {
    Napi::String ret_content = Napi::String::New(env, (const char *)content);
    xmlFree(content);
    return ret_content;
  }
  return Napi::String::New(env, "");
}

Napi::Value XmlElement::get_next_element(Napi::Env env) {
  xmlNode *sibling = xml_obj->next;
  if (!sibling) {
    return env.Null();
  }

  while (sibling && sibling->type != XML_ELEMENT_NODE) {
    sibling = sibling->next;
  }

  if (sibling) {
    return XmlElement::New(env, sibling);
  }

  return env.Null();
}

Napi::Value XmlElement::get_prev_element(Napi::Env env) {
  xmlNode *sibling = xml_obj->prev;
  if (!sibling) {
    return env.Null();
  }

  while (sibling && sibling->type != XML_ELEMENT_NODE) {
    sibling = sibling->prev;
  }

  if (sibling) {
    return XmlElement::New(env, sibling);
  }

  return env.Null();
}

void XmlElement::replace_element(xmlNode *element) {
  xmlReplaceNode(xml_obj, element);
  if (element->_private != nullptr) {
    XmlNode *node = static_cast<XmlNode *>(element->_private);
    node->ref_wrapped_ancestor();
  }
}

void XmlElement::replace_text(const char *content) {
  xmlNodePtr txt = xmlNewDocText(xml_obj->doc, (const xmlChar *)content);
  xmlReplaceNode(xml_obj, txt);
}

bool XmlElement::child_will_merge(xmlNode *child) {
  return ((child->type == XML_TEXT_NODE) && (xml_obj->last != nullptr) &&
          (xml_obj->last->type == XML_TEXT_NODE) &&
          (xml_obj->last->name == child->name) && (xml_obj->last != child));
}

} // namespace libxmljs
