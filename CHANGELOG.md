# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.0] - 2026-07-28

### Breaking

- **`engines.node` narrowed to `^22.22.2 || ^24.15.0 || >=26`** (previously
  `>=22`). This matches the range required by node-gyp 13. Node 22.0 through
  22.22.1 satisfied the old range and no longer do.
- **`node-gyp` moved from `dependencies` to `devDependencies`.** It is only
  used by the build scripts; the runtime addon-loading path goes through
  `node-gyp-build`. Consumers no longer download node-gyp on install. Anyone
  who was relying on libxmljs4 to transitively provide node-gyp now needs to
  depend on it directly.

### Fixed

- **`namespace()` no longer aborts the process when given a non-string href.**
  `el.namespace('bar', null)` terminated the process with
  `FATAL ERROR: Error::New napi_get_last_error_info` instead of throwing.

  `XmlNamespace`'s constructor correctly rejected the missing href, but this
  addon is built with `NAPI_DISABLE_CPP_EXCEPTIONS`, so
  `ThrowAsJavaScriptException()` only schedules a JS exception rather than
  unwinding. `Namespace_Method` then unwrapped the half-constructed object
  with that exception still pending, which is a fatal N-API error.

  The href is now validated up front, and a pending exception is checked for
  after construction. Invalid hrefs throw a `TypeError`. The same abort was
  reachable via `namespace(undefined)`, `namespace(42)`, and `namespace({})`;
  all now throw. The single-argument `namespace(null)` form still removes the
  namespace as documented.

- **Prebuilt binaries now cover every supported platform.** Releases before
  this one published prebuilds for `darwin-arm64` only (and 1.0.0 shipped only
  a stale `libxmljs3.node`, so it had no loadable prebuild at all under the
  current package name). Everyone else silently compiled from source on
  install, which requires python and a C++ toolchain. The release now ships
  prebuilds for macOS arm64, Linux x64/arm64 (glibc), Linux x64 (musl), and
  Windows x64/arm64.
- **Alpine/musl prebuilds are tagged correctly.** `prebuildify` now runs with
  `--tag-libc`. Without it, the musl and glibc Linux x64 builds were both
  named `linux-x64/libxmljs4.node`, so only one survived packaging and a musl
  user could receive a glibc binary that fails to load.

### Changed

- Upgraded to **TypeScript 7**. Both tsconfigs now set `"types": ["node"]`,
  which TypeScript 6 and 7 require since the `types` default became `[]`. The
  emitted `.d.ts` files are byte-identical to the TypeScript 5.9 output.
- Upgraded to **ESLint 10**. `no-useless-assignment` (new in
  `eslint:recommended`) is disabled for `test/**`, where variables are
  deliberately reassigned to drop references so the GC can collect the
  underlying native objects.
- Upgraded to **Vitest 4**. `test.poolOptions` was removed upstream, so
  `execArgv: ['--expose_gc']` moved to the top level of the `test` config.
- Upgraded node-gyp (11 → 13), globals (16 → 17), node-addon-api (8.6 → 8.9),
  prettier (3.5 → 3.9), tsd (0.32 → 0.33), and `@types/node` (22.15 → 22.20).

  `@types/node` intentionally stays on 22.x to track the minimum supported
  Node version rather than the latest, because the published `.d.ts` files
  re-export `node:stream` and `node:events` types to consumers.

### Known issues

- `test/memory_management.test.js` and `test/ref_integrity.test.js` remain
  excluded from the default test run. They no longer abort the process, but
  still have unresolved memory-accounting failures.
- The vendored libxml2 is **2.9.9** (January 2019) and carries known
  unpatched CVEs, including CVE-2022-40304, CVE-2021-3517, CVE-2021-3541, and
  CVE-2021-3518. Because it is compiled into the addon rather than resolved as
  a package, `npm audit` does not surface it. Upgrading is non-trivial:
  libxml2 2.12–2.14 moved error handling to thread-local and per-context
  handlers and changed `xmlGetLastError` to return a `const` pointer, which
  the current global-handler usage in `src/xml_document.cc` depends on.
  Treat untrusted XML, XInclude, and Schematron input with caution.

## [1.2.0] - 2026-03-26

### Changed

- Optimized native `toString()`: element and node option parsing now caches
  property lookups instead of repeatedly querying the options object.
- Reduced published package size by disabling source maps and declaration
  maps in the shipped `dist` files.

## [1.1.0] - 2026-03-26

### Changed

- Documentation updates.

## [1.0.0] - 2026-03-25

Initial release of the fork, from
[libxmljs2](https://github.com/marudor/libxmljs2).

### Security

- **Fixed CVE-2024-34393 and CVE-2024-34394** — type confusion vulnerabilities
  in `attrs()` and `namespaces()` that could lead to denial of service, data
  leakage, or remote code execution. The C++ binding layer was rewritten from
  NAN to node-addon-api with type-safe wrapping, removing the class of unsafe
  pointer casts that caused them.

### Breaking

- **Removed compatibility synonyms**: `parseXmlString` (use `parseXml`),
  `parseHtmlString` (use `parseHtml`), `Document.fromXmlString` (use
  `Document.fromXml`), `Document.fromHtmlString` (use `Document.fromHtml`).
- **Renamed package**: published as `libxmljs4`.

### Changed

- **Node-API (N-API)**: the native addon uses ABI-stable Node-API instead of
  NAN, so prebuilt binaries work across Node.js versions without
  recompilation.
- **TypeScript source**: the JS wrapper layer is written in TypeScript with
  generated type definitions.
- **ESM support**: both `require()` and `import` work via the `exports` field.
- **Replaced the `bindings` package**: native addon loading no longer depends
  on it.

[2.0.0]: https://github.com/jstilwell/libxmljs4/compare/v1.2.0...v2.0.0
[1.2.0]: https://github.com/jstilwell/libxmljs4/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/jstilwell/libxmljs4/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/jstilwell/libxmljs4/releases/tag/v1.0.0
