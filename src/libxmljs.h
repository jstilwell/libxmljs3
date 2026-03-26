// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_LIBXMLJS_H_
#define SRC_LIBXMLJS_H_

#include <napi.h>

#define LIBXMLJS_ARGUMENT_TYPE_CHECK(arg, type, err)                           \
  if (!arg.type()) {                                                           \
    Napi::TypeError::New(info.Env(), err).ThrowAsJavaScriptException();        \
    return info.Env().Undefined();                                             \
  }

#define NAPI_CONSTRUCTOR_CHECK(name)                                           \
  if (info.NewTarget().IsEmpty()) {                                            \
    Napi::TypeError::New(info.Env(),                                           \
                         "Class constructor " #name                            \
                         " cannot be invoked without 'new'")                  \
        .ThrowAsJavaScriptException();                                         \
    return;                                                                    \
  }

#define DOCUMENT_ARG_CHECK                                                     \
  if (info.Length() == 0 ||                                                    \
      (info[0].IsNull() || info[0].IsUndefined())) {                           \
    Napi::Error::New(info.Env(), "document argument required")                 \
        .ThrowAsJavaScriptException();                                         \
    return;                                                                    \
  }                                                                            \
  Napi::Object doc = info[0].As<Napi::Object>();                               \
  if (!info[0].InstanceOf(XmlDocument::constructor.Value())) {                 \
    Napi::Error::New(info.Env(),                                               \
                     "document argument must be an instance of Document")      \
        .ThrowAsJavaScriptException();                                         \
    return;                                                                    \
  }

namespace libxmljs {

#ifdef LIBXML_DEBUG_ENABLED
static const bool debugging = true;
#else
static const bool debugging = false;
#endif

// Ensure that libxml is properly initialised and destructed at shutdown
class LibXMLJS {
public:
  LibXMLJS();
  virtual ~LibXMLJS();

private:
  static LibXMLJS instance;
};

} // namespace libxmljs

#endif // SRC_LIBXMLJS_H_
