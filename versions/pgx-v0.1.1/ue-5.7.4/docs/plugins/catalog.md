# Public Plugin Catalog

This catalog describes the code present in the current Development Preview. It
does not promise API stability, production readiness or completion of every
planned feature.

| Plugin | Implemented responsibility | Current boundary |
|---|---|---|
| `PGXCore` | Shared types, subsystems, configuration, registry, messages, event handlers, logging, trace, observability, validation and editor primitives | Foundation for the selected plugin graph |
| `PGXAbility` | Experimental Gameplay Ability System facades for abilities, attributes and effects | Projects own ActorInfo and replication setup |
| `PGXAI` | Agent registry and Behavior Tree dispatch/status | Perception, planning, state-machine and squad orchestration are not complete |
| `PGXAudio` | Data-driven playback, channels, music/dialogue, pooling, mixing and legacy/Audio Modulation backends | Cross-platform and packaged-runtime coverage remains limited |
| `PGXCamera` | Camera mode and configuration state | No blending, modifiers or collision resolution |
| `PGXColony` | Survivor registry, handles, configuration and tags | No settlement simulation, policy, needs or jobs |
| `PGXCrafting` | Recipe registry, validation, simulation and job lifecycle | Does not mutate inventory or produce gameplay outputs |
| `PGXEnvironment` | Zones, authored seeds, thresholds, severity and modifiers | Limited editor and Blueprint facade coverage |
| `PGXGameFlow` | Multi-channel Gameplay Tag state machine, rules, history and session state | APIs may change during the preview |
| `PGXInput` | Enhanced Input context application, priority/exclusivity, buffering and manual device override | No rebinding or automatic device detection |
| `PGXInteraction` | Explicit target registry, selection, prompt snapshots and action lifecycle | No automatic trace or overlap detector |
| `PGXInventory` | Component-based add/remove/stack/transfer with slot and weight limits | No persistence, replication, equipment or global coordination |
| `PGXLoading` | Loading presentation, visual strategies, level flow, streaming and progress | Packaged-target coverage remains open |
| `PGXMGOS` | Garbage-collection observation, snapshots, baselines, class health and incidents | Observation does not replace engine profiling |
| `PGXPSO` | Data-driven PSO warm-up, batching, deduplication, recording/export and contextual activation | Platform-specific pipelines require project validation |
| `PGXSave` | Slots, domains, async save/load, serialization, backup, versioning, migrations and storage providers | Project-specific data compatibility remains the integrator's responsibility |
| `PGXSpawn` | Actor spawning, pooling, request lifecycle, budgets and spawn-wave bookkeeping | Volume registration, spawn-point selection and several conditions are incomplete |
| `PGXTrade` | Offers, reputation, information, promises and transaction records | Does not transfer external items or currency |
| `PGXUI` | Screen-stack, notification and widget-pool state services | Does not automatically instantiate or present UMG widgets |
| `PGXVehicles` | Vehicle registry and claim, park, refuel and repair state operations | No vehicle physics, driving or world binding |
| `PGXDocs` | Documentation roots, registry, Markdown rendering, tree/search/link validation, watching and editor viewer | Automatic provider discovery is not yet a documented guarantee |
| `PGXEditorTools` | Integrated editor hub and inspectors for selected systems and registries | Editor-only aggregation layer |
| `PGXScaffold` | Project analysis, templates, build plans, validation, transactional execution and rollback | Generated output still requires project review |
| `PGXSimHarness` | Editor verification, demo population and partial/missing coverage reporting | A diagnostic tool, not proof that every system is complete |
| `PGXTutorials` | Editor tutorial hub, overlays and guided actions | No stable public extension headers claimed in this preview |
| `PGXVersionControl` | Unreal Source Control overlay for changelists, tags/templates, validation and inspection | Provider behavior depends on the active Unreal source-control integration |

## API stability

Check the selected tag's plugin descriptors, module rules and headers before
depending on a symbol. During the `0.x` series, extension points may move or
change with migration notes.
