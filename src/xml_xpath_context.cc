// Copyright 2009, Squish Tech, LLC.

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "xml_element.h"
#include "xml_xpath_context.h"

namespace libxmljs {

XmlXpathContext::XmlXpathContext(xmlNode *node) {
  ctxt = xmlXPathNewContext(node->doc);
  ctxt->node = node;
}

XmlXpathContext::~XmlXpathContext() { xmlXPathFreeContext(ctxt); }

void XmlXpathContext::register_ns(const xmlChar *prefix, const xmlChar *uri) {
  xmlXPathRegisterNs(ctxt, prefix, uri);
}

Napi::Value XmlXpathContext::evaluate(Napi::Env env, const xmlChar *xpath) {
  xmlXPathObject *xpathobj = xmlXPathEval(xpath, ctxt);
  Napi::Value res = env.Null();

  if (xpathobj) {
    switch (xpathobj->type) {
    case XPATH_NODESET: {
      if (xmlXPathNodeSetIsEmpty(xpathobj->nodesetval)) {
        res = Napi::Array::New(env, 0);
        break;
      }

      Napi::Array nodes =
          Napi::Array::New(env, xpathobj->nodesetval->nodeNr);
      for (int i = 0; i != xpathobj->nodesetval->nodeNr; ++i) {
        nodes.Set(i, XmlNode::New(env, xpathobj->nodesetval->nodeTab[i]));
      }

      res = nodes;
      break;
    }

    case XPATH_BOOLEAN:
      res = Napi::Boolean::New(env, xpathobj->boolval);
      break;

    case XPATH_NUMBER:
      res = Napi::Number::New(env, xpathobj->floatval);
      break;

    case XPATH_STRING:
      res = Napi::String::New(env,
                              reinterpret_cast<const char *>(xpathobj->stringval),
                              xmlStrlen(xpathobj->stringval));
      break;

    default:
      res = env.Null();
      break;
    }
  }

  xmlXPathFreeObject(xpathobj);
  return res;
}

} // namespace libxmljs
