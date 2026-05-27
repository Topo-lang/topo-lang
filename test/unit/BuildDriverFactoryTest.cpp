// topo-lang — BuildDriverFactory contract tests

#include "MinimalPlugin.h"
#include "topo/Lang/BuildDriverFactory.h"

#include <gtest/gtest.h>

using example::MinimalBuildDriverFactory;
using topo::lang::BuildDriverFactory;

TEST(BuildDriverFactoryTest, BackendToolNameMatchesMinimal) {
    MinimalBuildDriverFactory factory;
    EXPECT_EQ(factory.backendToolName(), "topo-build-minimal");
}

TEST(BuildDriverFactoryTest, ExtractorToolNameMatchesMinimal) {
    MinimalBuildDriverFactory factory;
    EXPECT_EQ(factory.extractorToolName(), "topo-extract-minimal");
}

TEST(BuildDriverFactoryTest, BackendToolNameNonEmpty) {
    MinimalBuildDriverFactory factory;
    EXPECT_FALSE(factory.backendToolName().empty());
}

TEST(BuildDriverFactoryTest, ExtractorToolNameNonEmpty) {
    MinimalBuildDriverFactory factory;
    EXPECT_FALSE(factory.extractorToolName().empty());
}

TEST(BuildDriverFactoryTest, UsableViaBasePointer) {
    MinimalBuildDriverFactory concrete;
    BuildDriverFactory* base = &concrete;
    EXPECT_EQ(base->backendToolName(), "topo-build-minimal");
    EXPECT_EQ(base->extractorToolName(), "topo-extract-minimal");
}

TEST(BuildDriverFactoryTest, ToolNamesAreStable) {
    // Contract: repeated calls must return the same identifier.
    MinimalBuildDriverFactory factory;
    std::string a = factory.backendToolName();
    std::string b = factory.backendToolName();
    EXPECT_EQ(a, b);
}
