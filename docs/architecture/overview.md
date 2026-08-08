# Architecture Overview

PGX is a set of Unreal Engine plugins. It extends Unreal's plugin,
module, subsystem, Data Asset, Gameplay Tag, Blueprint and editor-extension
models instead of introducing a separate application runtime.

## Included code

This release contains 12 plugins and 23 modules. Its descriptors and `Build.cs`
files define dependency and loading boundaries.

| Layer | Responsibility |
|---|---|
| Foundation | `PGXCore` supplies shared contracts, registries, messages, event handlers, configuration, observability and editor primitives. |
| Runtime systems | Audio, game flow, loading, memory/GC observation, PSO warm-up and save/persistence. |
| Editor experience | Integrated inspectors, documentation, scaffolding, tutorials and source-control workflows. |

Runtime and editor responsibilities are separated whenever the current Unreal
module boundary permits it. Two specialized modules are `UncookedOnly`, and one
Core diagnostics module is `DeveloperTool`; these classifications matter when
selecting a target.

## Dependency shape

Feature plugins depend inward on `PGXCore`. Runtime systems do not compile
directly against one another. Where systems coordinate, they use Core message
contracts, Gameplay Tags, shared interfaces or registry discovery.

`PGXEditorTools` is intentionally different: it is an editor-only aggregation
layer that compiles against the selected systems so it can present their
inspectors in one place.

See [Modules and dependencies](modules-and-dependencies.md) for the exact graph.

## Common system pattern

Most runtime systems follow a recognizable Unreal-native flow:

1. project settings and Data Assets define policy;
2. a subsystem resolves the active configuration;
3. Blueprint libraries and native interfaces expose operations;
4. Gameplay Tags identify channels, domains or states;
5. delegates, messages, traces and editor inspectors expose observable state.

Each plugin owns its domain types; sibling runtime plugins communicate through
Core instead of linking to one another.

## Release architecture

The public repository is a curated, dependency-checked snapshot of the selected
plugins and public documentation. A candidate must pass path and provenance
checks, descriptor closure, exact file inventory checks and a clean exported-tree
build before it can be proposed as a tagged preview.

Components absent from the release tree are outside its public API and support
scope.

## Continue reading

- [System map](system-map.md)
- [Modules and dependencies](modules-and-dependencies.md)
- [Runtime flows](runtime-flows.md)
- [Plugin catalog](../plugins/catalog.md)
- [Verification](../validation/verification.md)
