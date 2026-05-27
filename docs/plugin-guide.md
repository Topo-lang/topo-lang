# Language Plugin Development Guide

This guide covers how to build a language support package that plugs into the Topo toolchain via `topo-lang/`.

## Overview

A language plugin aggregates all language-specific capabilities into a single `LanguagePlugin` subclass, which is registered in a global registry. The toolchain queries this registry by `HostLanguage` to obtain the correct implementations at runtime.

## Required interface: LanguageAnalysisProvider

Every plugin must implement `topo::check::LanguageAnalysisProvider` (defined in `topo-core/include/topo/Check/LanguageAnalysisProvider.h`). This is the only mandatory capability.

Provide concrete implementations of:

- **SymbolExtractor** — Extract function/type declarations from host source files
- **ImportExtractor** — Extract import/include statements with unsafe level classification
- **CallSiteExtractor** — Extract function call sites with unsafe level classification
- **collectSourceFiles()** — Scan a project directory for source files of this language

Optional LSP integration:

- **initLSP() / shutdownLSP() / isLSPReady()** — Manage an LSP bridge for semantic analysis
- **runDeepContainment()** — L2 deep containment analysis using LSP semantic data

## Optional: EmitterFactory

Implement `topo::lang::EmitterFactory` if the language supports transpilation output:

```cpp
class MyEmitterFactory : public topo::lang::EmitterFactory {
public:
    std::unique_ptr<transpile::Emitter> createEmitter() override;
    std::string fileExtension() const override; // e.g. ".rs"
};
```

## Optional: BuildDriverFactory

Implement `topo::lang::BuildDriverFactory` to declare which backend tools the build system should invoke:

```cpp
class MyBuildDriverFactory : public topo::lang::BuildDriverFactory {
public:
    std::string backendToolName() const override;    // e.g. "topo-build-llvm-rust"
    std::string extractorToolName() const override;  // e.g. "topo-extract-rust"
};
```

## Optional: InitTemplateProvider

Implement `topo::lang::InitTemplateProvider` for `topo init` project scaffolding:

```cpp
class MyInitTemplateProvider : public topo::lang::InitTemplateProvider {
public:
    std::string languageName() const override;                        // e.g. "rust"
    std::vector<std::string> filePatterns() const override;           // e.g. {"*.rs"}
    std::string generateTopoToml(const std::string& name) const override;
    std::string generateTypeBindings() const override;
};
```

## Aggregating into LanguagePlugin

Subclass `topo::lang::LanguagePlugin` to wire everything together:

```cpp
class RustPlugin : public topo::lang::LanguagePlugin {
public:
    HostLanguage language() const override { return HostLanguage::Rust; }

    std::unique_ptr<check::LanguageAnalysisProvider> createAnalysisProvider() override {
        return std::make_unique<RustAnalysisProvider>();
    }

    EmitterFactory* emitterFactory() override { return &emitterFactory_; }
    BuildDriverFactory* buildDriverFactory() override { return &buildDriverFactory_; }
    InitTemplateProvider* initTemplateProvider() override { return &initTemplateProvider_; }

private:
    RustEmitterFactory emitterFactory_;
    RustBuildDriverFactory buildDriverFactory_;
    RustInitTemplateProvider initTemplateProvider_;
};
```

## Registration

Register at startup (before any toolchain queries):

```cpp
topo::lang::registerPlugin(std::make_unique<RustPlugin>());
```

Lookup:

```cpp
auto* plugin = topo::lang::getPlugin(HostLanguage::Rust);
if (plugin) {
    auto provider = plugin->createAnalysisProvider();
    // ...
}
```

## Using CheckRunnerBase

`CheckRunnerBase` provides pre-built orchestration for completeness and containment checks. Subclass it in your language check runner to reuse the common logic:

```cpp
class RustCheckRunner : public topo::lang::CheckRunnerBase {
    // Add language-specific setup, then call
    // runCompleteness() and runContainment() from the base class.
};
```

## See also

- `topo-lang/examples/minimal-plugin/` — Complete working example
- `topo-lang/docs/directory-convention.md` — Standard directory layout
