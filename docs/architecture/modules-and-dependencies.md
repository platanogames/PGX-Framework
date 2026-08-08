# Modules and Dependencies

The current public candidate contains 23 Unreal modules: 8 `Runtime`, 12
`Editor`, 2 `UncookedOnly` and 1 `DeveloperTool`.

| Plugin | Modules | Direct PGX plugin dependencies |
|---|---|---|
| `PGXCore` | `PGXCoreRuntime`, `PGXCoreEditor`, `PGXCoreNodes`, `PGXCoreDeveloper`, `PGXRegistryEditor` | None |
| `PGXAudio` | `PGXAudioRuntime`, `PGXAudioEditor` | `PGXCore` |
| `PGXGameFlow` | `PGXGameFlowRuntime`, `PGXGameFlowEditor` | `PGXCore` |
| `PGXLoading` | `PGXLoadingRuntime`, `PGXLoadingEditor` | `PGXCore` |
| `PGXMGOS` | `PGXMGOSRuntime`, `PGXMGOSEditor` | `PGXCore` |
| `PGXPSO` | `PGXPSORuntime`, `PGXPSOEditor` | `PGXCore` |
| `PGXSave` | `PGXSaveRuntime`, `PGXSaveNodes` | `PGXCore` |
| `PGXDocs` | `PGXDocs`, `PGXDocsEditor` | `PGXCore` |
| `PGXEditorTools` | `PGXEditorToolsEditor` | Core, six runtime systems and Version Control |
| `PGXScaffold` | `PGXScaffoldEditor` | `PGXCore` |
| `PGXTutorials` | `PGXTutorialsEditor` | `PGXCore` |
| `PGXVersionControl` | `PGXVersionControlEditor` | `PGXCore` |

Engine plugin dependencies also exist: Core uses `SlateIM` and `StructUtils`,
Audio uses `AudioModulation`, and Tutorials uses
`EditorScriptingUtilities`. Unreal's own module dependencies remain listed in
each `Build.cs` file.

## Selection rules

- Start with `PGXCore`.
- Add a runtime system together with its declared engine dependencies.
- Add that system's editor module when developing with the Unreal Editor.
- Enable `PGXEditorTools` only when its complete declared plugin set is present.
- Treat `UncookedOnly` and `DeveloperTool` modules according to the target being
  built; their presence in the source tree is not a packaged-runtime guarantee.

The repository is validated as the complete 12-plugin set. Smaller
subsets must be checked against their descriptors and build rules rather than
assumed to work in arbitrary combinations.

## Content-bearing plugins

Only `PGXDocs` and `PGXPSO` currently declare `CanContainContent=true`. The
remaining plugins in this candidate are source/configuration plugins according
to their descriptors.
