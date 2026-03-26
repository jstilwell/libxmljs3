// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_PI_H_
#define SRC_XML_PI_H_

#include "libxmljs.h"
#include "xml_node.h"

namespace libxmljs {

class XmlProcessingInstruction : public Napi::ObjectWrap<XmlProcessingInstruction>,
                                 public XmlNode {
public:
  using Napi::ObjectWrap<XmlProcessingInstruction>::Unwrap;
  static Napi::FunctionReference constructor;

  XmlProcessingInstruction(const Napi::CallbackInfo& info);

  static void Init(Napi::Env env, Napi::Object exports);
  static Napi::Object New(Napi::Env env, xmlNode *node);

  // XmlNode virtual implementations
  Napi::Object Value_() override { return Napi::ObjectWrap<XmlProcessingInstruction>::Value(); }
  void Ref_() override { Napi::ObjectWrap<XmlProcessingInstruction>::Ref(); }
  void Unref_() override { Napi::ObjectWrap<XmlProcessingInstruction>::Unref(); }
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
  Napi::Value Text(const Napi::CallbackInfo& info);

private:
  void set_name(const char *name);
  Napi::Value get_name(Napi::Env env);
  void set_content(const char *content);
  Napi::Value get_content(Napi::Env env);
};

} // namespace libxmljs

#endif // SRC_XML_PI_H_
