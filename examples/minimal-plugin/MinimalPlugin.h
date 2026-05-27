#ifndef TOPO_EXAMPLE_MINIMALPLUGIN_H
#define TOPO_EXAMPLE_MINIMALPLUGIN_H

#include "topo/Lang/BuildDriverFactory.h"
#include "topo/Lang/EmitterFactory.h"
#include "topo/Lang/InitTemplateProvider.h"
#include "topo/Lang/LanguagePlugin.h"

#include "topo/Check/LanguageAnalysisProvider.h"
#include "topo/Transpile/Emitter.h"

namespace example {

// -- Stub analysis provider (returns empty results) --

class MinimalAnalysisProvider : public topo::check::LanguageAnalysisProvider {
public:
    std::unique_ptr<topo::check::SymbolExtractor> createSymbolExtractor() override {
        return nullptr; // Real plugin would return a concrete extractor
    }
    std::unique_ptr<topo::check::ImportExtractor> createImportExtractor() override {
        return nullptr;
    }
    std::unique_ptr<topo::check::CallSiteExtractor> createCallSiteExtractor() override {
        return nullptr;
    }
    std::vector<std::string> collectSourceFiles(
        const std::string& /*projectDir*/,
        const std::vector<std::string>& /*includeDirs*/) const override {
        return {};
    }
};

// -- Stub emitter --

class MinimalEmitter : public topo::transpile::Emitter {
public:
    topo::transpile::EmitResult emit(const topo::transpile::TranspileModule& /*module*/) override {
        return {"// minimal plugin output\n", {}};
    }
};

// -- Factory implementations --

class MinimalEmitterFactory : public topo::lang::EmitterFactory {
public:
    std::unique_ptr<topo::transpile::Emitter> createEmitter() override {
        return std::make_unique<MinimalEmitter>();
    }
    std::string fileExtension() const override { return ".txt"; }
};

class MinimalBuildDriverFactory : public topo::lang::BuildDriverFactory {
public:
    std::string backendToolName() const override { return "topo-build-minimal"; }
    std::string extractorToolName() const override { return "topo-extract-minimal"; }
};

class MinimalInitTemplateProvider : public topo::lang::InitTemplateProvider {
public:
    std::string languageName() const override { return "minimal"; }
    std::vector<std::string> filePatterns() const override { return {"*.min"}; }
    std::string generateTopoToml(const std::string& projectName) const override {
        return "[topo]\nroot = \"" + projectName + ".topo\"\n\n"
               "[build]\nlanguage = \"minimal\"\nsources = \"src\"\n";
    }
    std::string generateTypeBindings() const override {
        return "// Minimal language type bindings\n";
    }
    // ISSUE: sourceFileGlob() was added to InitTemplateProvider in f7be290
    // (Apr 2026) but the minimal-plugin example fixture was not updated.
    // Without this override MinimalInitTemplateProvider is abstract and
    // MinimalPlugin cannot be instantiated — the example executable fails
    // to compile. The example is not wired into the main build so the
    // breakage has been silent. Fixing here to unblock topo-lang unit tests.
    std::string sourceFileGlob() const override { return "src/**/*.min"; }
};

// -- Plugin --

class MinimalPlugin : public topo::lang::LanguagePlugin {
public:
    topo::HostLanguage language() const override { return topo::HostLanguage::Cpp; }

    std::unique_ptr<topo::check::LanguageAnalysisProvider> createAnalysisProvider() override {
        return std::make_unique<MinimalAnalysisProvider>();
    }

    topo::lang::EmitterFactory* emitterFactory() override { return &emitterFactory_; }
    topo::lang::BuildDriverFactory* buildDriverFactory() override { return &buildDriverFactory_; }
    topo::lang::InitTemplateProvider* initTemplateProvider() override { return &initTemplateProvider_; }

private:
    MinimalEmitterFactory emitterFactory_;
    MinimalBuildDriverFactory buildDriverFactory_;
    MinimalInitTemplateProvider initTemplateProvider_;
};

} // namespace example

#endif // TOPO_EXAMPLE_MINIMALPLUGIN_H
