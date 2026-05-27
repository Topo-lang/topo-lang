# Language Support Package Directory Convention

Standard directory layout for `topo-lang-<lang>/` packages.

Each package's second-level directories are named after **which `topo` tool they
serve** (`topo-<tool>/`), so component ownership is explicit. This mirrors the
authoritative table in the project root `CLAUDE.md` (§语言支持包).

## Structure

```
topo-lang-<lang>/
├── runtime/           Host-language native runtime (headers / crate / jar), imported by user code
├── topo-check/        Serves topo-check
│   ├── analysis/      AnalysisProvider + extract/ + catalog/ (unsafe APIs) + stub/ (body stubbing)
│   ├── runner/        Language-specific CheckRunner (subclass of CheckRunnerBase)
│   └── extractor/     topo-extract-<lang> function-body extraction subprocess (C++/Rust/Java only)
├── topo-build/        Compile driver (CppDriver / RustDriver / JavaDriver) — Python: topo-build-python executable
├── topo-init/         Project-template Provider (InitTemplateProvider)
├── topo-lsp/          Host LSP bridge (ClangdBridge / RustAnalyzerBridge / JdtBridge / PyrightBridge)
├── topo-transpile/    Code Emitter (AST → host source)
├── topo-debug/        Host debug-adapter glue for topo-debug
├── topo-profile/      Host trace-conversion glue for topo-profile
├── topo-lang/         LanguagePlugin registration entry (aggregates the above, injects into Registry)
├── test/              The package's own tests
└── examples/          Example projects
```

> **V8-family host exception (TypeScript, future JavaScript):** AST-level
> analysis, the tsserver LSP bridge, and source codegen common infrastructure
> live in `topo-v8/lib/` (`TsServer` / `Check` / `Codegen` / `Debug`), not in
> the per-language package. `topo-lang-typescript/` keeps only TS-specific glue
> (tsconfig-driven config, idiom differences, TS-specific catalog, spec
> fixtures, Plugin registration). It has no `topo-check/extractor/` subprocess —
> L2 deep analysis goes exclusively through the shared tsserver bridge.

## Directory responsibilities

### `runtime/`

Host-language native runtime imported by **user** code. C++: `#include
<topo/parallel.h>` etc. Rust: FFI crate (+ `runtime-macros/` proc-macro crate).
Java: Gradle project building `topo-runtime.jar`. Python / TypeScript:
check-only (README placeholder; no compiled runtime).

### `topo-check/`

Everything topo-check needs for this language:

- `analysis/` — `LanguageAnalysisProvider` (factory for extractors),
  symbol/import/call-site extractors under `extract/`, the language `catalog/`
  of unsafe API patterns, and `stub/` body-stubbing.
- `runner/` — language `CheckRunner` extending `topo::lang::CheckRunnerBase`,
  adding language-specific pre/post-processing around the common check
  algorithms.
- `extractor/` — the `topo-extract-<lang>` function-body extraction subprocess
  (only C++ / Rust / Java ship an executable here; Python uses in-process
  extraction; TypeScript uses the shared tsserver bridge).

### `topo-build/`

Compile-driver integration. C++/Rust/Java provide a `*Driver` library that the
backend tool links (e.g. `topo-build-jvm-java` links `TopoJavaDriver`). Python
has no driver — `topo-build/` directly holds the `topo-build-python`
executable source. TypeScript holds the check-only `topo-build-typescript`
executable source.

### `topo-init/`

`InitTemplateProvider` — generates `Topo.toml` and `.topo` files with the
language's type bindings.

### `topo-lsp/`

Bridge to the host language's LSP server (clangd / rust-analyzer / jdtls /
pyright). Used for semantic symbol extraction, deep containment analysis (L2),
and hover/definition queries for the topo-lsp proxy. (TypeScript's
`TsServerBridge` lives in `topo-v8/lib/TsServer/`, not here.)

### `topo-transpile/`

Emitter implementation generating host source from `TranspileModel`.

### `topo-debug/` / `topo-profile/`

Per-host glue for the `topo-debug` debugger frontend and `topo-profile` trace
normalizer (debug-adapter wiring, trace-format conversion).

### `topo-lang/`

`LanguagePlugin` registration entry — aggregates the components above and
injects them into the `topo-lang` Registry.

### `test/` / `examples/`

The package's own tests and example projects.

## CMake targets

Library targets follow the naming pattern below; the directory that owns each
target is the `topo-<tool>/` dir above. All targets link against `topo-core`
libraries. None link against `topo-llvm`.

| Component | Target name pattern |
|-----------|---------------------|
| `topo-check/analysis/` | `Topo{Lang}Analysis` (e.g. `TopoCppAnalysis`) |
| `topo-lsp/` | `Topo{Lang}LSP` (e.g. `TopoCppLSP`) |
| `topo-build/` | `Topo{Lang}Driver` (e.g. `TopoJavaDriver`) |
| `topo-check/extractor/` | `topo-extract-{lang}` executable (C++/Rust/Java) |
