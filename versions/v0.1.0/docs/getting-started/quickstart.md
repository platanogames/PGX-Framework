# Quickstart

> [!WARNING]
> PGX is a Development Preview. Use a tagged release, expect `0.x` API changes
> and read its known issues before integrating it into a project.

## Current verification boundary

The complete exported snapshot passed clean Editor and Game rebuilds and 344/344
Automation tests on Unreal Engine 5.6 with Windows Development targets. Packaged
builds, other platforms and other engine versions are outside the current claim.
See [Verification](../validation/verification.md) for exact counts and limits.

## Install for evaluation

1. Check out a tagged Development Preview release.
2. Close the Unreal Editor.
3. Copy the selected directories from `Plugins/` into your project's `Plugins/`
   directory without renaming them.
4. Include `PGXCore` and every dependency declared by the selected `.uplugin`
   files. The verified 26-plugin result does not validate arbitrary subsets.
5. Regenerate project files.
6. Build your editor target before opening the project.
7. Enable and configure one system at a time, keeping its settings and Data
   Assets under source control.

If Unreal reports a missing plugin or module, do not bypass the descriptor. Add
the declared dependency or disable the plugin that requires it.

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
