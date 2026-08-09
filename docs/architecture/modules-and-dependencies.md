# Modules and Dependencies

The `v0.1.0` public release contains 48 Unreal modules: 21 `Runtime`, 24
`Editor`, 2 `UncookedOnly` and 1 `DeveloperTool`.

| Plugin | Modules | Direct PGX plugin dependencies |
|---|---|---|
| `PGXCore` | `PGXCoreRuntime`, `PGXCoreEditor`, `PGXCoreNodes`, `PGXCoreDeveloper`, `PGXRegistryEditor` | None |
| `PGXAbility` | `PGXAbilityRuntime`, `PGXAbilityEditor` | `PGXCore` |
| `PGXAI` | `PGXAIRuntime`, `PGXAIEditor` | `PGXCore` |
| `PGXAudio` | `PGXAudioRuntime`, `PGXAudioEditor` | `PGXCore` |
| `PGXCamera` | `PGXCameraRuntime`, `PGXCameraEditor` | `PGXCore` |
| `PGXColony` | `PGXColonyRuntime` | `PGXCore` |
| `PGXCrafting` | `PGXCraftingRuntime`, `PGXCraftingEditor` | `PGXCore` |
| `PGXEnvironment` | `PGXEnvironmentRuntime` | `PGXCore` |
| `PGXGameFlow` | `PGXGameFlowRuntime`, `PGXGameFlowEditor` | `PGXCore` |
| `PGXInput` | `PGXInputRuntime`, `PGXInputEditor` | `PGXCore` |
| `PGXInteraction` | `PGXInteractionRuntime`, `PGXInteractionEditor` | `PGXCore` |
| `PGXInventory` | `PGXInventoryRuntime`, `PGXInventoryEditor` | `PGXCore` |
| `PGXLoading` | `PGXLoadingRuntime`, `PGXLoadingEditor` | `PGXCore` |
| `PGXMGOS` | `PGXMGOSRuntime`, `PGXMGOSEditor` | `PGXCore` |
| `PGXPSO` | `PGXPSORuntime`, `PGXPSOEditor` | `PGXCore` |
| `PGXSave` | `PGXSaveRuntime`, `PGXSaveNodes` | `PGXCore` |
| `PGXSpawn` | `PGXSpawnRuntime`, `PGXSpawnEditor` | `PGXCore` |
| `PGXTrade` | `PGXTradeRuntime`, `PGXTradeEditor` | `PGXCore` |
| `PGXUI` | `PGXUIRuntime`, `PGXUIEditor` | `PGXCore` |
| `PGXVehicles` | `PGXVehiclesRuntime`, `PGXVehiclesEditor` | `PGXCore` |
| `PGXDocs` | `PGXDocs`, `PGXDocsEditor` | `PGXCore` |
| `PGXEditorTools` | `PGXEditorToolsEditor` | Core, Version Control and six established runtime systems |
| `PGXScaffold` | `PGXScaffoldEditor` | `PGXCore` |
| `PGXSimHarness` | `PGXSimHarnessEditor` | Core, Editor Tools, Docs and selected runtime previews used by the harness |
| `PGXTutorials` | `PGXTutorialsEditor` | `PGXCore` |
| `PGXVersionControl` | `PGXVersionControlEditor` | `PGXCore` |

Engine plugin dependencies also exist. Core uses `SlateIM` and `StructUtils`,
Audio uses `AudioModulation`, Ability uses the Gameplay Ability System, and
Tutorials uses `EditorScriptingUtilities`. Unreal module dependencies remain
listed in each `Build.cs` file.

## Selection rules

- Start with `PGXCore`.
- Add a runtime system together with every dependency declared in its descriptor.
- Add that system's editor module only for editor targets.
- Enable `PGXEditorTools` only when its complete declared plugin set is present.
- Enable `PGXSimHarness` only with every plugin declared by its descriptor.
- Treat `UncookedOnly` and `DeveloperTool` modules according to the target being built.

The 26-plugin set is the selected release boundary and passed the documented
clean exported-tree Editor and Game rebuilds. Smaller subsets must be checked
against descriptors and module rules rather than assumed to work in arbitrary
combinations.

## Content-bearing plugins

Only `PGXDocs` and `PGXPSO` currently declare `CanContainContent=true`. The
remaining plugins in this release are source/configuration plugins according
to their descriptors.
