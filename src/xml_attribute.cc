// Copyright 2009, Squish Tech, LLC.
#include <cassert>

#include "xml_attribute.h"
#include "xml_document.h"
#include "xml_element.h"
#include "xml_namespace.h"

namespace libxmljs {

Napi::FunctionReference XmlAttribute::constructor;

XmlAttribute::XmlAttribute(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<XmlAttribute>(info), XmlNode(nullptr) {
  // Zero-argument form: called from C++ factory. Caller sets xml_obj directly.
  if (info.Length() == 0) return;

  // JS-land construction not supported for attributes.
  Napi::Error::New(info.Env(), "XmlAttribute cannot be constructed from JS directly")
      .ThrowAsJavaScriptException();
}

// static
Napi::Object XmlAttribute::New(Napi::Env env, xmlNode *xml_obj,
                                const xmlChar *name, const xmlChar *value) {
  xmlAttr *attr = xmlSetProp(xml_obj, name, value);
  assert(attr);
  return XmlAttribute::New(env, attr);
}

// static
Napi::Object XmlAttribute::New(Napi::Env env, xmlAttr *attr) {
  assert(attr->type == XML_ATTRIBUTE_NODE);

  if (attr->_private) {
    return static_cast<XmlNode *>(attr->_private)->Value_();
  }

  Napi::Object obj = constructor.New({});
  XmlAttribute *wrapper = Napi::ObjectWrap<XmlAttribute>::Unwrap(obj);
  wrapper->xml_obj = reinterpret_cast<xmlNode *>(attr);
  attr->_private = static_cast<XmlNode *>(wrapper);
  wrapper->ancestor = nullptr;
  if (attr->doc && attr->doc->_private) {
    wrapper->doc = attr->doc;
    static_cast<XmlDocument *>(attr->doc->_private)->Ref();
  }
  wrapper->ref_wrapped_ancestor();
  wrapper->attach(obj);
  return obj;
}

Napi::Value XmlAttribute::Name(const Napi::CallbackInfo& info) {
  return get_name(info.Env());
}

Napi::Value XmlAttribute::GetValue(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  // attr.value('new value')
  if (info.Length() > 0) {
    std::string val = info[0].As<Napi::String>().Utf8Value();
    set_value(val.c_str());
    return Value_();
  }

  // attr.value()
  return get_value(env);
}

Napi::Value XmlAttribute::Node(const Napi::CallbackInfo& info) {
  return get_element(info.Env());
}

Napi::Value XmlAttribute::GetNamespace(const Napi::CallbackInfo& info) {
  return get_attr_namespace(info.Env());
}

Napi::Value XmlAttribute::get_name(Napi::Env env) {
  if (xml_obj->name) {
    return Napi::String::New(env,
        reinterpret_cast<const char *>(xml_obj->name),
        xmlStrlen(xml_obj->name));
  }
  return env.Null();
}

Napi::Value XmlAttribute::get_value(Napi::Env env) {
  xmlChar *value = xmlNodeGetContent(xml_obj);
  if (value != nullptr) {
    Napi::String ret = Napi::String::New(env,
        reinterpret_cast<const char *>(value),
        xmlStrlen(value));
    xmlFree(value);
    return ret;
  }
  return env.Null();
}

void XmlAttribute::set_value(const char *value) {
  xmlAttr *attr = reinterpret_cast<xmlAttr *>(xml_obj);

  if (attr->children)
    xmlFreeNodeList(attr->children);

  attr->children = attr->last = nullptr;

  if (value) {
    xmlChar *buffer;
    xmlNode *tmp;

    // Encode our content
    buffer = xmlEncodeEntitiesReentrant(attr->doc, (const xmlChar *)value);

    attr->children = xmlStringGetNodeList(attr->doc, buffer);
    attr->last = nullptr;
    tmp = attr->children;

    // Loop through the children
    for (tmp = attr->children; tmp; tmp = tmp->next) {
      tmp->parent = reinterpret_cast<xmlNode *>(attr);
      tmp->doc = attr->doc;
      if (tmp->next == nullptr)
        attr->last = tmp;
    }

    // Free up memory
    xmlFree(buffer);
  }
}

Napi::Value XmlAttribute::get_element(Napi::Env env) {
  xmlAttr *attr = reinterpret_cast<xmlAttr *>(xml_obj);
  return XmlElement::New(env, attr->parent);
}

Napi::Value XmlAttribute::get_attr_namespace(Napi::Env env) {
  xmlAttr *attr = reinterpret_cast<xmlAttr *>(xml_obj);
  if (!attr->ns) {
    return env.Null();
  }
  return XmlNamespace::New(env, attr->ns);
}

// static
void XmlAttribute::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "Attribute", {
    // XmlNode base methods
    InstanceMethod("doc",         &XmlAttribute::Doc),
    InstanceMethod("parent",      &XmlAttribute::Parent),
    InstanceMethod("namespace",   &XmlAttribute::GetNamespace),
    InstanceMethod("namespaces",  &XmlAttribute::Namespaces),
    InstanceMethod("prevSibling", &XmlAttribute::PrevSibling),
    InstanceMethod("nextSibling", &XmlAttribute::NextSibling),
    InstanceMethod("line",        &XmlAttribute::LineNumber),
    InstanceMethod("type",        &XmlAttribute::Type),
    InstanceMethod("remove",      &XmlAttribute::Remove),
    InstanceMethod("clone",       &XmlAttribute::Clone),
    InstanceMethod("toString",    &XmlAttribute::ToString),
    // Own methods
    InstanceMethod("name",  &XmlAttribute::Name),
    InstanceMethod("value", &XmlAttribute::GetValue),
    InstanceMethod("node",  &XmlAttribute::Node),
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("Attribute", func);
}

} // namespace libxmljs
