// topo-lang — CheckRunnerBase orchestration tests
//
// Uses MinimalAnalysisProvider from the minimal-plugin fixture. That provider
// returns nullptr for all three extractor factories, which exercises the
// "extractor unavailable" branches in runCompleteness / runContainment.

#include "MinimalPlugin.h"
#include "topo/Build/PassConfig.h"
#include "topo/Check/CallSiteExtractor.h"
#include "topo/Check/CheckTypes.h"
#include "topo/Check/CompletenessCheck.h"
#include "topo/Check/ImportExtractor.h"
#include "topo/Check/LanguageAnalysisProvider.h"
#include "topo/Lang/CheckRunnerBase.h"
#include "topo/Sema/SymbolTable.h"
#include "topo/Sema/VisibilityCollector.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

using example::MinimalAnalysisProvider;
using topo::ContainmentConfig;
using topo::FeatureMode;
using topo::SymbolTable;
using topo::VisibilityEntry;
using topo::check::CallSiteExtractor;
using topo::check::CheckResult;
using topo::check::CompletenessConfig;
using topo::check::DetectedCallSite;
using topo::check::HostImport;
using topo::check::ImportExtractor;
using topo::check::LanguageAnalysisProvider;
using topo::check::Severity;
using topo::check::SymbolExtractor;
using topo::lang::CheckRunnerBase;

namespace {

CheckRunnerBase& runner() {
    static CheckRunnerBase instance;
    return instance;
}

// Extractors that return non-null instances yielding empty vectors.
// Models the runtime case where an LSP/regex extractor runs successfully
// but has nothing to report (project metadata missing, parser silently
// returning empty, etc.). The runContainment guard must catch this and
// emit Severity::Error rather than silently passing the check.
class EmptyImportExtractor : public ImportExtractor {
public:
    std::vector<HostImport> extractImports(const std::string& /*filePath*/) override {
        return {};
    }
};

class EmptyCallSiteExtractor : public CallSiteExtractor {
public:
    std::vector<DetectedCallSite> extractCallSites(const std::string& /*filePath*/) override {
        return {};
    }
};

class EmptyResultProvider : public LanguageAnalysisProvider {
public:
    std::unique_ptr<SymbolExtractor> createSymbolExtractor() override {
        return nullptr; // not needed for containment path
    }
    std::unique_ptr<ImportExtractor> createImportExtractor() override {
        return std::make_unique<EmptyImportExtractor>();
    }
    std::unique_ptr<CallSiteExtractor> createCallSiteExtractor() override {
        return std::make_unique<EmptyCallSiteExtractor>();
    }
    std::vector<std::string> collectSourceFiles(
        const std::string& /*projectDir*/,
        const std::vector<std::string>& /*includeDirs*/) const override {
        return {};
    }
};

} // namespace

TEST(CheckRunnerBaseTest, CompletenessWithEmptySourceFilesReturnsWarning) {
    SymbolTable symbols;
    std::vector<VisibilityEntry> visEntries;
    std::vector<std::string> sourceFiles;
    MinimalAnalysisProvider provider;
    CompletenessConfig config;

    CheckResult result =
        runner().runCompleteness(symbols, visEntries, sourceFiles, provider, config);

    EXPECT_EQ(result.errorCount, 0);
    EXPECT_GE(result.warningCount, 1);
    ASSERT_FALSE(result.diagnostics.empty());
    EXPECT_EQ(result.diagnostics.front().severity, Severity::Warning);
    EXPECT_EQ(result.diagnostics.front().check, "completeness");
}

TEST(CheckRunnerBaseTest, CompletenessWithNullExtractorReportsError) {
    SymbolTable symbols;
    std::vector<VisibilityEntry> visEntries;
    std::vector<std::string> sourceFiles{"fake_source.cpp"};
    MinimalAnalysisProvider provider; // Returns nullptr symbol extractor
    CompletenessConfig config;

    CheckResult result =
        runner().runCompleteness(symbols, visEntries, sourceFiles, provider, config);

    EXPECT_GE(result.errorCount, 1);
    EXPECT_FALSE(result.passed());
    ASSERT_FALSE(result.diagnostics.empty());
    EXPECT_EQ(result.diagnostics.front().check, "completeness");
    EXPECT_EQ(result.diagnostics.front().severity, Severity::Error);
}

TEST(CheckRunnerBaseTest, ContainmentWithModeOffReportsNote) {
    SymbolTable symbols;
    std::vector<std::string> sourceFiles;
    MinimalAnalysisProvider provider;
    ContainmentConfig config;
    config.mode = FeatureMode::Off;

    CheckResult result = runner().runContainment(
        symbols, sourceFiles, provider, config, /*projectDir=*/"", /*verbose=*/false);

    EXPECT_EQ(result.errorCount, 0);
    ASSERT_FALSE(result.diagnostics.empty());
    EXPECT_EQ(result.diagnostics.front().severity, Severity::Note);
    EXPECT_EQ(result.diagnostics.front().check, "containment");
}

TEST(CheckRunnerBaseTest, ContainmentWithEmptySourceFilesProducesNoDiagnostics) {
    SymbolTable symbols;
    std::vector<std::string> sourceFiles;
    MinimalAnalysisProvider provider;
    ContainmentConfig config;
    config.mode = FeatureMode::Force;

    CheckResult result = runner().runContainment(
        symbols, sourceFiles, provider, config, /*projectDir=*/"", /*verbose=*/false);

    EXPECT_EQ(result.errorCount, 0);
    EXPECT_TRUE(result.diagnostics.empty());
}

TEST(CheckRunnerBaseTest, ContainmentWithNullImportExtractorReportsError) {
    SymbolTable symbols;
    std::vector<std::string> sourceFiles{"fake.cpp"};
    MinimalAnalysisProvider provider; // Returns nullptr import extractor
    ContainmentConfig config;
    config.mode = FeatureMode::Force;

    CheckResult result = runner().runContainment(
        symbols, sourceFiles, provider, config, /*projectDir=*/"", /*verbose=*/false);

    EXPECT_GE(result.errorCount, 1);
    ASSERT_FALSE(result.diagnostics.empty());
    bool foundContainment = false;
    for (const auto& d : result.diagnostics) {
        if (d.check == "containment" && d.severity == Severity::Error) {
            foundContainment = true;
            break;
        }
    }
    EXPECT_TRUE(foundContainment);
}

TEST(CheckRunnerBaseTest, CheckRunnerBaseIsConstructible) {
    CheckRunnerBase local;
    SymbolTable symbols;
    std::vector<std::string> sourceFiles;
    MinimalAnalysisProvider provider;
    ContainmentConfig config;
    config.mode = FeatureMode::Off;

    CheckResult result =
        local.runContainment(symbols, sourceFiles, provider, config, "", false);
    EXPECT_TRUE(result.passed());
}

// Regression: when ALL extractors run successfully but return zero results,
// runContainment must report Severity::Error rather than passing silently.
// Mirrors the runCompleteness empty-extraction guard (issue
// checker-runcontainment-empty-extraction-silent-warn).
TEST(CheckRunnerBaseTest, ContainmentEmptyExtractionReportsError) {
    SymbolTable symbols;
    std::vector<std::string> sourceFiles{"fake1.cpp", "fake2.cpp"};
    EmptyResultProvider provider;
    ContainmentConfig config;
    config.mode = FeatureMode::Force;

    CheckResult result = runner().runContainment(
        symbols, sourceFiles, provider, config, /*projectDir=*/"", /*verbose=*/false);

    EXPECT_GE(result.errorCount, 1);
    EXPECT_FALSE(result.passed());
    bool foundContainmentError = false;
    for (const auto& d : result.diagnostics) {
        if (d.check == "containment" && d.severity == Severity::Error &&
            d.message.find("zero host symbols") != std::string::npos) {
            foundContainmentError = true;
            break;
        }
    }
    EXPECT_TRUE(foundContainmentError);
}

// Regression: when SymbolExtractor returns hostSymbols but Import/CallSite
// extractors return empty, the file is just clean code and the guard must
// NOT fire. Validates the fix for
// checker-empty-extraction-error-misclassifies-clean-fixtures.
class CleanCodeProvider : public LanguageAnalysisProvider {
public:
    std::unique_ptr<SymbolExtractor> createSymbolExtractor() override {
        class HasSymbols : public SymbolExtractor {
        public:
            std::vector<topo::check::HostSymbol> extractSymbols(
                const std::string& /*filePath*/) override {
                topo::check::HostSymbol sym;
                sym.qualifiedName = "app::compute";
                sym.simpleName = "compute";
                sym.kind = topo::check::HostSymbolKind::Function;
                sym.file = "fake.cpp";
                sym.line = 1;
                return {sym};
            }
        };
        return std::make_unique<HasSymbols>();
    }
    std::unique_ptr<ImportExtractor> createImportExtractor() override {
        return std::make_unique<EmptyImportExtractor>();
    }
    std::unique_ptr<CallSiteExtractor> createCallSiteExtractor() override {
        return std::make_unique<EmptyCallSiteExtractor>();
    }
    std::vector<std::string> collectSourceFiles(
        const std::string& /*projectDir*/,
        const std::vector<std::string>& /*includeDirs*/) const override {
        return {};
    }
};

TEST(CheckRunnerBaseTest, ContainmentCleanCodeNotMisclassifiedAsExtractionFailure) {
    SymbolTable symbols;
    std::vector<std::string> sourceFiles{"fake.cpp"};
    CleanCodeProvider provider;
    ContainmentConfig config;
    config.mode = FeatureMode::Force;

    CheckResult result = runner().runContainment(
        symbols, sourceFiles, provider, config, /*projectDir=*/"", /*verbose=*/false);

    // No "extraction failure" error must be reported — symbols were parsed,
    // the file is just clean. errorCount must be 0.
    EXPECT_EQ(result.errorCount, 0);
    EXPECT_TRUE(result.passed());
    for (const auto& d : result.diagnostics) {
        EXPECT_EQ(d.message.find("possible extraction failure"), std::string::npos)
            << "Clean code must not trigger extraction-failure error; got: " << d.message;
    }
}
