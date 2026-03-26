// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_ATTRIBUTE_H_
#define SRC_XML_ATTRIBUTE_H_

#include "libxmljs.h"
#include "xml_node.h"

namespace libxmljs {

class XmlElement;

class XmlAttribute : public Napi::ObjectWrap<XmlAttribute>, public XmlNode {
public:
  using Napi::ObjectWrap<XmlAttribute>::Unwrap;
  static Napi::FunctionReference constructor;

  XmlAttribute(const Napi::CallbackInfo& info);

  static void Init(Napi::Env env, Napi::Object exports);

  // Factory: create from an existing element+name+value (set/update attr)
  static Napi::Object New(Napi::Env env, xmlNode *xml_obj,
                          const xmlChar *name, const xmlChar *value);

  // Factory: wrap an existing xmlAttr
  static Napi::Object New(Napi::Env env, xmlAttr *attr);

  // XmlNode virtual implementations
  Napi::Object Value_() override { return Napi::ObjectWrap<XmlAttribute>::Value(); }
  void Ref_() override { Napi::ObjectWrap<XmlAttribute>::Ref(); }
  void Unref_() override { Napi::ObjectWrap<XmlAttribute>::Unref(); }
  int refs_() override { return 0; }

  // Delegated XmlNode methods
  Napi::Value Doc(const Napi::CallbackInfo& i) { return Doc_Method(i); }
  Napi::Value Parent(const Napi::CallbackInfo& i) { return Parent_Method(i); }
  Napi::Value Namespace(const Napi::CallbackInfo& i) { return Namespace_Method(i); }
  Napi::Value Namespaces(const Napi::CallbackInfo& i) { return Namespaces_Method(i); }
  Napi::Value PrevSibling(const Napi::CallbackInfo& i) { return PrevSibling_Method(i); }
  Napi::Value NextSibling(const Napi::CallbackInfo& i) { return NextSibling_Method(i); }
  Napi::Value LineNumber(const Napi::CallbackInfo& i) { return LineNumber_Method(i); }
  Napi::Value Type(const Napi::CallbackInfo& i) { return Type_Method(i); }
  Napi::Value ToString(const Napi::CallbackInfo& i) { return ToString_Method(i); }
  Napi::Value Remove(const Napi::CallbackInfo& i) { return Remove_Method(i); }
  Napi::Value Clone(const Napi::CallbackInfo& i) { return Clone_Method(i); }

  // Own methods
  Napi::Value Name(const Napi::CallbackInfo& info);
  Napi::Value GetValue(const Napi::CallbackInfo& info);
  Napi::Value Node(const Napi::CallbackInfo& info);
  Napi::Value GetNamespace(const Napi::CallbackInfo& info);

private:
  Napi::Value get_name(Napi::Env env);
  Napi::Value get_value(Napi::Env env);
  void set_value(const char *value);
  Napi::Value get_element(Napi::Env env);
  Napi::Value get_attr_namespace(Napi::Env env);
};

} // namespace libxmljs

#endif // SRC_XML_ATTRIBUTE_H_
