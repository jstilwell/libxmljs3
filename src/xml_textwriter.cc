// Copyright 2011, Squish Tech, LLC.

#include "xml_textwriter.h"
#include "libxmljs.h"

namespace libxmljs {

// Throws a JS error and returns env.Null() when the xmlTextWriter call
// returned -1.  Relies on `env` and `result` being in scope.
#define THROW_ON_ERROR(text)                                                   \
  if (result == -1) {                                                          \
    Napi::Error::New(env, text).ThrowAsJavaScriptException();                  \
    return env.Null();                                                         \
  }

Napi::FunctionReference XmlTextWriter::constructor;

XmlTextWriter::XmlTextWriter(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<XmlTextWriter>(info),
      textWriter(nullptr),
      writerBuffer(nullptr) {
  // Open an in-memory buffer immediately on construction.
  OpenMemory(info);
}

XmlTextWriter::~XmlTextWriter() {
  if (textWriter) {
    xmlFreeTextWriter(textWriter);
    textWriter = nullptr;
  }
  if (writerBuffer) {
    xmlBufferFree(writerBuffer);
    writerBuffer = nullptr;
  }
}

Napi::Value XmlTextWriter::OpenMemory(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  writerBuffer = xmlBufferCreate();
  if (!writerBuffer) {
    Napi::Error::New(env, "Failed to create memory buffer")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  textWriter = xmlNewTextWriterMemory(writerBuffer, 0);
  if (!textWriter) {
    xmlBufferFree(writerBuffer);
    writerBuffer = nullptr;
    Napi::Error::New(env, "Failed to create buffer writer")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  return env.Undefined();
}

Napi::Value XmlTextWriter::BufferContent(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  // Flush the output buffer so all content reaches writerBuffer.
  xmlTextWriterFlush(textWriter);

  const xmlChar *buf = xmlBufferContent(writerBuffer);
  return Napi::String::New(env, (const char *)buf,
                           xmlBufferLength(writerBuffer));
}

void XmlTextWriter::clearBuffer() {
  xmlTextWriterFlush(textWriter);
  xmlBufferEmpty(writerBuffer);
}

Napi::Value XmlTextWriter::BufferEmpty(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  clearBuffer();
  return env.Undefined();
}

Napi::Value XmlTextWriter::StartDocument(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  // version — info[0]
  std::string version_str;
  const char *version = nullptr;
  if (!info[0].IsUndefined() && !info[0].IsNull()) {
    version_str = info[0].As<Napi::String>().Utf8Value();
    version = version_str.c_str();
  }

  // encoding — info[1]
  std::string encoding_str;
  const char *encoding = nullptr;
  if (!info[1].IsUndefined() && !info[1].IsNull()) {
    encoding_str = info[1].As<Napi::String>().Utf8Value();
    encoding = encoding_str.c_str();
  }

  // standalone — info[2]: boolean or string
  std::string standalone_str;
  const char *standalone = nullptr;
  if (info.Length() > 2 && !info[2].IsUndefined() && !info[2].IsNull()) {
    if (info[2].IsBoolean()) {
      standalone_str = info[2].As<Napi::Boolean>().Value() ? "yes" : "no";
      standalone = standalone_str.c_str();
    } else if (info[2].IsString()) {
      standalone_str = info[2].As<Napi::String>().Utf8Value();
      standalone = standalone_str.c_str();
    }
  }

  int result = xmlTextWriterStartDocument(textWriter, version, encoding,
                                          standalone);
  THROW_ON_ERROR("Failed to start document");

  return Napi::Number::New(env, static_cast<double>(result));
}

Napi::Value XmlTextWriter::EndDocument(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  int result = xmlTextWriterEndDocument(textWriter);
  THROW_ON_ERROR("Failed to end document");

  return Napi::Number::New(env, static_cast<double>(result));
}

Napi::Value XmlTextWriter::StartElementNS(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  std::string prefix_str, name_str, ns_str;
  const xmlChar *prefix = nullptr, *name = nullptr, *namespaceURI = nullptr;

  if (!info[0].IsUndefined() && !info[0].IsNull()) {
    prefix_str = info[0].As<Napi::String>().Utf8Value();
    prefix = (const xmlChar *)prefix_str.c_str();
  }
  if (!info[1].IsUndefined() && !info[1].IsNull()) {
    name_str = info[1].As<Napi::String>().Utf8Value();
    name = (const xmlChar *)name_str.c_str();
  }
  if (!info[2].IsUndefined() && !info[2].IsNull()) {
    ns_str = info[2].As<Napi::String>().Utf8Value();
    namespaceURI = (const xmlChar *)ns_str.c_str();
  }

  int result = xmlTextWriterStartElementNS(textWriter, prefix, name,
                                           namespaceURI);
  THROW_ON_ERROR("Failed to start element");

  return Napi::Number::New(env, static_cast<double>(result));
}

Napi::Value XmlTextWriter::EndElement(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  int result = xmlTextWriterEndElement(textWriter);
  THROW_ON_ERROR("Failed to end element");

  return Napi::Number::New(env, static_cast<double>(result));
}

Napi::Value XmlTextWriter::StartAttributeNS(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  std::string prefix_str, name_str, ns_str;
  const xmlChar *prefix = nullptr, *name = nullptr, *namespaceURI = nullptr;

  if (!info[0].IsUndefined() && !info[0].IsNull()) {
    prefix_str = info[0].As<Napi::String>().Utf8Value();
    prefix = (const xmlChar *)prefix_str.c_str();
  }
  if (!info[1].IsUndefined() && !info[1].IsNull()) {
    name_str = info[1].As<Napi::String>().Utf8Value();
    name = (const xmlChar *)name_str.c_str();
  }
  if (!info[2].IsUndefined() && !info[2].IsNull()) {
    ns_str = info[2].As<Napi::String>().Utf8Value();
    namespaceURI = (const xmlChar *)ns_str.c_str();
  }

  int result = xmlTextWriterStartAttributeNS(textWriter, prefix, name,
                                             namespaceURI);
  THROW_ON_ERROR("Failed to start attribute");

  return Napi::Number::New(env, static_cast<double>(result));
}

Napi::Value XmlTextWriter::EndAttribute(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  int result = xmlTextWriterEndAttribute(textWriter);
  THROW_ON_ERROR("Failed to end attribute");

  return Napi::Number::New(env, static_cast<double>(result));
}

Napi::Value XmlTextWriter::StartCdata(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  int result = xmlTextWriterStartCDATA(textWriter);
  THROW_ON_ERROR("Failed to start CDATA section");

  return Napi::Number::New(env, static_cast<double>(result));
}

Napi::Value XmlTextWriter::EndCdata(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  int result = xmlTextWriterEndCDATA(textWriter);
  THROW_ON_ERROR("Failed to end CDATA section");

  return Napi::Number::New(env, static_cast<double>(result));
}

Napi::Value XmlTextWriter::StartComment(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  int result = xmlTextWriterStartComment(textWriter);
  THROW_ON_ERROR("Failed to start Comment section");

  return Napi::Number::New(env, static_cast<double>(result));
}

Napi::Value XmlTextWriter::EndComment(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  int result = xmlTextWriterEndComment(textWriter);
  THROW_ON_ERROR("Failed to end Comment section");

  return Napi::Number::New(env, static_cast<double>(result));
}

Napi::Value XmlTextWriter::WriteString(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  std::string str = info[0].As<Napi::String>().Utf8Value();
  int result = xmlTextWriterWriteString(textWriter,
                                       (const xmlChar *)str.c_str());
  THROW_ON_ERROR("Failed to write string");

  return Napi::Number::New(env, static_cast<double>(result));
}

Napi::Value XmlTextWriter::OutputMemory(const Napi::CallbackInfo &info) {
  bool clear = (info.Length() == 0) ||
               info[0].As<Napi::Boolean>().Value();

  Napi::Value content = BufferContent(info);

  if (clear) {
    clearBuffer();
  }

  return content;
}

void XmlTextWriter::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "TextWriter", {
    InstanceMethod("toString",        &XmlTextWriter::BufferContent),
    InstanceMethod("outputMemory",    &XmlTextWriter::OutputMemory),
    InstanceMethod("clear",           &XmlTextWriter::BufferEmpty),
    InstanceMethod("startDocument",   &XmlTextWriter::StartDocument),
    InstanceMethod("endDocument",     &XmlTextWriter::EndDocument),
    InstanceMethod("startElementNS",  &XmlTextWriter::StartElementNS),
    InstanceMethod("endElement",      &XmlTextWriter::EndElement),
    InstanceMethod("startAttributeNS",&XmlTextWriter::StartAttributeNS),
    InstanceMethod("endAttribute",    &XmlTextWriter::EndAttribute),
    InstanceMethod("startCdata",      &XmlTextWriter::StartCdata),
    InstanceMethod("endCdata",        &XmlTextWriter::EndCdata),
    InstanceMethod("startComment",    &XmlTextWriter::StartComment),
    InstanceMethod("endComment",      &XmlTextWriter::EndComment),
    InstanceMethod("writeString",     &XmlTextWriter::WriteString),
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("TextWriter", func);
}

} // namespace libxmljs
