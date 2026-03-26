// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_NODE_H_
#define SRC_XML_NODE_H_

#include <napi.h>
#include <libxml/tree.h>

namespace libxmljs {

// XmlNode is a plain C++ base class (NOT an ObjectWrap itself).
// Each concrete subclass (XmlElement, XmlText, XmlComment, etc.) inherits
// from BOTH Napi::ObjectWrap<ConcreteType> AND XmlNode.
//
// This avoids the CRTP collision that would result from trying to have
// XmlNode : Napi::ObjectWrap<XmlNode> with XmlElement : XmlNode, because
// node-addon-api does not support inheriting ObjectWrap through a hierarchy.
class XmlNode {
public:
  xmlNode *xml_obj;

  // Closest wrapped ancestor in the libxml tree (used for ref-counting).
  xmlNode *ancestor;

  // The libxml document this node belongs to.
  xmlDoc *doc;

  explicit XmlNode(xmlNode *node);
  virtual ~XmlNode();

  // Called by each concrete subclass constructor after ObjectWrap construction
  // so that XmlNode::Unwrap(env, obj) can retrieve the XmlNode* from any
  // node JS wrapper without byte-offset arithmetic.
  void attach(Napi::Object wrapper);

  // Ancestor reference management — keeps the ancestor's JS wrapper alive.
  void ref_wrapped_ancestor();
  void unref_wrapped_ancestor();
  xmlNode *get_wrapped_ancestor();

  // Internal tree-walk helpers used by the destructor and ref management.
  // Exposed as static members so subclass .cc files can use them if needed.
  static xmlNode *get_wrapped_ancestor_or_root(xmlNode *node);
  static xmlNode *get_wrapped_descendant(xmlNode *node,
                                         xmlNode *skip = nullptr);

  // Pure virtual interface that each concrete ObjectWrap subclass implements.
  // Value_() returns the JS object for this wrapper.
  // Ref_() / Unref_() / refs_() delegate to Napi::ObjectWrap<T>::Ref/Unref.
  virtual Napi::Object Value_() = 0;
  virtual void Ref_() = 0;
  virtual void Unref_() = 0;
  virtual int refs_() = 0;

  // Factory: create the correct concrete wrapper (XmlElement, XmlText, etc.)
  // for the given libxml node.
  static Napi::Value New(Napi::Env env, xmlNode *node);

  // Cross-subclass unwrap helper.  Given a JS object that wraps any concrete
  // XmlNode subclass, return the XmlNode* by reading xml_obj->_private.
  // Returns nullptr if the object is not a valid node wrapper.
  static XmlNode *Unwrap(Napi::Env env, Napi::Object obj);

  // -----------------------------------------------------------------------
  // Helper methods (pure C++ logic, called by the Method_ wrappers below).
  // -----------------------------------------------------------------------
  Napi::Value get_doc(Napi::Env env);
  Napi::Value remove_namespace(Napi::Env env);
  Napi::Value get_namespace(Napi::Env env);
  void set_namespace(xmlNs *ns);
  xmlNs *find_namespace(const char *search_str);
  Napi::Value get_all_namespaces(Napi::Env env);
  Napi::Value get_local_namespaces(Napi::Env env);
  Napi::Value get_parent(Napi::Env env);
  Napi::Value get_prev_sibling(Napi::Env env);
  Napi::Value get_next_sibling(Napi::Env env);
  Napi::Value get_line_number(Napi::Env env);
  Napi::Value clone(Napi::Env env, bool recurse);
  Napi::Value get_type(Napi::Env env);
  Napi::Value to_string(Napi::Env env, int options = 0);
  void remove();
  void add_child(xmlNode *child);
  void add_prev_sibling(xmlNode *node);
  void add_next_sibling(xmlNode *node);
  xmlNode *import_node(xmlNode *node);

  // replace_element / replace_text are subclass-specific and not declared here.
  // XmlElement and XmlText each declare and implement their own versions.

  // -----------------------------------------------------------------------
  // Prototype method implementations.  Each concrete subclass registers these
  // on its own prototype by delegating from its own InstanceMethod callbacks.
  //
  // Example in XmlElement:
  //   Napi::Value Doc(const Napi::CallbackInfo& info) {
  //     return XmlNode::Doc_Method(info);
  //   }
  // -----------------------------------------------------------------------
  Napi::Value Doc_Method(const Napi::CallbackInfo& info);
  Napi::Value Namespace_Method(const Napi::CallbackInfo& info);
  Napi::Value Namespaces_Method(const Napi::CallbackInfo& info);
  Napi::Value Parent_Method(const Napi::CallbackInfo& info);
  Napi::Value PrevSibling_Method(const Napi::CallbackInfo& info);
  Napi::Value NextSibling_Method(const Napi::CallbackInfo& info);
  Napi::Value LineNumber_Method(const Napi::CallbackInfo& info);
  Napi::Value Type_Method(const Napi::CallbackInfo& info);
  Napi::Value ToString_Method(const Napi::CallbackInfo& info);
  Napi::Value Remove_Method(const Napi::CallbackInfo& info);
  Napi::Value Clone_Method(const Napi::CallbackInfo& info);
};

} // namespace libxmljs

#endif // SRC_XML_NODE_H_
