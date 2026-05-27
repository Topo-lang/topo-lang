# topo-lang plugin framework unit tests
#
# Links minimal-plugin fixture sources directly (not as a library) so the
# test executable owns its own MinimalPlugin translation unit. The example
# itself remains a standalone executable and is not built by default.

add_executable(topo-lang-unit-tests
    unit/RegistryTest.cpp
    unit/EmitterFactoryTest.cpp
    unit/BuildDriverFactoryTest.cpp
    unit/InitTemplateProviderTest.cpp
    unit/CheckRunnerBaseTest.cpp
)
target_link_libraries(topo-lang-unit-tests PRIVATE
    TopoLang
    GTest::gtest_main
)
target_include_directories(topo-lang-unit-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../examples/minimal-plugin
)
topo_set_compiler_flags(topo-lang-unit-tests)
gtest_discover_tests(topo-lang-unit-tests DISCOVERY_TIMEOUT 30
    PROPERTIES LABELS "toolchain;lang" TIMEOUT 60)
