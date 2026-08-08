# Quickstart

> [!WARNING]
> PGX is a Development Preview. Use a tagged release, expect `0.x` API changes
> and read its known issues before integrating it into a project.

## Current validated boundary

The current release candidate has been compiled from a clean exported tree with
Unreal Engine 5.6 on a Windows editor target. Packaged builds, other platforms
and other engine versions are not yet part of the published compatibility
claim.

## Install for evaluation

1. Check out a tagged Development Preview release.
2. Close the Unreal Editor.
3. Copy the selected directories from `Plugins/` into your project's `Plugins/`
   directory without renaming them.
4. Include `PGXCore` and every dependency declared by the selected `.uplugin`
   files. Use the complete 12-plugin set for the currently verified
   build boundary.
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
