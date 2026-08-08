# Public Plugin Catalog

This catalog describes the code present in the current Development Preview. It
does not promise API stability or production readiness.

| Plugin | Current responsibility | Notable extension points |
|---|---|---|
| `PGXCore` | Shared types, subsystems, configuration, registry, messages, event handlers, logging, trace, observability, validation and editor primitives | `IPGX*` interfaces, handler and observable bases, registries, Blueprint libraries and K2 message support |
| `PGXAudio` | Data-driven playback, channels, music/dialogue, pooling, mixing and legacy/Audio Modulation backends | Abstract audio backend, config/profile/channel assets, audio component, delegates and Blueprint library |
| `PGXGameFlow` | Multi-channel Gameplay Tag state machine, rules, history and session state | Rules/config assets, Blueprint library, delegates and typed bridge payloads |
| `PGXLoading` | Loading presentation, visual strategies, level flow, streaming and progress | Strategy base, loading widget, level-flow actor, profiles/configs and Blueprint libraries |
| `PGXMGOS` | Garbage-collection observation, snapshots, baselines, class health and incidents | Observer configuration, Blueprint queries/control and native delegates |
| `PGXPSO` | Data-driven PSO warm-up, batching, deduplication, recording/export and contextual activation | Warm-up config, entries/context tags, Blueprint library, delegates and recording API |
| `PGXSave` | Slots, domains, async save/load, serialization, backup, versioning, migrations and storage providers | Saveable interface, save game/provider bases, registration APIs, async actions and save-domain K2 node |
| `PGXEditorTools` | Integrated editor hub and inspectors for the selected runtime systems and registries | Slate panels, tab spawners and category tooling; no runtime API |
| `PGXDocs` | Documentation roots, registry, Markdown rendering, tree/search/link validation, watching and editor viewer | Additional documentation roots and a documentation-provider interface; automatic provider discovery is not yet a documented guarantee |
| `PGXScaffold` | Project analysis, templates, build plans, validation, transactional execution and rollback | Public template registry and template/plan types |
| `PGXTutorials` | Editor tutorial hub, overlays and guided actions | Editor workflow only; no stable public extension headers are claimed in this preview |
| `PGXVersionControl` | Unreal Source Control overlay for changelists, tags/templates, validation and inspection | Source-control subsystem, settings, public result types and change delegate |

## API stability

Check the selected tag's plugin descriptors, module rules, and headers before
depending on a symbol. During the `0.x` series, extension points may move or
change with migration notes.
