# Minimal plugin example — standalone, not included in main build.
#
# To build from the project root:
#   cmake -B build-example -S topo-lang/examples/minimal-plugin \
#         -DCMAKE_PREFIX_PATH=<install-prefix>
#
# Or include in the main build with:
#   add_subdirectory(topo-lang/examples/minimal-plugin)

add_executable(minimal-plugin-example
    MinimalPlugin.cpp
)
target_include_directories(minimal-plugin-example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(minimal-plugin-example PRIVATE TopoLang)
topo_set_compiler_flags(minimal-plugin-example)
