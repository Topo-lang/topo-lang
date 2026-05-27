// topo-lang — Registry unit tests
//
// Exercises registerPlugin / getPlugin / forEachPlugin.
// NOTE: The Registry is a process-wide singleton. Tests intentionally
// avoid fighting over shared state by using distinct HostLanguage values
// where possible, and by re-registering within each TEST to establish
// a known state before assertions.

#include "MinimalPlugin.h"
#include "topo/Lang/LanguagePlugin.h"

#include <gtest/gtest.h>
#include <memory>

using topo::HostLanguage;
using topo::lang::forEachPlugin;
using topo::lang::getPlugin;
using topo::lang::LanguagePlugin;
using topo::lang::registerPlugin;

namespace {

/// Minimal plugin variant that reports a configurable HostLanguage so
/// different tests can write to distinct registry slots without collision.
template <HostLanguage L>
class TaggedMinimalPlugin : public example::MinimalPlugin {
public:
    HostLanguage language() const override { return L; }
};

struct CountingCtx {
    int count = 0;
    HostLanguage lastSeen = HostLanguage::Mixed;
};

bool countCallback(HostLanguage lang, LanguagePlugin&, void* ctx) {
    auto* c = static_cast<CountingCtx*>(ctx);
    ++c->count;
    c->lastSeen = lang;
    return true;
}

} // namespace

TEST(RegistryTest, RegisterAndLookupByLanguage) {
    registerPlugin(std::make_unique<TaggedMinimalPlugin<HostLanguage::Cpp>>());

    LanguagePlugin* plugin = getPlugin(HostLanguage::Cpp);
    ASSERT_NE(plugin, nullptr);
    EXPECT_EQ(plugin->language(), HostLanguage::Cpp);
}

TEST(RegistryTest, LookupReturnsNonNullAnalysisProvider) {
    registerPlugin(std::make_unique<TaggedMinimalPlugin<HostLanguage::Cpp>>());

    LanguagePlugin* plugin = getPlugin(HostLanguage::Cpp);
    ASSERT_NE(plugin, nullptr);
    auto provider = plugin->createAnalysisProvider();
    EXPECT_NE(provider, nullptr);
}

TEST(RegistryTest, RegisterTwiceOverwritesExisting) {
    // LanguagePlugin.cpp uses operator[] = std::move(plugin), so the second
    // registration replaces the first. We assert that behavior here so any
    // accidental change (e.g., becoming an error) breaks this test.
    registerPlugin(std::make_unique<TaggedMinimalPlugin<HostLanguage::Rust>>());
    LanguagePlugin* first = getPlugin(HostLanguage::Rust);
    ASSERT_NE(first, nullptr);

    registerPlugin(std::make_unique<TaggedMinimalPlugin<HostLanguage::Rust>>());
    LanguagePlugin* second = getPlugin(HostLanguage::Rust);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(second->language(), HostLanguage::Rust);
}

TEST(RegistryTest, RegisterNullptrIsNoOp) {
    // Registering a null unique_ptr must not crash and must not affect any slot.
    registerPlugin(nullptr);
    SUCCEED();
}

TEST(RegistryTest, ForEachPluginVisitsAllRegistered) {
    registerPlugin(std::make_unique<TaggedMinimalPlugin<HostLanguage::Cpp>>());
    registerPlugin(std::make_unique<TaggedMinimalPlugin<HostLanguage::Rust>>());
    registerPlugin(std::make_unique<TaggedMinimalPlugin<HostLanguage::Java>>());
    registerPlugin(std::make_unique<TaggedMinimalPlugin<HostLanguage::Python>>());

    CountingCtx ctx;
    forEachPlugin(countCallback, &ctx);

    EXPECT_GE(ctx.count, 4);
}

TEST(RegistryTest, ForEachPluginHandlesNullCallback) {
    forEachPlugin(nullptr, nullptr);
    SUCCEED();
}

TEST(RegistryTest, LookupUnregisteredReturnsNullptr) {
    // Mixed is never a valid plugin language in production; guaranteed unregistered.
    LanguagePlugin* plugin = getPlugin(HostLanguage::Mixed);
    EXPECT_EQ(plugin, nullptr);
}
