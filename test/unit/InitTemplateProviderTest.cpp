// topo-lang — InitTemplateProvider contract tests

#include "MinimalPlugin.h"
#include "topo/Lang/InitTemplateProvider.h"

#include <gtest/gtest.h>
#include <algorithm>

using example::MinimalInitTemplateProvider;
using topo::lang::InitTemplateProvider;

TEST(InitTemplateProviderTest, LanguageNameMatchesMinimal) {
    MinimalInitTemplateProvider provider;
    EXPECT_EQ(provider.languageName(), "minimal");
}

TEST(InitTemplateProviderTest, FilePatternsNonEmpty) {
    MinimalInitTemplateProvider provider;
    auto patterns = provider.filePatterns();
    EXPECT_FALSE(patterns.empty());
}

TEST(InitTemplateProviderTest, FilePatternsIncludesMinExtension) {
    MinimalInitTemplateProvider provider;
    auto patterns = provider.filePatterns();
    auto it = std::find(patterns.begin(), patterns.end(), "*.min");
    EXPECT_NE(it, patterns.end());
}

TEST(InitTemplateProviderTest, GenerateTopoTomlEmbedsProjectName) {
    MinimalInitTemplateProvider provider;
    std::string toml = provider.generateTopoToml("demo");
    EXPECT_NE(toml.find("demo"), std::string::npos);
}

TEST(InitTemplateProviderTest, GenerateTopoTomlContainsTopoAndBuildSections) {
    MinimalInitTemplateProvider provider;
    std::string toml = provider.generateTopoToml("demo");
    EXPECT_NE(toml.find("[topo]"), std::string::npos);
    EXPECT_NE(toml.find("[build]"), std::string::npos);
}

TEST(InitTemplateProviderTest, GenerateTopoTomlDeclaresLanguage) {
    MinimalInitTemplateProvider provider;
    std::string toml = provider.generateTopoToml("demo");
    // Every language provider must emit a [build].language field so
    // topo-build can route to the right backend.
    EXPECT_NE(toml.find("language"), std::string::npos);
}

TEST(InitTemplateProviderTest, GenerateTypeBindingsNonEmpty) {
    MinimalInitTemplateProvider provider;
    std::string bindings = provider.generateTypeBindings();
    EXPECT_FALSE(bindings.empty());
}

TEST(InitTemplateProviderTest, SourceFileGlobNonEmpty) {
    MinimalInitTemplateProvider provider;
    std::string glob = provider.sourceFileGlob();
    EXPECT_FALSE(glob.empty());
}

TEST(InitTemplateProviderTest, UsableViaBasePointer) {
    MinimalInitTemplateProvider concrete;
    InitTemplateProvider* base = &concrete;
    EXPECT_EQ(base->languageName(), "minimal");
    EXPECT_FALSE(base->filePatterns().empty());
    EXPECT_FALSE(base->generateTypeBindings().empty());
    EXPECT_FALSE(base->sourceFileGlob().empty());
}
