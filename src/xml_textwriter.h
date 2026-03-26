// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_TEXTWRITER_H_
#define SRC_XML_TEXTWRITER_H_

#include <libxml/xmlwriter.h>

#include "libxmljs.h"

namespace libxmljs {

class XmlTextWriter : public Napi::ObjectWrap<XmlTextWriter> {
public:
  static Napi::FunctionReference constructor;

  explicit XmlTextWriter(const Napi::CallbackInfo &info);
  ~XmlTextWriter() override;

  static void Initialize(Napi::Env env, Napi::Object exports);

  Napi::Value OpenMemory(const Napi::CallbackInfo &info);
  Napi::Value BufferContent(const Napi::CallbackInfo &info);
  Napi::Value BufferEmpty(const Napi::CallbackInfo &info);
  Napi::Value StartDocument(const Napi::CallbackInfo &info);
  Napi::Value EndDocument(const Napi::CallbackInfo &info);
  Napi::Value StartElementNS(const Napi::CallbackInfo &info);
  Napi::Value EndElement(const Napi::CallbackInfo &info);
  Napi::Value StartAttributeNS(const Napi::CallbackInfo &info);
  Napi::Value EndAttribute(const Napi::CallbackInfo &info);
  Napi::Value StartCdata(const Napi::CallbackInfo &info);
  Napi::Value EndCdata(const Napi::CallbackInfo &info);
  Napi::Value StartComment(const Napi::CallbackInfo &info);
  Napi::Value EndComment(const Napi::CallbackInfo &info);
  Napi::Value WriteString(const Napi::CallbackInfo &info);
  Napi::Value OutputMemory(const Napi::CallbackInfo &info);

private:
  xmlTextWriterPtr textWriter;
  xmlBufferPtr writerBuffer;

  void clearBuffer();
  bool is_open();
  bool is_inmemory();
};

} // namespace libxmljs

#endif // SRC_XML_TEXTWRITER_H_
