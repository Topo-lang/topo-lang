// topo-lang — CheckRunnerBase implementation
//
// Common orchestration logic for language-specific check runners.
// Delegates to topo-core check algorithms.

#include "topo/Lang/CheckRunnerBase.h"

#include "topo/Check/CompletenessCheck.h"
#include "topo/Check/ContainmentCheck.h"
#include "topo/Check/PurityCheck.h"
#include "topo/Check/StageIsolationCheck.h"
#include "topo/Check/SymbolExtractor.h"
#include "topo/Check/VisibilityCheck.h"

namespace topo::lang {

check::CheckResult CheckRunnerBase::runCompleteness(
    const SymbolTable& symbols,
    const std::vector<VisibilityEntry>& visEntries,
    const std::vector<std::string>& sourceFiles,
    check::LanguageAnalysisProvider& provider,
    const check::CompletenessConfig& config) {

    check::CheckResult result;

    if (sourceFiles.empty()) {
        check::CheckDiagnostic d;
        d.severity = check::Severity::Warning;
        d.check = "completeness";
        d.message = "no source files found";
        result.addDiagnostic(std::move(d));
        return result;
    }

    auto extractor = provider.createSymbolExtractor();
    if (!extractor) {
        check::CheckDiagnostic d;
        d.severity = check::Severity::Error;
        d.check = "completeness";
        d.message = "symbol extraction unavailable for this language";
        result.addDiagnostic(std::move(d));
        return result;
    }

    auto hostSymbols = extractor->extractAll(sourceFiles);

    if (hostSymbols.empty() && !sourceFiles.empty()) {
        check::CheckDiagnostic d;
        d.severity = check::Severity::Error;
        d.check = "completeness";
        d.message = "extraction returned zero symbols from " +
                    std::to_string(sourceFiles.size()) +
                    " source file(s) — possible extraction failure";
        result.addDiagnostic(std::move(d));
        return result;
    }

    check::checkCompleteness(hostSymbols, symbols, visEntries, config, result);
    return result;
}

check::CheckResult CheckRunnerBase::runContainment(
    const SymbolTable& symbols,
    const std::vector<std::string>& sourceFiles,
    check::LanguageAnalysisProvider& provider,
    const ContainmentConfig& config,
    const std::string& projectDir,
    bool verbose) {

    check::CheckResult result;

    if (!config.isEnabled()) {
        check::CheckDiagnostic d;
        d.severity = check::Severity::Note;
        d.check = "containment";
        d.message = "containment check is disabled (mode=off)";
        result.addDiagnostic(std::move(d));
        return result;
    }

    if (sourceFiles.empty()) {
        return result;
    }

    // Extract imports
    auto ie = provider.createImportExtractor();
    if (!ie) {
        check::CheckDiagnostic d;
        d.severity = check::Severity::Error;
        d.check = "containment";
        d.message = "import extraction unavailable for this language";
        result.addDiagnostic(std::move(d));
        return result;
    }
    auto imports = ie->extractAll(sourceFiles);

    // Extract call sites
    auto cse = provider.createCallSiteExtractor();
    if (!cse) {
        check::CheckDiagnostic d;
        d.severity = check::Severity::Error;
        d.check = "containment";
        d.message = "call site extraction unavailable for this language";
        result.addDiagnostic(std::move(d));
        return result;
    }

    std::vector<check::DetectedCallSite> callSites;
    for (const auto& f : sourceFiles) {
        auto sites = cse->extractCallSites(f);
        callSites.insert(callSites.end(), sites.begin(), sites.end());
    }

    // Probe the symbol extractor as an "extractor health" signal. If symbols can
    // be parsed but only the unsafe-pattern-driven import and callsite extractors
    // are empty, the file is just clean code, not a failed extraction.
    // See checker-empty-extraction-error-misclassifies-clean-fixtures.md.
    std::vector<check::HostSymbol> hostSymbols;
    auto se = provider.createSymbolExtractor();
    if (se) {
        hostSymbols = se->extractAll(sourceFiles);
    }

    // Guard: only fire when ALL three extractors return nothing — that signals a
    // real extraction failure (not just a fixture without any unsafe patterns).
    if (imports.empty() && callSites.empty() && hostSymbols.empty()) {
        check::CheckDiagnostic d;
        d.severity = check::Severity::Error;
        d.check = "containment";
        d.message = "extraction returned zero imports, zero call sites, and zero host symbols from " +
                    std::to_string(sourceFiles.size()) +
                    " source file(s) — possible extraction failure";
        result.addDiagnostic(std::move(d));
        return result;
    }

    // Run L1 containment
    check::checkContainment(symbols, imports, callSites, config, result);

    // Attempt L2 deep containment if the provider supports it
    auto deepResult = provider.runDeepContainment(
        symbols, sourceFiles, config, projectDir, verbose);
    if (deepResult && deepResult->errorCount > 0) {
        // Merge L2 diagnostics into result
        for (auto& d : deepResult->diagnostics) {
            result.addDiagnostic(std::move(d));
        }
    }

    return result;
}

check::CheckResult CheckRunnerBase::runPurity(
    const SymbolTable& symbols,
    const std::vector<std::string>& sourceFiles,
    check::LanguageAnalysisProvider& provider,
    const PurityConfig& config) {

    check::CheckResult result;

    if (!config.isEnabled()) {
        check::CheckDiagnostic d;
        d.severity = check::Severity::Note;
        d.check = "purity";
        d.message = "purity check is disabled (mode=off)";
        result.addDiagnostic(std::move(d));
        return result;
    }

    if (sourceFiles.empty()) {
        return result;
    }

    auto sae = provider.createSymbolAccessExtractor();
    if (!sae) {
        check::CheckDiagnostic d;
        d.severity = check::Severity::Error;
        d.check = "purity";
        d.message = "symbol access extraction unavailable for this language";
        result.addDiagnostic(std::move(d));
        return result;
    }

    std::vector<check::SymbolAccess> accesses;
    for (const auto& f : sourceFiles) {
        auto fileAccesses = sae->extractSymbolAccesses(f);
        accesses.insert(accesses.end(), fileAccesses.begin(), fileAccesses.end());
    }

    check::checkPurity(symbols, accesses, result);
    return result;
}

check::CheckResult CheckRunnerBase::runVisibility(
    const SymbolTable& symbols,
    const std::vector<VisibilityEntry>& visEntries,
    const std::vector<std::string>& sourceFiles,
    check::LanguageAnalysisProvider& provider,
    const VisibilityCheckConfig& config) {

    check::CheckResult result;

    if (!config.isEnabled()) {
        check::CheckDiagnostic d;
        d.severity = check::Severity::Note;
        d.check = "visibility";
        d.message = "visibility check is disabled (mode=off)";
        result.addDiagnostic(std::move(d));
        return result;
    }

    if (sourceFiles.empty()) {
        return result;
    }

    auto cee = provider.createCallEdgeExtractor();
    if (!cee) {
        check::CheckDiagnostic d;
        d.severity = check::Severity::Error;
        d.check = "visibility";
        d.message = "call edge extraction unavailable for this language";
        result.addDiagnostic(std::move(d));
        return result;
    }

    std::vector<check::CallEdge> edges;
    for (const auto& f : sourceFiles) {
        auto fileEdges = cee->extractCallEdges(f);
        edges.insert(edges.end(), fileEdges.begin(), fileEdges.end());
    }

    check::checkVisibilityConsistency(symbols, visEntries, edges, result);
    return result;
}

check::CheckResult CheckRunnerBase::runStageIsolation(
    const SymbolTable& symbols,
    const std::vector<std::string>& sourceFiles,
    check::LanguageAnalysisProvider& provider,
    const StageIsolationConfig& config) {

    check::CheckResult result;

    if (!config.isEnabled()) {
        check::CheckDiagnostic d;
        d.severity = check::Severity::Note;
        d.check = "stage-isolation";
        d.message = "stage-isolation check is disabled (mode=off)";
        result.addDiagnostic(std::move(d));
        return result;
    }

    if (sourceFiles.empty()) {
        return result;
    }

    auto cee = provider.createCallEdgeExtractor();
    if (!cee) {
        check::CheckDiagnostic d;
        d.severity = check::Severity::Error;
        d.check = "stage-isolation";
        d.message = "call edge extraction unavailable for this language";
        result.addDiagnostic(std::move(d));
        return result;
    }

    std::vector<check::CallEdge> edges;
    for (const auto& f : sourceFiles) {
        auto fileEdges = cee->extractCallEdges(f);
        edges.insert(edges.end(), fileEdges.begin(), fileEdges.end());
    }

    check::checkStageIsolation(symbols, edges, result);
    return result;
}

} // namespace topo::lang
