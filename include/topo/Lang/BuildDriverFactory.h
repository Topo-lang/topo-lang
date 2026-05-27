#ifndef TOPO_LANG_BUILDDRIVERFACTORY_H
#define TOPO_LANG_BUILDDRIVERFACTORY_H

#include <string>

namespace topo::lang {

/// Provides build driver information for a language.
/// The actual build driver runs as a separate process; this factory
/// tells topo-build which tool to invoke.
class BuildDriverFactory {
public:
    virtual ~BuildDriverFactory() = default;

    /// The backend tool executable name (e.g. "topo-build-llvm-cpp").
    virtual std::string backendToolName() const = 0;

    /// The extractor tool name for transpile (e.g. "topo-extract-cpp").
    virtual std::string extractorToolName() const = 0;
};

} // namespace topo::lang

#endif // TOPO_LANG_BUILDDRIVERFACTORY_H
