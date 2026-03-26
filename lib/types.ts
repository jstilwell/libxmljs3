import { EventEmitter } from 'node:events';

export interface StringMap {
  [key: string]: string;
}

export interface ParserOptions {
  recover?: boolean;
  noent?: boolean;
  dtdload?: boolean;
  doctype?: boolean;
  dtdattr?: any;
  dtdvalid?: boolean;
  noerror?: boolean;
  errors?: boolean;
  nowarning?: boolean;
  warnings?: boolean;
  pedantic?: boolean;
  noblanks?: boolean;
  blanks?: boolean;
  sax1?: boolean;
  xinclude?: boolean;
  nonet?: boolean;
  net?: boolean;
  nodict?: boolean;
  dict?: boolean;
  nsclean?: boolean;
  implied?: boolean;
  nocdata?: boolean;
  cdata?: boolean;
  noxincnode?: boolean;
  compact?: boolean;
  old?: boolean;
  nobasefix?: boolean;
  basefix?: boolean;
  huge?: boolean;
  oldsax?: boolean;
  ignore_enc?: boolean;
  big_lines?: boolean;
  baseUrl?: string;
  encoding?: string;
  excludeImpliedElements?: boolean;
}

export interface SyntaxError extends Error {
  domain: number | null;
  code: number | null;
  level: number | null;
  file: string | null;
  line: number | null;
  column: number;
  str1: number | null;
  str2: number | null;
  str3: number | null;
  int1: number | null;
}

export interface ValidationError extends Error {
  domain: number | null;
  code: number | null;
  level: number | null;
  line: number | null;
  column: number;
}

export interface ToStringOptions {
  declaration?: boolean;
  selfCloseEmpty?: boolean;
  whitespace?: boolean;
  format?: boolean;
  type?: 'xml' | 'html' | 'xhtml';
  encoding?: 'HTML' | 'ASCII' | 'UTF-8' | 'UTF-16' | 'ISO-Latin-1' | 'ISO-8859-1';
}

// Type declarations for the native C++ bindings
export interface NativeBindings {
  Document: NativeDocumentConstructor;
  Element: NativeElementConstructor;
  Comment: NativeCommentConstructor;
  Text: NativeTextConstructor;
  ProcessingInstruction: NativePIConstructor;
  Namespace: NativeNamespaceConstructor;
  SaxParser: NativeSaxParserConstructor;
  SaxPushParser: NativeSaxPushParserConstructor;
  TextWriter: NativeTextWriterConstructor;

  fromXml(source: string, options: ParserOptions): any;
  fromHtml(source: string, options: ParserOptions): any;

  libxml_version: string;
  libxml_parser_version: string;
  libxml_debug_enabled: boolean;
  features: Record<string, boolean>;

  xmlMemUsed(): number;
  xmlNodeCount(): number;
}

export interface NativeDocumentConstructor {
  new (version?: string, encoding?: string): any;
  prototype: any;
}

export interface NativeElementConstructor {
  new (doc: any, name: string, content?: string): any;
  prototype: any;
}

export interface NativeCommentConstructor {
  new (doc: any, content?: string): any;
}

export interface NativeTextConstructor {
  new (doc: any, content: string): any;
}

export interface NativePIConstructor {
  new (doc: any, name: string, content?: string): any;
}

export interface NativeNamespaceConstructor {
  new (node: any, prefix: string | null, href: string): any;
}

export interface NativeSaxParserConstructor extends EventEmitter {
  new (): any;
  prototype: any;
}

export interface NativeSaxPushParserConstructor extends EventEmitter {
  new (): any;
  prototype: any;
}

export interface NativeTextWriterConstructor {
  new (): any;
}
