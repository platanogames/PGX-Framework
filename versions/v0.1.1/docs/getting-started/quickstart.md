# Quickstart

> [!WARNING]
> PGX is a Development Preview. Use a tagged release, expect `0.x` API changes
> and read its known issues before integrating it into a project.

## Current verification boundary

The complete exported snapshot passed 792/792 Editor build actions, 414/414 Game
build actions, 242/242 Editor Automation tests, 115/115 Game Automation tests
and 3/3 `PGX.Demo` tests on Unreal Engine 5.7.4 with Windows Development targets.
Packaged builds, Shipping, other platforms and other engine versions are outside
the current claim. See [Verification](../validation/verification.md) for limits.

## Install for evaluation

1. Prefer the version-specific artifact attached to the `v0.1.1` GitHub Release.
   GitHub automatic source archives contain the cumulative version catalog.
2. Close the Unreal Editor.
3. From `versions/v0.1.1/`, copy the selected directories from `Plugins/` into
   your project's `Plugins/` directory without renaming them.
4. Include `PGXCore` and every dependency declared by the selected `.uplugin`
   files. The verified 26-plugin result does not validate arbitrary subsets.
5. Regenerate project files.
6. Build your editor target before opening the project.
7. Enable and configure one system at a time, keeping its settings and Data
   Assets under source control.

If Unreal reports a missing plugin or module, do not bypass the descriptor. Add
the declared dependency or disable the plugin that requires it.

## Open the example project

Open `versions/v0.1.1/Samples/PGXDemo/PGXDemo.uproject` to inspect the complete
constellation in a configured project. The example demonstrates Message,
GameFlow, Save and InputBuffer. The other 22 plugins are present for dependency,
loading and linking checks rather than runtime demonstrations.

## Suggested reading order

1. [Architecture overview](../architecture/overview.md)
2. [Modules and dependencies](../architecture/modules-and-dependencies.md)
3. [Plugin catalog](../plugins/catalog.md)
4. [Runtime flows](../architecture/runtime-flows.md)
5. [Verification boundary](../validation/verification.md)

## Report a result

Use a minimal project and record the PGX revision, Unreal Engine version,
platform, target, enabled plugins and exact reproduction steps. Remove private
project code, credentials, and unrelated logs before attaching a report.
