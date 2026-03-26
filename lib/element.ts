import bindings from './bindings';
import type { StringMap } from './types';

export function Element(doc: any, name: string, content?: string): any {
  if (!doc) {
    throw new Error('document argument required');
  } else if (!(doc instanceof bindings.Document)) {
    throw new Error('document argument must be an instance of Document');
  } else if (!name) {
    throw new Error('name argument required');
  }

  return new bindings.Element(doc, name, content);
}

Element.prototype = bindings.Element.prototype;

Element.prototype.attr = function attr(this: any, ...args: any[]) {
  if (args.length === 1) {
    const arg = args[0];

    if (typeof arg === 'object') {
      for (const k in arg) {
        this._attr(k, (arg as StringMap)[k]);
      }
      return this;
    } else if (typeof arg === 'string') {
      return this._attr(arg);
    }
  } else if (args.length === 2) {
    this._attr(args[0], args[1]);
    return this;
  }
};

Element.prototype.node = function node(
  this: any,
  name: string,
  content?: string
) {
  const elem = Element(this.doc(), name, content);
  this.addChild(elem);
  return elem;
};

Element.prototype.get = function get(this: any, ...args: any[]) {
  const res = this.find(...args);

  if (Array.isArray(res)) {
    return res[0];
  }

  return res;
};

Element.prototype.defineNamespace = function defineNamespace(
  this: any,
  prefix: string,
  href?: string
) {
  if (!href) {
    href = prefix;
    prefix = null as any;
  }
  return new bindings.Namespace(this, prefix, href);
};
