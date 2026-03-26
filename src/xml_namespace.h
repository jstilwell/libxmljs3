// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_NAMESPACE_H_
#define SRC_XML_NAMESPACE_H_

#include <napi.h>
#include <libxml/tree.h>

namespace libxmljs {

class XmlNamespace : public Napi::ObjectWrap<XmlNamespace> {
public:
  xmlNs *xml_obj;

  xmlDoc *context; // reference-managed context

  static Napi::FunctionReference constructor;

  XmlNamespace(const Napi::CallbackInfo& info);
  ~XmlNamespace();

  // Factory: wraps an existing xmlNs, reusing JS wrapper if already created
  static Napi::Object New(Napi::Env env, xmlNs *ns);

  // Called after construction via constructor.New({}) to bind the native pointer
  void initFromNative(xmlNs *ns);

  static void Init(Napi::Env env, Napi::Object exports);

  Napi::Value Href(const Napi::CallbackInfo& info);
  Napi::Value Prefix(const Napi::CallbackInfo& info);

private:
  Napi::Value get_href(Napi::Env env);
  Napi::Value get_prefix(Napi::Env env);
};

} // namespace libxmljs

#endif // SRC_XML_NAMESPACE_H_
