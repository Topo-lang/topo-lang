#ifndef TOPO_LANG_EMITTERFACTORY_H
#define TOPO_LANG_EMITTERFACTORY_H

#include <memory>
#include <string>

namespace topo::transpile { class Emitter; }

namespace topo::lang {

/// Creates language-specific Emitter instances for transpilation.
class EmitterFactory {
public:
    virtual ~EmitterFactory() = default;

    /// Create an Emitter for this language.
    virtual std::unique_ptr<transpile::Emitter> createEmitter() = 0;

    /// File extension for emitted output (e.g. ".cpp", ".rs", ".java", ".py").
    virtual std::string fileExtension() const = 0;
};

} // namespace topo::lang

#endif // TOPO_LANG_EMITTERFACTORY_H
