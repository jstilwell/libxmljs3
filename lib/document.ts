import bindings from './bindings';
import { Element } from './element';
import type { ParserOptions } from './types';

function assertRoot(doc: any): void {
  if (!doc.root()) {
    throw new Error('Document has no root element');
  }
}

const Document = bindings.Document;
Document.prototype = bindings.Document.prototype;

Document.prototype.node = function node(
  this: any,
  name: string,
  content?: string
) {
  return this.root(Element(this, name, content));
};

Document.prototype.find = function find(
  this: any,
  xpath: string,
  ns_uri?: string | Record<string, string>
) {
  assertRoot(this);
  return this.root().find(xpath, ns_uri);
};

Document.prototype.get = function get(
  this: any,
  xpath: string,
  ns_uri?: string | Record<string, string>
) {
  assertRoot(this);
  return this.root().get(xpath, ns_uri);
};

Document.prototype.child = function child(this: any, id: number) {
  if (id === undefined || typeof id !== 'number') {
    throw new Error('id argument required for #child');
  }
  assertRoot(this);
  return this.root().child(id);
};

Document.prototype.childNodes = function childNodes(this: any) {
  assertRoot(this);
  return this.root().childNodes();
};

Document.prototype.setDtd = function setDtd(
  this: any,
  name: string,
  ext?: string,
  sys?: string
) {
  if (!name) {
    throw new Error('Must pass in a DTD name');
  } else if (typeof name !== 'string') {
    throw new Error('Must pass in a valid DTD name');
  }

  const params: string[] = [name];

  if (typeof ext !== 'undefined') {
    params.push(ext);
  }
  if (ext && typeof sys !== 'undefined') {
    params.push(sys);
  }

  return this._setDtd(...params);
};

Document.prototype.namespaces = function namespaces(this: any) {
  assertRoot(this);
  return this.root().namespaces();
};

export default Document;

export function fromHtml(
  string: string,
  opts: ParserOptions = {}
): typeof Document {
  if (typeof opts !== 'object') {
    throw new Error('fromHtml options must be an object');
  }
  return bindings.fromHtml(string, opts);
}

export function fromHtmlFragment(
  string: string,
  opts: ParserOptions = {}
): typeof Document {
  if (typeof opts !== 'object') {
    throw new Error('fromHtmlFragment options must be an object');
  }
  (opts as any).doctype = false;
  (opts as any).implied = false;
  return bindings.fromHtml(string, opts);
}

export function fromXml(
  string: string,
  options: ParserOptions = {}
): typeof Document {
  return bindings.fromXml(string, options);
}
