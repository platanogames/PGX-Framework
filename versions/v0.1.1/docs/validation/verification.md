# Verification and Compatibility

## Evidence for the verified preview snapshot

The `v0.1.1` preview snapshot contains **26 plugins and 48 Unreal modules** in
one compatible constellation. On Unreal Engine 5.7.4 with Windows Development
targets, the exact exported candidate completed:

- clean Editor build: 792/792 actions passed;
- clean Game build: 414/414 actions passed;
- Editor Automation: 242/242 tests passed;
- Game Automation: 115/115 tests passed;
- `PGX.Demo`: 3/3 tests passed.

The release preparation also completed:

- explicit plugin and file allowlists;
- forbidden-content and internal-provenance scans;
- descriptor parsing and static dependency-closure review;
- JSON, YAML and Markdown-link validation;
- focused UnrealBuildTool and UnrealHeaderTool builds for selected plugin lanes.

`PGXSimHarness` is supporting evidence only. It reports implemented, partial and
missing coverage; it is not a completeness certificate.

The SimHarness `LogRoundtrip` and `EnvironmentSmoke` paths report environmental
N/A in commandlet or headless contexts without a live game world. Compilation,
linking and a passing suite do not turn those paths into runtime coverage.

The Registry 10k, 50k and 100k benchmark entrypoints require the integrating
project to provision their corresponding Gameplay Tags. The Registry
intentionally rejects inventing tags at runtime.

`Samples/PGXDemo` demonstrates Message, GameFlow, Save and InputBuffer. The
other 22 plugins are present for dependency, loading and linking validation;
the sample does not demonstrate their runtime behavior.

## What this does not establish

- packaged Development or Shipping builds;
- complete runtime behavior across all included plugins;
- platforms other than the current Windows editor evaluation environment;
- compatibility with every Unreal Engine 5 release;
- arbitrary plugin subsets;
- stable ABI or source compatibility between `0.x` releases.

## Reporting a result

When opening an issue, include:

1. PGX tag or commit;
2. Unreal Engine version and source/launcher distribution;
3. platform, target and configuration;
4. enabled plugin list;
5. clean reproduction steps;
6. expected and observed behavior;
7. a focused log with private paths, credentials and project code removed.

See [Known Issues](../../KNOWN_ISSUES.md) for the current preview limits.
