#ifndef TOPO_LANG_LANGUAGEPLUGIN_H
#define TOPO_LANG_LANGUAGEPLUGIN_H

#include "topo/Basic/HostLanguage.h"
#include <memory>

namespace topo::check { class LanguageAnalysisProvider; }
namespace topo::lang { class CheckRunnerBase; }
namespace topo::lsp { class LSPBridge; }
namespace topo::transpile { class Emitter; struct TranspileModule; }

namespace topo::lang {

class EmitterFactory;
class BuildDriverFactory;
class InitTemplateProvider;

/// Aggregates all language-specific capabilities for a single host language.
class LanguagePlugin {
public:
    virtual ~LanguagePlugin() = default;

    /// The host language this plugin supports.
    virtual HostLanguage language() const = 0;

    /// Create a LanguageAnalysisProvider for check operations.
    virtual std::unique_ptr<check::LanguageAnalysisProvider> createAnalysisProvider() = 0;

    /// Get the emitter factory (transpile support). May return nullptr if not supported.
    virtual EmitterFactory* emitterFactory() { return nullptr; }

    /// Get the build driver factory. May return nullptr if not supported.
    virtual BuildDriverFactory* buildDriverFactory() { return nullptr; }

    /// Get the init template provider. May return nullptr if not supported.
    virtual InitTemplateProvider* initTemplateProvider() { return nullptr; }

    /// Create an LSP bridge for this language. May return nullptr if not supported.
    /// Definition out-of-line in LanguagePlugin.cpp: inline `{ return nullptr; }`
    /// instantiates `std::unique_ptr<lsp::LSPBridge>`'s destructor in every TU
    /// that sees this header, and libstdc++'s default_delete static_asserts on
    /// complete type — LSPBridge is only forward-declared here to keep
    /// topo-lang free of a link-time topo-lsp dep.
    virtual std::unique_ptr<lsp::LSPBridge> createLSPBridge();

    /// Create a language-specific check runner. Returns nullptr if the
    /// default CheckRunnerBase behavior is sufficient.
    /// Out-of-line for the same libstdc++ complete-type reason as above.
    virtual std::unique_ptr<CheckRunnerBase> createCheckRunner();
};

// -- Registry --

/// Register a language plugin. Takes ownership. Call at static init or early main.
void registerPlugin(std::unique_ptr<LanguagePlugin> plugin);

/// Look up a registered plugin by language. Returns nullptr if not registered.
LanguagePlugin* getPlugin(HostLanguage lang);

/// Iterate all registered plugins.
/// Callback receives (HostLanguage, LanguagePlugin&) and returns true to continue.
void forEachPlugin(bool (*callback)(HostLanguage, LanguagePlugin&, void*), void* ctx);

} // namespace topo::lang

#endif // TOPO_LANG_LANGUAGEPLUGIN_H
