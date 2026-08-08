# Verification and Compatibility

## Evidence for the current candidate

The current 12-plugin candidate has passed these bounded checks:

- explicit file allowlist and internal-provenance scan;
- PGX descriptor and module dependency-closure validation;
- JSON and YAML parsing for public descriptors and repository forms;
- local Markdown-link validation;
- deterministic source inventory and file hashes;
- a clean exported-tree `PGXPublicRCEditor` build on Windows with Unreal Engine
  5.6, including UnrealHeaderTool and 514 build actions.

The build result applies to the exact candidate tree from which it was produced.
A tagged release should carry its own revision and release notes so this evidence
can be correlated to published bytes.

## What this does not establish

- packaged Development or Shipping builds;
- runtime behavior in a complete sample game;
- platforms other than the validated Windows editor target;
- compatibility with every Unreal Engine 5 release;
- arbitrary subsets of the 12-plugin set;
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
