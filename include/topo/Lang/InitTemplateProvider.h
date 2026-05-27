#ifndef TOPO_LANG_INITTEMPLATEPROVIDER_H
#define TOPO_LANG_INITTEMPLATEPROVIDER_H

#include <string>
#include <vector>

namespace topo::lang {

/// Provides project initialization templates for a language.
class InitTemplateProvider {
public:
    virtual ~InitTemplateProvider() = default;

    /// Language name as used in Topo.toml [build].language (e.g. "cpp", "rust").
    virtual std::string languageName() const = 0;

    /// File glob patterns for detecting this language in a project.
    /// e.g. {"*.cpp", "*.cc", "*.h", "*.hpp"} for C++.
    virtual std::vector<std::string> filePatterns() const = 0;

    /// Generate initial Topo.toml content for this language.
    virtual std::string generateTopoToml(const std::string& projectName) const = 0;

    /// Generate initial .topo file content with type bindings for this language.
    virtual std::string generateTypeBindings() const = 0;

    /// Default source glob pattern for Topo.toml [build].sources.
    /// e.g. "src/**/*.cpp" for C++, "src/**/*.rs" for Rust.
    virtual std::string sourceFileGlob() const = 0;

    /// Generate a project `.lldbinit` that wires the zero-install native-lldb
    /// formatter as a courtesy convenience. `formatterRelPath` is the path
    /// to the language's `lldb_formatter.py` relative to the project root,
    /// as laid down by `topo init`. Returns empty for languages with no
    /// native-lldb formatter (Java/Python/TypeScript/minimal) — `topo init`
    /// then writes no `.lldbinit`. Default is empty so only C++/Rust opt in.
    virtual std::string generateLldbInit(const std::string& /*formatterRelPath*/) const {
        return {};
    }

    /// The native-lldb formatter script's contents. `topo init` writes this
    /// alongside the `.lldbinit` so the scaffold is self-contained (no
    /// dependency on an installed Topo toolchain path). Empty when the
    /// language has no formatter; paired with generateLldbInit().
    virtual std::string lldbFormatterScript() const { return {}; }

    /// Relative path (from project root) where lldbFormatterScript() should be
    /// written and which generateLldbInit() imports. Empty when no formatter.
    virtual std::string lldbFormatterRelPath() const { return {}; }
};

} // namespace topo::lang

#endif // TOPO_LANG_INITTEMPLATEPROVIDER_H
