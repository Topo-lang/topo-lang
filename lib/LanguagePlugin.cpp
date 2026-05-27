// topo-lang — LanguagePlugin registry implementation

#include "topo/Lang/LanguagePlugin.h"

// Full definitions of the forward-declared return types — needed so the
// out-of-line createLSPBridge / createCheckRunner default bodies below
// can let std::unique_ptr's destructor see a complete type. LSPBridge
// lives in topo-core; topo-lang PUBLIC-depends on TopoLSPBridge so the
// header is reachable without adding a link-time dep on topo-lsp.
#include "topo/LSP/LSPBridge.h"
#include "topo/Lang/CheckRunnerBase.h"

#include <unordered_map>

namespace topo::lang {

// Default no-op implementations — plugins that don't support these
// capabilities inherit the nullptr return.
std::unique_ptr<lsp::LSPBridge> LanguagePlugin::createLSPBridge() {
    return nullptr;
}

std::unique_ptr<CheckRunnerBase> LanguagePlugin::createCheckRunner() {
    return nullptr;
}


namespace {

struct HostLanguageHash {
    std::size_t operator()(HostLanguage lang) const noexcept {
        return std::hash<int>{}(static_cast<int>(lang));
    }
};

using PluginMap = std::unordered_map<HostLanguage, std::unique_ptr<LanguagePlugin>, HostLanguageHash>;

/// Global plugin registry. Thread safety is not required — registration
/// happens at startup before any concurrent access.
PluginMap& registry() {
    static PluginMap instance;
    return instance;
}

} // namespace

void registerPlugin(std::unique_ptr<LanguagePlugin> plugin) {
    if (!plugin) return;
    HostLanguage lang = plugin->language();
    registry()[lang] = std::move(plugin);
}

LanguagePlugin* getPlugin(HostLanguage lang) {
    auto& reg = registry();
    auto it = reg.find(lang);
    return (it != reg.end()) ? it->second.get() : nullptr;
}

void forEachPlugin(bool (*callback)(HostLanguage, LanguagePlugin&, void*), void* ctx) {
    if (!callback) return;
    for (auto& [lang, plugin] : registry()) {
        if (!callback(lang, *plugin, ctx)) break;
    }
}

} // namespace topo::lang
