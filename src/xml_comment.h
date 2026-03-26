// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_COMMENT_H_
#define SRC_XML_COMMENT_H_

#include "libxmljs.h"
#include "xml_node.h"

namespace libxmljs {

class XmlComment : public Napi::ObjectWrap<XmlComment>, public XmlNode {
public:
  using Napi::ObjectWrap<XmlComment>::Unwrap;
  static Napi::FunctionReference constructor;

  XmlComment(const Napi::CallbackInfo& info);

  static void Init(Napi::Env env, Napi::Object exports);
  static Napi::Object New(Napi::Env env, xmlNode *node);

  // XmlNode virtual implementations
  Napi::Object Value_() override { return Napi::ObjectWrap<XmlComment>::Value(); }
  void Ref_() override { Napi::ObjectWrap<XmlComment>::Ref(); }
  void Unref_() override { Napi::ObjectWrap<XmlComment>::Unref(); }
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
  Napi::Value Text(const Napi::CallbackInfo& info);

private:
  void set_content(const char *content);
  Napi::Value get_content(Napi::Env env);
};

} // namespace libxmljs

#endif // SRC_XML_COMMENT_H_
