// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_ELEMENT_H_
#define SRC_XML_ELEMENT_H_

#include <napi.h>
#include <libxml/tree.h>

#include "xml_node.h"

namespace libxmljs {

class XmlElement : public Napi::ObjectWrap<XmlElement>, public XmlNode {
public:
  using Napi::ObjectWrap<XmlElement>::Unwrap;
  static Napi::FunctionReference constructor;

  XmlElement(const Napi::CallbackInfo& info);

  static void Init(Napi::Env env, Napi::Object exports);
  static Napi::Object New(Napi::Env env, xmlNode *node);

  // Implement XmlNode pure virtual interface
  Napi::Object Value_() override { return Napi::ObjectWrap<XmlElement>::Value(); }
  void Ref_() override { Napi::ObjectWrap<XmlElement>::Ref(); }
  void Unref_() override { Napi::ObjectWrap<XmlElement>::Unref(); }
  int refs_() override { return 0; } // placeholder

  // Own methods
  Napi::Value Name(const Napi::CallbackInfo& info);
  Napi::Value Attr(const Napi::CallbackInfo& info);
  Napi::Value Attrs(const Napi::CallbackInfo& info);
  Napi::Value Find(const Napi::CallbackInfo& info);
  Napi::Value Text(const Napi::CallbackInfo& info);
  Napi::Value Path(const Napi::CallbackInfo& info);
  Napi::Value Child(const Napi::CallbackInfo& info);
  Napi::Value ChildNodes(const Napi::CallbackInfo& info);
  Napi::Value AddChild(const Napi::CallbackInfo& info);
  Napi::Value AddCData(const Napi::CallbackInfo& info);
  Napi::Value NextElement(const Napi::CallbackInfo& info);
  Napi::Value PrevElement(const Napi::CallbackInfo& info);
  Napi::Value AddPrevSibling(const Napi::CallbackInfo& info);
  Napi::Value AddNextSibling(const Napi::CallbackInfo& info);
  Napi::Value Replace(const Napi::CallbackInfo& info);

  // Delegated from XmlNode base
  Napi::Value Doc(const Napi::CallbackInfo& i) { return Doc_Method(i); }
  Napi::Value Namespace(const Napi::CallbackInfo& i) { return Namespace_Method(i); }
  Napi::Value Namespaces(const Napi::CallbackInfo& i) { return Namespaces_Method(i); }
  Napi::Value Parent(const Napi::CallbackInfo& i) { return Parent_Method(i); }
  Napi::Value PrevSibling(const Napi::CallbackInfo& i) { return PrevSibling_Method(i); }
  Napi::Value NextSibling(const Napi::CallbackInfo& i) { return NextSibling_Method(i); }
  Napi::Value LineNumber(const Napi::CallbackInfo& i) { return LineNumber_Method(i); }
  Napi::Value Type(const Napi::CallbackInfo& i) { return Type_Method(i); }
  Napi::Value ToString(const Napi::CallbackInfo& i) { return ToString_Method(i); }
  Napi::Value Remove(const Napi::CallbackInfo& i) { return Remove_Method(i); }
  Napi::Value Clone(const Napi::CallbackInfo& i) { return Clone_Method(i); }

private:
  // helper methods (take Napi::Env)
  void set_name(const char *name);
  Napi::Value get_name(Napi::Env env);
  Napi::Value get_child(Napi::Env env, int32_t idx);
  Napi::Value get_child_nodes(Napi::Env env);
  Napi::Value get_path(Napi::Env env);
  Napi::Value get_attr(Napi::Env env, const char *name);
  Napi::Value get_attrs(Napi::Env env);
  void set_attr(Napi::Env env, const char *name, const char *value);
  void add_cdata(xmlNode *cdata);
  void unlink_children();
  void set_content(const char *content);
  Napi::Value get_content(Napi::Env env);
  Napi::Value get_next_element(Napi::Env env);
  Napi::Value get_prev_element(Napi::Env env);
  void replace_element(xmlNode *element);
  void replace_text(const char *content);
  bool child_will_merge(xmlNode *child);
};

} // namespace libxmljs

#endif // SRC_XML_ELEMENT_H_
