import * as events from 'node:events';
import bindings from './bindings';

export function SaxParser(callbacks?: Record<string, (...args: any[]) => void>) {
  const parser = new bindings.SaxParser();

  if (callbacks) {
    for (const callback in callbacks) {
      parser.on(callback, callbacks[callback]);
    }
  }

  return parser;
}

// Copy EventEmitter methods onto the native prototype
for (const k in events.EventEmitter.prototype) {
  (bindings.SaxParser.prototype as any)[k] = (
    events.EventEmitter.prototype as any
  )[k];
}

export function SaxPushParser(
  callbacks?: Record<string, (...args: any[]) => void>
) {
  const parser = new bindings.SaxPushParser();

  if (callbacks) {
    for (const callback in callbacks) {
      parser.on(callback, callbacks[callback]);
    }
  }

  return parser;
}

// Copy EventEmitter methods onto the native prototype
for (const k in events.EventEmitter.prototype) {
  (bindings.SaxPushParser.prototype as any)[k] = (
    events.EventEmitter.prototype as any
  )[k];
}
