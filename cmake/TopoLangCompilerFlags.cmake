# TopoLangCompilerFlags.cmake — standalone compiler-flag helper for topo-lang.
#
# Same shape as the topo-core variant: cxx_std_17, -Wall -Wextra -Wpedantic
# (or /W4 on MSVC), optional sanitizer.

if(NOT WIN32)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    if(APPLE)
        set(CMAKE_MACOSX_RPATH ON)
    endif()
endif()

set(TOPO_LANG_SANITIZER "" CACHE STRING
    "Enable sanitizers (address, undefined, thread, memory)")

if(TOPO_LANG_SANITIZER)
    message(STATUS "topo-lang sanitizers enabled: ${TOPO_LANG_SANITIZER}")
endif()

function(topo_lang_apply_sanitizer target)
    if(NOT TOPO_LANG_SANITIZER)
        return()
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target}
            PRIVATE -fsanitize=${TOPO_LANG_SANITIZER} -fno-omit-frame-pointer)
        target_link_options(${target}
            PRIVATE -fsanitize=${TOPO_LANG_SANITIZER})
    endif()
endfunction()

function(topo_set_compiler_flags target)
    target_compile_features(${target} PUBLIC cxx_std_17)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    elseif(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    endif()

    topo_lang_apply_sanitizer(${target})
endfunction()

# PCH stub: no-op in standalone (lib/CMakeLists doesn't currently call it
# but keep the symbol available for future converts from the monorepo
# topo_apply_std_pch pattern).
function(topo_apply_std_pch target)
endfunction()
