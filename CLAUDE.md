# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

libxmljs4 is a Node.js native addon providing LibXML2 bindings for JavaScript. Published as `libxmljs4` on npm. Forked from [libxmljs2](https://github.com/marudor/libxmljs2). It wraps the C library libxml2 (vendored in `vendor/libxml/`) via C++ using node-addon-api (N-API), with a TypeScript wrapper layer on top.

## Build & Test Commands

```bash
# Build the native addon (produces build/Release/xmljs.node)
pnpm run build          # or: node-gyp rebuild
make all                # alternative: clean + configure + build

# Compile TypeScript (CJS + ESM) to dist/
pnpm run build:ts

# Run all tests
# vitest.config.js passes --expose_gc to the forked workers, which the GC
# tests require; there is no need to pass it on the command line.
pnpm test               # runs: vitest run

# Run a single test file
pnpm test test/element.test.js

# Run tests matching a pattern
pnpm test -- -t "pattern"

# Type-check the .d.ts definitions
pnpm tsd

# Lint
pnpm lint

# Build prebuilt binaries (for CI)
pnpm prebuild
```

You must rebuild the native addon (`pnpm run build`) after any changes to C++ files in `src/` or vendor code. You must recompile TypeScript (`pnpm run build:ts`) after any changes to `lib/*.ts`.

## Architecture

```
lib/                     → TypeScript source (compiled to dist/)
  index.ts               → Main entry point, exports public API
  index.mts              → ESM wrapper (uses createRequire for native addon)
  bindings.ts            → Loads native addon via node-gyp-build
  document.ts            → Document class (augments native prototype)
  element.ts             → Element class (augments native prototype)
  sax_parser.ts          → SaxParser / SaxPushParser wrappers
  sax_transform.ts       → SaxTransform (Transform stream wrapper)
  types.ts               → Shared TypeScript interfaces
dist/                    → Compiled output (gitignored)
  index.js / index.d.ts  → CJS entry
  index.mjs / index.d.mts → ESM entry
src/                     → C++ native bindings using node-addon-api (N-API)
  libxmljs.cc            → Module init, memory tracking (napi_adjust_external_memory)
  xml_node.cc/.h         → Base node class (plain C++, NOT ObjectWrap)
  xml_document.cc/.h     → Document (Napi::ObjectWrap<XmlDocument>)
  xml_element.cc/.h      → Element (Napi::ObjectWrap<XmlElement> + XmlNode)
  xml_sax_parser.cc/.h   → SAX parsing (two ObjectWrap classes + shared base)
  xml_textwriter.cc/.h   → XML serialization
  xml_xpath_context.cc   → XPath queries
  (+ attribute, comment, text, pi, namespace)
vendor/libxml/           → Embedded libxml2 C source
index.js                 → Thin CJS re-export of dist/index.js (for test compat)
```

### C++ Class Hierarchy (node-addon-api)

`XmlNode` is a plain C++ base class (NOT ObjectWrap) with pure virtual methods `Value_()`, `Ref_()`, `Unref_()`. Each concrete type uses multiple inheritance:
```
XmlElement : public Napi::ObjectWrap<XmlElement>, public XmlNode
XmlText    : public Napi::ObjectWrap<XmlText>,    public XmlNode
...
```
This avoids the CRTP collision that prevents `Napi::ObjectWrap<Base>` → `Napi::ObjectWrap<Derived>` inheritance.

### Memory Management

`src/libxmljs.cc` replaces libxml2's memory allocator with custom wrappers that call `napi_adjust_external_memory()`, letting V8's GC account for native memory usage. A global `napi_env` is stored during module init and cleared via cleanup hook. Tests verify this with `--expose_gc` and explicit GC cycling in `test/setup.js`.

## Native Build System

- `binding.gyp` defines the GYP build: compiles all `src/*.cc` and `vendor/libxml/*.c` into `xmljs.node`
- Uses `node-addon-api` for ABI-stable binaries (one build works across Node versions)
- Prebuilt binaries distributed via `prebuildify` / `node-gyp-build`
- CI builds prebuilts for Windows, macOS, Ubuntu, and Alpine

## Dual CJS/ESM Package

- CJS: `dist/index.js` (compiled from `lib/index.ts`)
- ESM: `dist/index.mjs` (compiled from `lib/index.mts`, uses `createRequire` for native addon)
- Configured via `exports` field in package.json

## Fork Maintenance

When making any change that diverges from the upstream libxmljs2 behavior (API changes, removed features, new defaults, breaking changes), update the "Changes from libxmljs2" section in `README.md`. This helps users migrating from libxmljs2 understand what differs. Even seemingly minor changes should be documented if they could affect existing code.

## Code Style

- Prettier: single quotes, 2-space indent, 80 char width, ES5 trailing commas
- ESLint: flat config (`eslint.config.js`), `@eslint/js` recommended + prettier
- EditorConfig: LF line endings, 2-space indentation

## Known Issues

- 2 test suites (ref_integrity, memory_management) are excluded from the default run via `vitest.config.js`. They have unresolved memory-accounting failures: some assertions comparing `libxml.memoryUsage()` before and after a GC cycle do not hold, and a few tests time out waiting for memory to be reclaimed. They no longer abort the process — that was a separate bug in `XmlNode::Namespace_Method`, fixed in 2.0.0.
- The vendored libxml2 is 2.9.9 (January 2019) and has known unpatched CVEs. Because it is compiled into the addon rather than resolved as a package, `npm audit` does not see it. Upgrading is a real project, not a file swap: libxml2 2.12–2.14 moved error handling to thread-local and per-context handlers and made `xmlGetLastError` return a `const` pointer, which the global-handler pattern used at ~13 sites in `src/xml_document.cc` depends on. 2.14 also broke ABI and changed CDATA merging, which would alter user-visible `text()` results.
