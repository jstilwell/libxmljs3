// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_DOCUMENT_H_
#define SRC_XML_DOCUMENT_H_

#include <libxml/tree.h>

#include "libxmljs.h"

namespace libxmljs {

class XmlDocument : public Napi::ObjectWrap<XmlDocument> {
public:
  static Napi::FunctionReference constructor;

  // TODO make private with accessor
  xmlDoc *xml_obj;

  XmlDocument(const Napi::CallbackInfo& info);
  virtual ~XmlDocument();

  // setup the document handle bindings and internal constructor
  static void Init(Napi::Env env, Napi::Object exports);

  // create a new document handle initialized with the
  // given xmlDoc object, intended for use in c++ space
  static Napi::Object New(Napi::Env env, xmlDoc *doc);

  void setEncoding(const char *encoding);

  // Instance methods
  Napi::Value Encoding(const Napi::CallbackInfo& info);
  Napi::Value Version(const Napi::CallbackInfo& info);
  Napi::Value Root(const Napi::CallbackInfo& info);
  Napi::Value GetDtd(const Napi::CallbackInfo& info);
  Napi::Value SetDtd(const Napi::CallbackInfo& info);
  Napi::Value ToString(const Napi::CallbackInfo& info);
  Napi::Value Validate(const Napi::CallbackInfo& info);
  Napi::Value RngValidate(const Napi::CallbackInfo& info);
  Napi::Value SchematronValidate(const Napi::CallbackInfo& info);
  Napi::Value type(const Napi::CallbackInfo& info);

  // Static methods (on exports, not prototype)
  static Napi::Value FromHtml(const Napi::CallbackInfo& info);
  static Napi::Value FromXml(const Napi::CallbackInfo& info);
};

} // namespace libxmljs

#endif // SRC_XML_DOCUMENT_H_
