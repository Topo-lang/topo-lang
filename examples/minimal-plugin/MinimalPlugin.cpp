// Minimal plugin example — demonstrates LanguagePlugin registration and lookup.
//
// This is a standalone example, not part of the main build.
// Build: see CMakeLists.txt in this directory.

#include "MinimalPlugin.h"
#include "topo/Lang/LanguagePlugin.h"

#include <cassert>
#include <iostream>

int main() {
    // 1. Create and register the plugin
    auto plugin = std::make_unique<example::MinimalPlugin>();
    topo::HostLanguage lang = plugin->language();

    topo::lang::registerPlugin(std::move(plugin));

    // 2. Look up by language
    auto* found = topo::lang::getPlugin(lang);
    assert(found && "plugin should be registered");

    std::cout << "Plugin registered for language: "
              << static_cast<int>(found->language()) << "\n";

    // 3. Verify capabilities
    auto provider = found->createAnalysisProvider();
    assert(provider && "analysis provider should be available");
    std::cout << "  AnalysisProvider: ok\n";

    auto* ef = found->emitterFactory();
    assert(ef && "emitter factory should be available");
    std::cout << "  EmitterFactory: ok (ext=" << ef->fileExtension() << ")\n";

    auto* bdf = found->buildDriverFactory();
    assert(bdf && "build driver factory should be available");
    std::cout << "  BuildDriverFactory: ok (tool=" << bdf->backendToolName() << ")\n";

    auto* itp = found->initTemplateProvider();
    assert(itp && "init template provider should be available");
    std::cout << "  InitTemplateProvider: ok (lang=" << itp->languageName() << ")\n";

    // 4. Iterate all plugins
    int count = 0;
    topo::lang::forEachPlugin([](topo::HostLanguage, topo::lang::LanguagePlugin&, void* ctx) {
        ++(*static_cast<int*>(ctx));
        return true;
    }, &count);
    assert(count == 1 && "exactly one plugin should be registered");
    std::cout << "  forEachPlugin count: " << count << "\n";

    // 5. Verify unregistered lookup returns nullptr
    auto* missing = topo::lang::getPlugin(topo::HostLanguage::Mixed);
    assert(!missing && "Mixed should not be registered");
    std::cout << "  Unregistered lookup: nullptr (correct)\n";

    std::cout << "\nAll checks passed.\n";
    return 0;
}
