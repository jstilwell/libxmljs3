// Copyright 2009, Squish Tech, LLC.

#include <cassert>
#include <string>
#include <vector>

#include <libxml/xmlsave.h>

#include "xml_attribute.h"
#include "xml_comment.h"
#include "xml_document.h"
#include "xml_element.h"
#include "xml_namespace.h"
#include "xml_node.h"
#include "xml_pi.h"
#include "xml_text.h"

namespace libxmljs {

// ---------------------------------------------------------------------------
// XmlNode constructor / destructor
// ---------------------------------------------------------------------------

XmlNode::XmlNode(xmlNode *node) : xml_obj(node), ancestor(nullptr), doc(nullptr) {
  // nullptr is passed by concrete subclass constructors that use the C++
  // factory pattern (New(env, node)).  In that case the subclass sets xml_obj
  // and calls attach() manually after construction.
  if (xml_obj == nullptr) {
    return;
  }

  xml_obj->_private = this;

  if ((xml_obj->doc != nullptr) && (xml_obj->doc->_private != nullptr)) {
    this->doc = xml_obj->doc;
    static_cast<XmlDocument *>(this->doc->_private)->Ref();
  }

  this->ref_wrapped_ancestor();
}

// Property key used to store the XmlNode* back-pointer on the JS object.
static const char *kXmlNodePtrKey = "__xmlNodePtr__";

void XmlNode::attach(Napi::Object wrapper) {
  // Store a napi_external holding the XmlNode* on the JS wrapper object so
  // that XmlNode::Unwrap(env, obj) can recover it without knowing the
  // concrete subclass type.
  Napi::Env env = wrapper.Env();
  Napi::External<XmlNode> ext = Napi::External<XmlNode>::New(env, this);
  wrapper.DefineProperty(
      Napi::PropertyDescriptor::Value(kXmlNodePtrKey, ext, napi_default));
}

XmlNode::~XmlNode() {
  if ((this->doc != nullptr) && (this->doc->_private != nullptr)) {
    static_cast<XmlDocument *>(this->doc->_private)->Unref();
  }
  this->unref_wrapped_ancestor();

  if (xml_obj == nullptr) {
    return;
  }

  xml_obj->_private = nullptr;
  if (xml_obj->parent == nullptr) {
    if (get_wrapped_descendant(xml_obj) == nullptr) {
      xmlFreeNode(xml_obj);
    }
  } else {
    xmlNode *anc = get_wrapped_ancestor_or_root(xml_obj);
    if ((anc->_private == nullptr) && (anc->parent == nullptr) &&
        (get_wrapped_descendant(anc, xml_obj) == nullptr)) {
      xmlFreeNode(anc);
    }
  }
}

// ---------------------------------------------------------------------------
// Static factory: wrap a libxml node in the correct JS subclass wrapper.
// ---------------------------------------------------------------------------

// static
Napi::Value XmlNode::New(Napi::Env env, xmlNode *node) {
  switch (node->type) {
  case XML_ATTRIBUTE_NODE:
    return XmlAttribute::New(env, reinterpret_cast<xmlAttr *>(node));
  case XML_TEXT_NODE:
    return XmlText::New(env, node);
  case XML_PI_NODE:
    return XmlProcessingInstruction::New(env, node);
  case XML_COMMENT_NODE:
    return XmlComment::New(env, node);
  default:
    // Fall back to XmlElement for element nodes and anything else.
    return XmlElement::New(env, node);
  }
}

// ---------------------------------------------------------------------------
// Cross-subclass unwrap helper.
// All concrete XmlNode subclasses set xml_obj->_private = this (XmlNode*)
// in their XmlNode base constructor.  We use napi_unwrap to ensure the JS
// object really is one of our wrappers (it must have an internal field),
// then read xml_obj->_private for the XmlNode*.
// ---------------------------------------------------------------------------

// static
XmlNode *XmlNode::Unwrap(Napi::Env /*env*/, Napi::Object obj) {
  // Each concrete subclass constructor calls XmlNode::attach(info.This())
  // which stores a napi_external containing the XmlNode* as a property named
  // kXmlNodePtrKey on the JS object.  We read it back here.
  if (!obj.Has(kXmlNodePtrKey)) {
    return nullptr;
  }
  Napi::Value val = obj.Get(kXmlNodePtrKey);
  if (!val.IsExternal()) {
    return nullptr;
  }
  return val.As<Napi::External<XmlNode>>().Data();
}

// ---------------------------------------------------------------------------
// Static tree-walk helpers
// ---------------------------------------------------------------------------

/*
 * Return the (non-document) root, or a wrapped ancestor: whichever is closest.
 */
// static
xmlNode *XmlNode::get_wrapped_ancestor_or_root(xmlNode *xml_obj) {
  while ((xml_obj->parent != nullptr) &&
         (static_cast<void *>(xml_obj->doc) !=
          static_cast<void *>(xml_obj->parent)) &&
         (xml_obj->parent->_private == nullptr)) {
    xml_obj = xml_obj->parent;
  }
  return ((xml_obj->parent != nullptr) &&
          (static_cast<void *>(xml_obj->doc) !=
           static_cast<void *>(xml_obj->parent)))
             ? xml_obj->parent
             : xml_obj;
}

/*
 * Search linked list for a JS wrapper, ignoring the given node.
 */
static xmlAttr *get_wrapped_attr_in_list(xmlAttr *xml_obj, void *skip_xml_obj) {
  xmlAttr *wrapped_attr = nullptr;
  while (xml_obj != nullptr) {
    if ((xml_obj != skip_xml_obj) && (xml_obj->_private != nullptr)) {
      wrapped_attr = xml_obj;
      xml_obj = nullptr;
    } else {
      xml_obj = xml_obj->next;
    }
  }
  return wrapped_attr;
}

static xmlNs *get_wrapped_ns_in_list(xmlNs *xml_obj, void *skip_xml_obj) {
  xmlNs *wrapped_ns = nullptr;
  while (xml_obj != nullptr) {
    if ((xml_obj != skip_xml_obj) && (xml_obj->_private != nullptr)) {
      wrapped_ns = xml_obj;
      xml_obj = nullptr;
    } else {
      xml_obj = xml_obj->next;
    }
  }
  return wrapped_ns;
}

// Forward declaration.
static xmlNode *get_wrapped_node_in_children(xmlNode *xml_obj, xmlNode *skip_xml_obj);

/*
 * Search document for a JS wrapper, ignoring the given node.
 * Based on xmlFreeDoc.
 */
static xmlNode *get_wrapped_node_in_document(xmlDoc *xml_obj, xmlNode *skip_xml_obj) {
  xmlNode *wrapped_node = nullptr;
  if ((xml_obj->extSubset != nullptr) && (xml_obj->extSubset->_private != nullptr) &&
      (static_cast<void *>(xml_obj->extSubset) != skip_xml_obj)) {
    wrapped_node = reinterpret_cast<xmlNode *>(xml_obj->extSubset);
  }
  if ((wrapped_node == nullptr) && (xml_obj->intSubset != nullptr) &&
      (xml_obj->intSubset->_private != nullptr) &&
      (static_cast<void *>(xml_obj->intSubset) != skip_xml_obj)) {
    wrapped_node = reinterpret_cast<xmlNode *>(xml_obj->intSubset);
  }
  if ((wrapped_node == nullptr) && (xml_obj->children != nullptr)) {
    wrapped_node = get_wrapped_node_in_children(xml_obj->children, skip_xml_obj);
  }
  if ((wrapped_node == nullptr) && (xml_obj->oldNs != nullptr)) {
    wrapped_node = reinterpret_cast<xmlNode *>(
        get_wrapped_ns_in_list(xml_obj->oldNs, skip_xml_obj));
  }
  return wrapped_node;
}

/*
 * Search children of a node for a JS wrapper, ignoring the given node.
 * Based on xmlFreeNodeList.
 */
static xmlNode *get_wrapped_node_in_children(xmlNode *xml_obj, xmlNode *skip_xml_obj) {
  xmlNode *wrapped_node = nullptr;

  if (xml_obj->type == XML_NAMESPACE_DECL) {
    return reinterpret_cast<xmlNode *>(get_wrapped_ns_in_list(
        reinterpret_cast<xmlNs *>(xml_obj), skip_xml_obj));
  }

  if ((xml_obj->type == XML_DOCUMENT_NODE) ||
#ifdef LIBXML_DOCB_ENABLED
      (xml_obj->type == XML_DOCB_DOCUMENT_NODE) ||
#endif
      (xml_obj->type == XML_HTML_DOCUMENT_NODE)) {
    return get_wrapped_node_in_document(reinterpret_cast<xmlDoc *>(xml_obj),
                                        skip_xml_obj);
  }

  xmlNode *next;
  while (xml_obj != nullptr) {
    next = xml_obj->next;

    if ((xml_obj != skip_xml_obj) && (xml_obj->_private != nullptr)) {
      wrapped_node = xml_obj;
    } else {
      if ((xml_obj->children != nullptr) &&
          (xml_obj->type != XML_ENTITY_REF_NODE)) {
        wrapped_node = get_wrapped_node_in_children(xml_obj->children, skip_xml_obj);
      }

      if ((wrapped_node == nullptr) &&
          ((xml_obj->type == XML_ELEMENT_NODE) ||
           (xml_obj->type == XML_XINCLUDE_START) ||
           (xml_obj->type == XML_XINCLUDE_END))) {

        if ((wrapped_node == nullptr) && (xml_obj->properties != nullptr)) {
          wrapped_node = reinterpret_cast<xmlNode *>(
              get_wrapped_attr_in_list(xml_obj->properties, skip_xml_obj));
        }

        if ((wrapped_node == nullptr) && (xml_obj->nsDef != nullptr)) {
          wrapped_node = reinterpret_cast<xmlNode *>(
              get_wrapped_ns_in_list(xml_obj->nsDef, skip_xml_obj));
        }
      }
    }

    if (wrapped_node != nullptr) {
      break;
    }

    xml_obj = next;
  }

  return wrapped_node;
}

/*
 * Search descendants of a node for a JS wrapper, optionally ignoring a node.
 * Based on xmlFreeNode.
 */
// static
xmlNode *XmlNode::get_wrapped_descendant(xmlNode *xml_obj, xmlNode *skip_xml_obj) {
  xmlNode *wrapped_descendant = nullptr;

  if (xml_obj->type == XML_DTD_NODE) {
    return (xml_obj->children == nullptr)
               ? nullptr
               : get_wrapped_node_in_children(xml_obj->children, skip_xml_obj);
  }

  if (xml_obj->type == XML_NAMESPACE_DECL) {
    return nullptr;
  }

  if (xml_obj->type == XML_ATTRIBUTE_NODE) {
    return (xml_obj->children == nullptr)
               ? nullptr
               : get_wrapped_node_in_children(xml_obj->children, skip_xml_obj);
  }

  if ((xml_obj->children != nullptr) && (xml_obj->type != XML_ENTITY_REF_NODE)) {
    wrapped_descendant = get_wrapped_node_in_children(xml_obj->children, skip_xml_obj);
  }

  if ((xml_obj->type == XML_ELEMENT_NODE) ||
      (xml_obj->type == XML_XINCLUDE_START) ||
      (xml_obj->type == XML_XINCLUDE_END)) {

    if ((wrapped_descendant == nullptr) && (xml_obj->properties != nullptr)) {
      wrapped_descendant = reinterpret_cast<xmlNode *>(
          get_wrapped_attr_in_list(xml_obj->properties, skip_xml_obj));
    }

    if ((wrapped_descendant == nullptr) && (xml_obj->nsDef != nullptr)) {
      wrapped_descendant = reinterpret_cast<xmlNode *>(
          get_wrapped_ns_in_list(xml_obj->nsDef, skip_xml_obj));
    }
  }

  return wrapped_descendant;
}

// ---------------------------------------------------------------------------
// Ancestor ref-count management
// ---------------------------------------------------------------------------

xmlNode *XmlNode::get_wrapped_ancestor() {
  xmlNode *anc = get_wrapped_ancestor_or_root(xml_obj);
  return ((xml_obj == anc) || (anc->_private == nullptr)) ? nullptr : anc;
}

void XmlNode::ref_wrapped_ancestor() {
  xmlNode *anc = this->get_wrapped_ancestor();

  // If our closest wrapped ancestor has changed, release the old ref first.
  if (anc != this->ancestor) {
    this->unref_wrapped_ancestor();
    this->ancestor = anc;
  }

  if (this->ancestor != nullptr) {
    XmlNode *node = static_cast<XmlNode *>(this->ancestor->_private);
    node->Ref_();
  }
}

void XmlNode::unref_wrapped_ancestor() {
  if ((this->ancestor != nullptr) && (this->ancestor->_private != nullptr)) {
    static_cast<XmlNode *>(this->ancestor->_private)->Unref_();
  }
  this->ancestor = nullptr;
}

// ---------------------------------------------------------------------------
// Helper method implementations
// ---------------------------------------------------------------------------

Napi::Value XmlNode::get_doc(Napi::Env env) {
  return XmlDocument::New(env, xml_obj->doc);
}

Napi::Value XmlNode::remove_namespace(Napi::Env env) {
  xml_obj->ns = nullptr;
  return env.Null();
}

Napi::Value XmlNode::get_namespace(Napi::Env env) {
  if (!xml_obj->ns) {
    return env.Null();
  }
  return XmlNamespace::New(env, xml_obj->ns);
}

void XmlNode::set_namespace(xmlNs *ns) {
  xmlSetNs(xml_obj, ns);
  assert(xml_obj->ns);
}

xmlNs *XmlNode::find_namespace(const char *search_str) {
  xmlNs *ns = nullptr;

  // Try prefix first.
  ns = xmlSearchNs(xml_obj->doc, xml_obj,
                   reinterpret_cast<const xmlChar *>(search_str));

  // Fall back to href.
  if (!ns) {
    ns = xmlSearchNsByHref(xml_obj->doc, xml_obj,
                           reinterpret_cast<const xmlChar *>(search_str));
  }

  return ns;
}

Napi::Value XmlNode::get_all_namespaces(Napi::Env env) {
  Napi::Array namespaces = Napi::Array::New(env);
  xmlNs **nsList = xmlGetNsList(xml_obj->doc, xml_obj);
  if (nsList != nullptr) {
    for (int i = 0; nsList[i] != nullptr; i++) {
      namespaces.Set(i, XmlNamespace::New(env, nsList[i]));
    }
    xmlFree(nsList);
  }
  return namespaces;
}

Napi::Value XmlNode::get_local_namespaces(Napi::Env env) {
  Napi::Array namespaces = Napi::Array::New(env);
  xmlNs *nsDef = xml_obj->nsDef;
  for (int i = 0; nsDef; i++, nsDef = nsDef->next) {
    namespaces.Set(i, XmlNamespace::New(env, nsDef));
  }
  return namespaces;
}

Napi::Value XmlNode::get_parent(Napi::Env env) {
  if (xml_obj->parent) {
    // If the parent is actually the document node, return the document wrapper
    if (static_cast<void *>(xml_obj->parent) ==
        static_cast<void *>(xml_obj->doc)) {
      return XmlDocument::New(env, xml_obj->doc);
    }
    return XmlElement::New(env, xml_obj->parent);
  }
  return XmlDocument::New(env, xml_obj->doc);
}

Napi::Value XmlNode::get_prev_sibling(Napi::Env env) {
  if (xml_obj->prev) {
    return XmlNode::New(env, xml_obj->prev);
  }
  return env.Null();
}

Napi::Value XmlNode::get_next_sibling(Napi::Env env) {
  if (xml_obj->next) {
    return XmlNode::New(env, xml_obj->next);
  }
  return env.Null();
}

Napi::Value XmlNode::get_line_number(Napi::Env env) {
  return Napi::Number::New(env, static_cast<double>(xmlGetLineNo(xml_obj)));
}

Napi::Value XmlNode::clone(Napi::Env env, bool recurse) {
  xmlNode *new_xml_obj = xmlDocCopyNode(xml_obj, xml_obj->doc, recurse ? 1 : 0);
  return XmlNode::New(env, new_xml_obj);
}

Napi::Value XmlNode::to_string(Napi::Env env, int options) {
  xmlBuffer *buf = xmlBufferCreate();
  const char *enc = "UTF-8";

  xmlSaveCtxt *savectx = xmlSaveToBuffer(buf, enc, options);
  xmlSaveTree(savectx, xml_obj);
  xmlSaveFlush(savectx);

  const xmlChar *xmlstr = xmlBufferContent(buf);

  if (xmlstr) {
    Napi::String str = Napi::String::New(
        env, reinterpret_cast<const char *>(xmlstr), xmlBufferLength(buf));
    xmlSaveClose(savectx);
    xmlBufferFree(buf);
    return str;
  }

  xmlSaveClose(savectx);
  xmlBufferFree(buf);
  return env.Null();
}

void XmlNode::remove() {
  this->unref_wrapped_ancestor();
  xmlUnlinkNode(xml_obj);
}

void XmlNode::add_child(xmlNode *child) {
  xmlAddChild(xml_obj, child);
}

void XmlNode::add_prev_sibling(xmlNode *node) {
  xmlAddPrevSibling(xml_obj, node);
}

void XmlNode::add_next_sibling(xmlNode *node) {
  xmlAddNextSibling(xml_obj, node);
}

xmlNode *XmlNode::import_node(xmlNode *node) {
  if (xml_obj->doc == node->doc) {
    if ((node->parent != nullptr) && (node->_private != nullptr)) {
      static_cast<XmlNode *>(node->_private)->remove();
    }
    return node;
  }
  return xmlDocCopyNode(node, xml_obj->doc, 1);
}

Napi::Value XmlNode::get_type(Napi::Env env) {
  switch (xml_obj->type) {
  case XML_ELEMENT_NODE:        return Napi::String::New(env, "element");
  case XML_ATTRIBUTE_NODE:      return Napi::String::New(env, "attribute");
  case XML_TEXT_NODE:           return Napi::String::New(env, "text");
  case XML_CDATA_SECTION_NODE:  return Napi::String::New(env, "cdata");
  case XML_ENTITY_REF_NODE:     return Napi::String::New(env, "entity_ref");
  case XML_ENTITY_NODE:         return Napi::String::New(env, "entity");
  case XML_PI_NODE:             return Napi::String::New(env, "pi");
  case XML_COMMENT_NODE:        return Napi::String::New(env, "comment");
  case XML_DOCUMENT_NODE:       return Napi::String::New(env, "document");
  case XML_DOCUMENT_TYPE_NODE:  return Napi::String::New(env, "document_type");
  case XML_DOCUMENT_FRAG_NODE:  return Napi::String::New(env, "document_frag");
  case XML_NOTATION_NODE:       return Napi::String::New(env, "notation");
  case XML_HTML_DOCUMENT_NODE:  return Napi::String::New(env, "html_document");
  case XML_DTD_NODE:            return Napi::String::New(env, "dtd");
  case XML_ELEMENT_DECL:        return Napi::String::New(env, "element_decl");
  case XML_ATTRIBUTE_DECL:      return Napi::String::New(env, "attribute_decl");
  case XML_ENTITY_DECL:         return Napi::String::New(env, "entity_decl");
  case XML_NAMESPACE_DECL:      return Napi::String::New(env, "namespace_decl");
  case XML_XINCLUDE_START:      return Napi::String::New(env, "xinclude_start");
  case XML_XINCLUDE_END:        return Napi::String::New(env, "xinclude_end");
  case XML_DOCB_DOCUMENT_NODE:  return Napi::String::New(env, "docb_document");
  }
  return env.Null();
}

// ---------------------------------------------------------------------------
// Prototype method wrappers — called by subclass instance method delegates.
// Each subclass registers these as InstanceMethods by wrapping with a lambda
// or a concrete method that calls the corresponding XmlNode method.
// ---------------------------------------------------------------------------

Napi::Value XmlNode::Doc_Method(const Napi::CallbackInfo& info) {
  return get_doc(info.Env());
}

Napi::Value XmlNode::Namespace_Method(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  // namespace() — get the node's current namespace
  if (info.Length() == 0) {
    return get_namespace(env);
  }

  // namespace(null) — remove the namespace
  if (info[0].IsNull()) {
    return remove_namespace(env);
  }

  XmlNamespace *ns = nullptr;

  // namespace(nsObj) — an XmlNamespace JS object was provided
  if (info[0].IsObject() &&
      info[0].As<Napi::Object>().InstanceOf(XmlNamespace::constructor.Value())) {
    ns = Napi::ObjectWrap<XmlNamespace>::Unwrap(info[0].As<Napi::Object>());
  }

  // namespace(href) or namespace(prefix, href) — search existing namespaces
  if (info[0].IsString()) {
    std::string ns_to_find = info[0].As<Napi::String>().Utf8Value();
    xmlNs *found_ns = find_namespace(ns_to_find.c_str());
    if (found_ns) {
      Napi::Object existing = XmlNamespace::New(env, found_ns);
      ns = XmlNamespace::Unwrap(existing);
    }
  }

  // Namespace not found — create a new one attached to this node.
  if (!ns) {
    std::vector<napi_value> argv;
    argv.push_back(Value_());  // the JS object for this node

    if (info.Length() == 1) {
      argv.push_back(env.Null());
      argv.push_back(info[0]);
    } else {
      argv.push_back(info[0]);
      argv.push_back(info[1]);
    }

    Napi::Object new_ns = XmlNamespace::constructor.New(argv);
    ns = XmlNamespace::Unwrap(new_ns);
  }

  set_namespace(ns->xml_obj);
  return Value_();
}

Napi::Value XmlNode::Namespaces_Method(const Napi::CallbackInfo& info) {
  // Treat only a literal `true` as requesting local-only namespaces.
  if ((info.Length() == 0) || !info[0].IsBoolean() ||
      !info[0].As<Napi::Boolean>().Value()) {
    return get_all_namespaces(info.Env());
  }
  return get_local_namespaces(info.Env());
}

Napi::Value XmlNode::Parent_Method(const Napi::CallbackInfo& info) {
  return get_parent(info.Env());
}

Napi::Value XmlNode::PrevSibling_Method(const Napi::CallbackInfo& info) {
  return get_prev_sibling(info.Env());
}

Napi::Value XmlNode::NextSibling_Method(const Napi::CallbackInfo& info) {
  return get_next_sibling(info.Env());
}

Napi::Value XmlNode::LineNumber_Method(const Napi::CallbackInfo& info) {
  return get_line_number(info.Env());
}

Napi::Value XmlNode::Type_Method(const Napi::CallbackInfo& info) {
  return get_type(info.Env());
}

Napi::Value XmlNode::ToString_Method(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  int options = 0;

  if (info.Length() > 0) {
    if (info[0].IsBoolean()) {
      if (info[0].As<Napi::Boolean>().Value()) {
        options |= XML_SAVE_FORMAT;
      }
    } else if (info[0].IsObject()) {
      Napi::Object obj = info[0].As<Napi::Object>();

      // drop the XML declaration
      if (obj.Has("declaration") && obj.Get("declaration").IsBoolean() &&
          !obj.Get("declaration").As<Napi::Boolean>().Value()) {
        options |= XML_SAVE_NO_DECL;
      }

      // format output
      if (obj.Has("format") && obj.Get("format").IsBoolean() &&
          obj.Get("format").As<Napi::Boolean>().Value()) {
        options |= XML_SAVE_FORMAT;
      }

      // no empty tags (only XML)
      if (obj.Has("selfCloseEmpty") && obj.Get("selfCloseEmpty").IsBoolean() &&
          !obj.Get("selfCloseEmpty").As<Napi::Boolean>().Value()) {
        options |= XML_SAVE_NO_EMPTY;
      }

      // non-significant whitespace
      if (obj.Has("whitespace") && obj.Get("whitespace").IsBoolean() &&
          obj.Get("whitespace").As<Napi::Boolean>().Value()) {
        options |= XML_SAVE_WSNONSIG;
      }

      if (obj.Has("type") && obj.Get("type").IsString()) {
        std::string type_str = obj.Get("type").As<Napi::String>().Utf8Value();
        if (type_str == "XML" || type_str == "xml") {
          options |= XML_SAVE_AS_XML;
        } else if (type_str == "HTML" || type_str == "html") {
          options |= XML_SAVE_AS_HTML;
          if ((options & XML_SAVE_FORMAT) && !(options & XML_SAVE_XHTML)) {
            options |= XML_SAVE_XHTML;
          }
        } else if (type_str == "XHTML" || type_str == "xhtml") {
          options |= XML_SAVE_XHTML;
        }
      }
    }
  }

  return to_string(env, options);
}

Napi::Value XmlNode::Remove_Method(const Napi::CallbackInfo& info) {
  remove();
  return Value_();
}

Napi::Value XmlNode::Clone_Method(const Napi::CallbackInfo& info) {
  bool recurse = true;
  if (info.Length() == 1 && info[0].IsBoolean()) {
    recurse = info[0].As<Napi::Boolean>().Value();
  }
  return clone(info.Env(), recurse);
}

} // namespace libxmljs
