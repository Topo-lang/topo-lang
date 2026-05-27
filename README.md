# topo-lang — Language Plugin Framework

Lightweight framework defining how language support packages (`topo-lang-xxx`) plug into topo-core.

## Provides

| Interface | Purpose |
|-----------|---------|
| `LanguagePlugin` | Aggregates all language-specific capabilities |
| `EmitterFactory` | Creates transpile Emitter instances |
| `BuildDriverFactory` | Declares backend/extractor tool names |
| `InitTemplateProvider` | Project scaffolding templates |
| `CheckRunnerBase` | Common completeness/containment check orchestration |
| Registry | `registerPlugin()` / `getPlugin()` / `forEachPlugin()` |

## Adding a New Language

See `docs/plugin-guide.md` for step-by-step instructions.

See `docs/directory-convention.md` for the standard `topo-lang-xxx/` layout.

See `examples/minimal-plugin/` for a working registration example.

## Build

Part of the Topo project. Builds as `TopoLang` static library.
