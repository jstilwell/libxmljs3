import bindings from './bindings';
import Document, { fromXml, fromHtml, fromHtmlFragment } from './document';
import { Element } from './element';
import { SaxParser, SaxPushParser } from './sax_parser';

// Re-export types
export type {
  ParserOptions,
  StringMap,
  SyntaxError,
  ValidationError,
  ToStringOptions,
} from './types';

// Parse functions
export const parseXml = fromXml;
export const parseHtml = fromHtml;
export { fromHtmlFragment as parseHtmlFragment };

// Constants
export const version: string = require('../package.json').version;
export const libxml_version: string = bindings.libxml_version;
export const libxml_parser_version: string = bindings.libxml_parser_version;
export const libxml_debug_enabled: boolean = bindings.libxml_debug_enabled;
export const features: Record<string, boolean> = bindings.features;

// Classes
export const Comment = bindings.Comment;
export { Document };
export { Element };
export const ProcessingInstruction = bindings.ProcessingInstruction;
export const Text = bindings.Text;

// Static helpers on Document
(Document as any).fromXml = fromXml;
(Document as any).fromHtml = fromHtml;
(Document as any).fromHtmlFragment = fromHtmlFragment;

// SAX parsers
export { SaxParser, SaxPushParser };
export { SaxTransform } from './sax_transform';
export type { SaxTransformOptions } from './sax_transform';

// Utilities
export const memoryUsage = bindings.xmlMemUsed;
export const nodeCount = bindings.xmlNodeCount;

// TextWriter
export const TextWriter = bindings.TextWriter;
