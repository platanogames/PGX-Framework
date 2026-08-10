# Known Issues

PGX is a Development Preview. These limits apply to the `v0.1.1` public release.

## Compatibility and validation

- The expanded 26-plugin exported tree passed its documented clean Editor and
  Game rebuilds and Automation run. That result does not validate arbitrary
  plugin combinations.
- `Samples/PGXDemo` demonstrates Message, GameFlow, Save and InputBuffer. The
  other 22 included plugins are loaded and linked but are not demonstrated by
  the sample.
- A broader Unreal Engine compatibility matrix has not yet been established.

## Architecture boundaries

- `PGXDocs` is not currently claimed as packaged-runtime safe.
- The exported snapshot is validated with Unreal Engine 5.7.4 for Editor and
  Game Windows Development targets. Packaged and Shipping targets, other
  platforms and other engine versions remain unvalidated.
- `PGXTutorials` is editor-only and does not expose a stable public extension API
  in this preview.
- `PGXCamera` currently supplies mode/configuration state, not camera blending,
  modifiers or collision resolution.
- `PGXUI` supplies screen, notification and pool state services; it does not
  automatically create UMG widgets or present them in a viewport.
- `PGXInput` does not yet provide rebinding or automatic input-device detection.
- `PGXInteraction` requires explicit target registration; automatic trace and
  overlap detection are not included.
- `PGXInventory` does not yet provide persistence, replication, equipment or
  global inventory coordination.
- `PGXAbility` is an experimental GAS facade. Projects remain responsible for
  ActorInfo and replication setup.
- `PGXSpawn` includes actor spawning, pooling and wave bookkeeping, but volume
  registration, spawn-point selection and several condition types remain incomplete.
- `PGXAI` currently covers agent registration and Behavior Tree dispatch/status,
  not a complete perception, planning or squad system.
- `PGXColony` currently exposes a survivor registry and domain schemas, not
  settlement simulation or policy systems.
- `PGXCrafting` models recipes and job lifecycle without mutating an inventory
  or producing gameplay outputs.
- `PGXEnvironment` has limited editor and Blueprint facade coverage.
- `PGXTrade` records local trade state but does not transfer external items or currency.
- `PGXVehicles` models registry and state operations, not vehicle physics or driving.
- `PGXSimHarness` reports partial and missing coverage rather than treating every
  PGX system as complete.
- The SimHarness `LogRoundtrip` and `EnvironmentSmoke` paths report environmental
  N/A in commandlet or headless contexts without a live game world; build and
  suite results do not establish runtime coverage for those paths.
- Registry 10k, 50k and 100k benchmark entrypoints require project-provisioned
  Gameplay Tags. Runtime tag invention is intentionally rejected.
- `UPGXAssetAuditor`, `UPGXBlueprintAuditor`, `UPGXLevelValidator`, and
  `UPGXDashboard` are compatibility placeholders without behavior in this preview.
- APIs, configuration assets and editor integrations may require migration
  between `0.x` releases.

Production use is not currently recommended. Issues should include the exact PGX
revision, Unreal Engine version, platform, target, reproduction steps and focused
logs with private information removed.
