#ifndef TOPO_LANG_CHECKRUNNERBASE_H
#define TOPO_LANG_CHECKRUNNERBASE_H

#include "topo/Build/PassConfig.h"
#include "topo/Check/CheckTypes.h"
#include "topo/Check/CompletenessCheck.h"
#include "topo/Check/ContainmentCheck.h"
#include "topo/Check/LanguageAnalysisProvider.h"
#include "topo/Sema/SymbolTable.h"
#include "topo/Sema/VisibilityCollector.h"

#include <memory>
#include <string>
#include <vector>

namespace topo::lang {

/// Base class for language-specific check runners.
/// Provides common orchestration logic; subclasses supply the provider.
class CheckRunnerBase {
public:
    virtual ~CheckRunnerBase() = default;

    /// Run completeness check using the given symbols and provider.
    check::CheckResult runCompleteness(
        const SymbolTable& symbols,
        const std::vector<VisibilityEntry>& visEntries,
        const std::vector<std::string>& sourceFiles,
        check::LanguageAnalysisProvider& provider,
        const check::CompletenessConfig& config);

    /// Run containment check.
    check::CheckResult runContainment(
        const SymbolTable& symbols,
        const std::vector<std::string>& sourceFiles,
        check::LanguageAnalysisProvider& provider,
        const ContainmentConfig& config,
        const std::string& projectDir,
        bool verbose);

    /// Run purity check (parallel-stage global-state safety).
    check::CheckResult runPurity(
        const SymbolTable& symbols,
        const std::vector<std::string>& sourceFiles,
        check::LanguageAnalysisProvider& provider,
        const PurityConfig& config);

    /// Run visibility-consistency check (private/internal callers are in scope).
    check::CheckResult runVisibility(
        const SymbolTable& symbols,
        const std::vector<VisibilityEntry>& visEntries,
        const std::vector<std::string>& sourceFiles,
        check::LanguageAnalysisProvider& provider,
        const VisibilityCheckConfig& config);

    /// Run stage-isolation check (stage N does not depend on stage N+1).
    check::CheckResult runStageIsolation(
        const SymbolTable& symbols,
        const std::vector<std::string>& sourceFiles,
        check::LanguageAnalysisProvider& provider,
        const StageIsolationConfig& config);
};

} // namespace topo::lang

#endif // TOPO_LANG_CHECKRUNNERBASE_H
