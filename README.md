# Libxmljs3

LibXML bindings for [node.js](http://nodejs.org/)

Forked from [libxmljs2](https://github.com/marudor/libxmljs2), which was forked from the original [libxmljs](https://github.com/libxmljs/libxmljs). This fork was created to address unpatched security vulnerabilities ([CVE-2024-34393](https://nvd.nist.gov/vuln/detail/CVE-2024-34393), [CVE-2024-34394](https://nvd.nist.gov/vuln/detail/CVE-2024-34394)) in the unmaintained libxmljs2, and to modernize the project for current Node.js.

TypeScript definitions are included out of the box.

## Installation

```shell
npm install libxmljs3
```

Prebuilt binaries are available for Windows, macOS, Linux, and Alpine. If a prebuild is not available for your platform, it will compile from source automatically (requires Python 3, `make`, and a C++ compiler — see [node-gyp prerequisites](https://github.com/TooTallNate/node-gyp#installation)).

**Node.js >= 22 is required.**

## Migrating from libxmljs2

libxmljs3 starts at version `1.0.0` (forked from libxmljs2 `0.37.0`). The major version bump reflects breaking changes: removed API synonyms, a fully rewritten C++ binding layer, and a new package name. Update your install and imports:

```diff
- npm install libxmljs2
+ npm install libxmljs3
```

```diff
- const libxmljs = require('libxmljs2');
+ const libxmljs = require('libxmljs3');
```

### Changes from libxmljs2

- **Security: CVE-2024-34393 and CVE-2024-34394 fixed** — The type confusion vulnerabilities in `attrs()` and `namespaces()` (which could lead to denial of service, data leakage, or remote code execution) have been eliminated. The entire C++ binding layer was rewritten from NAN to node-addon-api with type-safe wrapping, removing the class of unsafe pointer casts that caused these vulnerabilities.
- **Node-API (N-API)**: Native addon now uses ABI-stable Node-API instead of NAN. Prebuilt binaries work across Node.js versions without recompilation.
- **TypeScript source**: The JS wrapper layer is now written in TypeScript with generated type definitions.
- **ESM support**: Both `require()` and `import` work via the `exports` field in package.json.
- **Removed compatibility synonyms**: `parseXmlString` (use `parseXml`), `parseHtmlString` (use `parseHtml`), `Document.fromXmlString` (use `Document.fromXml`), `Document.fromHtmlString` (use `Document.fromHtml`).
- **Replaced `bindings` package**: Native addon loading no longer depends on the `bindings` npm package.

## API Overview

### Parsing XML

```javascript
const libxmljs = require('libxmljs3');

const xmlDoc = libxmljs.parseXml(
  '<?xml version="1.0"?>' +
    '<root>' +
    '<child foo="bar">' +
    '<grandchild baz="fizbuzz">grandchild content</grandchild>' +
    '</child>' +
    '<sibling>with content!</sibling>' +
    '</root>'
);

console.log(xmlDoc.get('//grandchild').text()); // "grandchild content"
console.log(xmlDoc.root().childNodes()[0].attr('foo').value()); // "bar"
```

### Parsing HTML

```javascript
const htmlDoc = libxmljs.parseHtml('<html><body><p>Hello</p></body></html>');
const htmlFragment = libxmljs.parseHtmlFragment('<p>Hello</p><p>World</p>');
```

### Parser Options

Both `parseXml` and `parseHtml` accept an options object:

```javascript
const doc = libxmljs.parseXml(xml, {
  recover: true, // Recover from malformed XML
  noent: true, // Substitute entities
  noblanks: true, // Remove blank text nodes
  nonet: true, // Disable network access (default-like behavior)
  huge: true, // Allow parsing very large documents
  baseUrl: 'http://example.com', // Base URL for relative references
});
```

Other options: `dtdload`, `dtdattr`, `dtdvalid`, `noerror`, `nowarning`, `pedantic`, `sax1`, `xinclude`, `nodict`, `nsclean`, `nocdata`, `compact`, `old`, `nobasefix`, `big_lines`, `ignore_enc`.

### XPath Queries

```javascript
// Simple query
const nodes = doc.find('//child');
const node = doc.get('//child');

// With a default namespace URI
const divs = doc.find('//xmlns:div', 'http://www.w3.org/1999/xhtml');

// With a namespace map
const results = doc.find('//ex:body', { ex: 'urn:example' });
```

### Building Documents

```javascript
const doc = new libxmljs.Document();
doc
  .node('root')
  .node('child', 'content')
  .attr({ foo: 'bar' })
  .parent()
  .node('sibling', 'more content');

console.log(doc.toString());
```

### SAX Parser

Event-based parsing for large documents or streaming use cases.

```javascript
const parser = new libxmljs.SaxParser();

parser.on('startElementNS', (name, attrs, prefix, uri, namespaces) => {
  console.log('opened:', name);
});

parser.on('endElementNS', (name, prefix, uri) => {
  console.log('closed:', name);
});

parser.on('characters', (text) => {
  console.log('text:', text);
});

parser.parseString('<root><child>hello</child></root>');
```

**Events:** `startDocument`, `endDocument`, `startElementNS`, `endElementNS`, `characters`, `cdata`, `comment`, `warning`, `error`

The `SaxPushParser` works the same way but accepts incremental chunks via `.push(chunk)`.

### TextWriter

Serialize XML programmatically with fine-grained control.

```javascript
const writer = new libxmljs.TextWriter();

writer.startDocument('1.0', 'UTF-8');
writer.startElementNS(undefined, 'root', 'http://example.com');
writer.startElementNS(undefined, 'child');
writer.writeString('content');
writer.endElement();
writer.endElement();
writer.endDocument();

console.log(writer.outputMemory()); // flushes and returns the XML string
```

### XSD and Schematron Validation

```javascript
const xsdDoc = libxmljs.parseXml(xsdString);
const isValid = xmlDoc.validate(xsdDoc);
console.log(xmlDoc.validationErrors);

const schematronDoc = libxmljs.parseXml(schematronString);
const isValid2 = xmlDoc.schematronValidate(schematronDoc);
```

### Utilities

```javascript
libxmljs.version; // libxmljs3 package version
libxmljs.libxml_version; // Underlying libxml2 version
libxmljs.memoryUsage(); // Bytes allocated by libxml2
libxmljs.nodeCount(); // Number of live XML nodes
```

## Support

- [Wiki](https://github.com/jstilwell/libxmljs3/wiki)
- [Examples](https://github.com/jstilwell/libxmljs3/tree/main/examples)

## Contributing

Start by checking out the [open issues](https://github.com/jstilwell/libxmljs3/issues).

### Build from Source

Prerequisites: Python 3, `make`, C++ compiler (`g++` or equivalent).

```shell
git clone https://github.com/jstilwell/libxmljs3.git
cd libxmljs3
pnpm install --build-from-source
pnpm test
```

Tests require the `--expose_gc` Node.js flag (already configured in `npm test`).
