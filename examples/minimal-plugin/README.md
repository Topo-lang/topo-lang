# Minimal Language Plugin Example

Demonstrates the `topo::lang::LanguagePlugin` registration pattern.

## What it shows

1. Implementing `LanguagePlugin` with all optional capabilities
2. Providing stub `LanguageAnalysisProvider`, `EmitterFactory`, `BuildDriverFactory`, `InitTemplateProvider`
3. Registering a plugin via `registerPlugin()`
4. Looking up a plugin via `getPlugin()`
5. Iterating all registered plugins via `forEachPlugin()`

## Building

This example is not part of the main build. To include it:

```cmake
add_subdirectory(topo-lang/examples/minimal-plugin)
```

## Files

- `MinimalPlugin.h` — Plugin class with stub implementations of all interfaces
- `MinimalPlugin.cpp` — Registration, lookup, and verification
- `CMakeLists.txt` — Build configuration (links against `TopoLang`)
