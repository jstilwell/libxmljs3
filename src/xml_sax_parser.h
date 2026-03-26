// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_SAX_PARSER_H_
#define SRC_XML_SAX_PARSER_H_

#include <napi.h>
#include <libxml/parser.h>

namespace libxmljs {

// Base class holding the shared SAX handler infrastructure.
// Not an ObjectWrap — XmlSaxParser and XmlSaxPushParser inherit from this
// alongside their respective Napi::ObjectWrap<T> base.
class SaxParserBase {
public:
  SaxParserBase();
  virtual ~SaxParserBase();

  // Called from SAX callbacks (which fire synchronously during parse/push).
  // env_ must be set before parsing begins.
  void Callback(const char *what, int argc = 0,
                Napi::Value argv[] = nullptr);

  void initializeContext();
  void releaseContext();

  void parse_string(const char *str, unsigned int size);
  void initialize_push_parser();
  void push(const char *str, unsigned int size, bool terminate);

  // SAX callback statics — receive the parser context and route back to the
  // SaxParserBase* stored in context_->_private.
  static void start_document(void *context);
  static void end_document(void *context);
  static void start_element_ns(void *context, const xmlChar *localname,
                               const xmlChar *prefix, const xmlChar *uri,
                               int nb_namespaces, const xmlChar **namespaces,
                               int nb_attributes, int nb_defaulted,
                               const xmlChar **attributes);
  static void end_element_ns(void *context, const xmlChar *localname,
                             const xmlChar *prefix, const xmlChar *uri);
  static void characters(void *context, const xmlChar *ch, int len);
  static void comment(void *context, const xmlChar *value);
  static void cdata_block(void *context, const xmlChar *value, int len);
  static void warning(void *context, const char *msg, ...);
  static void error(void *context, const char *msg, ...);

  xmlParserCtxt *context_;
  xmlSAXHandler sax_handler_;

  // Set to the current env before each parse call; cleared afterwards.
  // Lets Callback() create Napi values without storing a persistent env.
  Napi::Env *env_;

  // Returns the JS object for this wrapper.  Implemented by each concrete
  // ObjectWrap subclass.
  virtual Napi::Object SelfObject() = 0;
};

// SAX pull parser — wraps xmlCreateMemoryParserCtxt / xmlParseDocument.
class XmlSaxParser : public Napi::ObjectWrap<XmlSaxParser>,
                     public SaxParserBase {
public:
  static Napi::FunctionReference constructor;

  explicit XmlSaxParser(const Napi::CallbackInfo &info);
  ~XmlSaxParser() override = default;

  static void Initialize(Napi::Env env, Napi::Object exports);

  Napi::Value ParseString(const Napi::CallbackInfo &info);

  Napi::Object SelfObject() override { return Value(); }
};

// SAX push parser — wraps xmlCreatePushParserCtxt / xmlParseChunk.
class XmlSaxPushParser : public Napi::ObjectWrap<XmlSaxPushParser>,
                         public SaxParserBase {
public:
  static Napi::FunctionReference constructor;

  explicit XmlSaxPushParser(const Napi::CallbackInfo &info);
  ~XmlSaxPushParser() override = default;

  static void Initialize(Napi::Env env, Napi::Object exports);

  Napi::Value Push(const Napi::CallbackInfo &info);

  Napi::Object SelfObject() override { return Value(); }
};

} // namespace libxmljs

#endif // SRC_XML_SAX_PARSER_H_
