// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_SYNTAX_ERROR_H_
#define SRC_XML_SYNTAX_ERROR_H_

#include <libxml/xmlerror.h>

#include "libxmljs.h"

namespace libxmljs {

// Context struct passed as the void* userdata to libxml2's structured error
// callback. Carries both the Napi::Env (needed to create JS values) and a
// pointer to the Napi::Array that errors are pushed onto.
struct XmlSyntaxErrorContext {
  Napi::Env env;
  Napi::Array *errors;
};

// basically being used like a namespace
class XmlSyntaxError {
public:
  // push xmlError onto a Napi::Array
  // Matches the xmlStructuredErrorFunc signature: void(*)(void*, xmlErrorPtr)
  // The context pointer must be a XmlSyntaxErrorContext*.
  static void PushToArray(void *context, xmlError *error);

  // create a Napi Error object for the syntax error
  static Napi::Value BuildSyntaxError(Napi::Env env, xmlError *error);
};

} // namespace libxmljs

#endif // SRC_XML_SYNTAX_ERROR_H_
