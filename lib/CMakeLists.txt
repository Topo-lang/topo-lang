add_library(TopoLang
    LanguagePlugin.cpp
    CheckRunnerBase.cpp
)
add_library(topo::lang::TopoLang ALIAS TopoLang)

target_include_directories(TopoLang PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# PUBLIC link surface:
#   - topo::core::TopoSema for SymbolTable + frontend types referenced by
#     LanguagePlugin.h
#   - topo::core::TopoCheck for CheckRunnerBase's check-orchestration types
#   - topo::core::TopoLSPBridge so consumers of LanguagePlugin's
#     createLSPBridge() return type see the complete LSPBridge definition
#     (previously a private include of topo-lsp/lib in the monorepo, since
#     LSPBridge.h moved to topo-core in the phase-1 D2 step).
target_link_libraries(TopoLang
    PUBLIC
        topo::core::TopoCheck
        topo::core::TopoSema
        topo::core::TopoLSPBridge
    PRIVATE
        nlohmann_json::nlohmann_json
)

topo_set_compiler_flags(TopoLang)

# Install: lib + headers + EXPORT membership.
install(TARGETS TopoLang
    EXPORT topo-langTargets
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include/topo/Lang
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/topo
    FILES_MATCHING PATTERN "*.h"
)
