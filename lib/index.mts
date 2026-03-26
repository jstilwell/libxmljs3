import { createRequire } from 'node:module';

const require = createRequire(import.meta.url);
const lib = require('./index.js');

export const parseXml = lib.parseXml;
export const parseHtml = lib.parseHtml;
export const parseHtmlFragment = lib.parseHtmlFragment;

export const version = lib.version;
export const libxml_version = lib.libxml_version;
export const libxml_parser_version = lib.libxml_parser_version;
export const libxml_debug_enabled = lib.libxml_debug_enabled;
export const features = lib.features;

export const Document = lib.Document;
export const Element = lib.Element;
export const Comment = lib.Comment;
export const Text = lib.Text;
export const ProcessingInstruction = lib.ProcessingInstruction;

export const SaxParser = lib.SaxParser;
export const SaxPushParser = lib.SaxPushParser;
export const SaxTransform = lib.SaxTransform;

export const memoryUsage = lib.memoryUsage;
export const nodeCount = lib.nodeCount;

export const TextWriter = lib.TextWriter;

export default lib;
