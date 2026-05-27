cmake_minimum_required(VERSION 3.20)

project(topo-lang
    VERSION 0.1.0
    DESCRIPTION "Topo language-plugin framework — Registry, Emitter/Driver factories, CheckRunnerBase"
    HOMEPAGE_URL "https://github.com/topo-lang/topo-lang"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(TopoLangCompilerFlags)

# ── Upstream Topo packages ────────────────────────────
# topo-core exports topo::core::TopoCheck, TopoSema, TopoLSPBridge — the
# PUBLIC dependency closure of TopoLang.
# Embedded under the meta-repo: topo::core::TopoBasic is already
# defined as an ALIAS via add_subdirectory(topo-core); skip find_package
# in that case. Standalone falls through to find_package as today.
if(NOT TARGET topo::core::TopoBasic)
    find_package(topo-core CONFIG REQUIRED)
endif()

# ── External dependencies ─────────────────────────────
find_package(nlohmann_json CONFIG REQUIRED)

# ── topo-lang library ─────────────────────────────────
add_subdirectory(lib)

# ── Tests (off by default in standalone mode) ─────────
option(TOPO_LANG_BUILD_TESTS "Build topo-lang tests" OFF)
if(TOPO_LANG_BUILD_TESTS AND (PROJECT_IS_TOP_LEVEL OR TOPO_META_DRIVING_TESTS))
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)
    add_subdirectory(test)
endif()

# ── Install / export ──────────────────────────────────
install(EXPORT topo-langTargets
    FILE topo-langTargets.cmake
    NAMESPACE topo::lang::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/topo-lang
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/topo-langConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/topo-langConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/topo-lang
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/topo-langConfigVersion.cmake
    VERSION       ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/topo-langConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/topo-langConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/topo-lang
)
