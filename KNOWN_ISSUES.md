# Known Issues

PGX is a Development Preview. These limits apply to the current public candidate.

## Compatibility and validation

- A clean exported-tree Unreal Engine 5.6 Windows editor build has succeeded,
  but packaged targets and other platforms have not yet been published as
  validated.
- The complete 12-plugin set is the verified build boundary. Smaller
  combinations must be checked against their descriptors and module rules.
- The example project and reproducible runtime walkthrough are still being
  prepared.
- A broader Unreal Engine compatibility matrix has not yet been established.

## Architecture boundaries

- `PGXDocs` is not currently claimed as packaged-runtime safe.
- `PGXGameFlowRuntime` and `PGXSaveRuntime` still use Core developer support in
  their current module rules; target coverage beyond the validated editor build
  remains open.
- `PGXTutorials` is editor-only and does not expose a stable public extension API
  in this preview.
- `UPGXAssetAuditor`, `UPGXBlueprintAuditor`, `UPGXLevelValidator`, and
  `UPGXDashboard` are compatibility placeholders without behavior in this preview.
- APIs, configuration assets and editor integrations may require migration
  between `0.x` releases.

Production use is not currently recommended. Issues should include the exact PGX
revision, Unreal Engine version, platform, target, reproduction steps and focused
logs with private information removed.
