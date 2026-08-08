# PGX Framework

**Professional Game Extensions for Unreal Engine 5**

## Development Preview

> [!WARNING]
> **PGX is under active development.** This public preview is available so
> Unreal Engine developers can inspect the architecture and APIs, evaluate the
> included systems, and provide useful technical feedback.
>
> APIs, packaging, compatibility boundaries, and contributor workflows may
> change between `0.x` releases. PGX is intended for evaluation and development
> use at this stage and is **not yet recommended for production projects**.

[![Status: Development Preview](https://img.shields.io/badge/status-development_preview-orange.svg)](#development-preview)
[![License: Apache-2.0](https://img.shields.io/badge/license-Apache--2.0-green.svg)](LICENSE.md)

English | [Español](README.es.md)

PGX is a set of Unreal Engine 5 plugins for projects that need shared
configuration, messaging, persistence, loading, audio, and editor diagnostics.
It uses Unreal subsystems, Data Assets, and Gameplay Tags directly, and keeps
its C++ extension points visible.

## Preview status

| Area | Current policy |
|---|---|
| Release stage | Development Preview (`0.x`) |
| Intended use | Architecture and API evaluation, learning, testing, and feedback |
| API stability | Breaking changes may occur between preview releases |
| Production use | Not currently recommended |
| License | Apache-2.0, subject to the notices and third-party terms described in this repository |

Supported engine versions, installation steps, and build results are published
only when they have been validated for a specific release. Consult that
release's notes before evaluating PGX in an Unreal Engine project.

The current release candidate has been compiled from a clean exported tree with
the Unreal Engine 5.6 Windows editor toolchain. That result is evidence for this
candidate only; it is not yet a cross-platform or packaged-game compatibility
claim.

## Development and release model

Day-to-day development takes place in a private canonical monorepo. This public
repository receives curated, reviewed release snapshots containing only the
source, documentation, tests, and project files approved for public
distribution.

Published release tags are treated as immutable snapshots. Fixes and changes
move forward through a new release instead of rewriting an existing one.

Private working material, unfinished components, generated files, caches, and
content without a verified redistribution boundary are excluded from the public
distribution.

Public issues and pull requests remain part of the project feedback loop. After
review, accepted changes are reconciled with the canonical development line and
included in a subsequent public release.

## What the public monorepo contains

```text
PGX-Framework/
├── Plugins/              Curated PGX runtime and editor plugins
├── docs/                 Versioned public guides
├── .github/              Contribution and repository workflows
├── README.md             English project overview
├── README.es.md          Spanish project overview
├── CHANGELOG.md          Public change history
├── CONTRIBUTING.md       Contribution guidance
├── SECURITY.md           Vulnerability reporting policy
├── SUPPORT.md            Support request guidance
├── LICENSE.md            Apache License 2.0
└── NOTICE                Attribution and third-party boundary
```

The contents of `Plugins/` define the scope of each public snapshot. Components
that are not present in a tagged release are not part of that release's public
API or support boundary.

## Included plugins

The first public candidate contains **12 plugins and 23 Unreal modules**.

| Layer | Plugins |
|---|---|
| Foundation | `PGXCore` |
| Runtime systems | `PGXAudio`, `PGXGameFlow`, `PGXLoading`, `PGXMGOS`, `PGXPSO`, `PGXSave` |
| Editor tools | `PGXDocs`, `PGXEditorTools`, `PGXScaffold`, `PGXTutorials`, `PGXVersionControl` |

Most feature plugins depend on `PGXCore`. `PGXEditorTools` is an editor-only
aggregation layer that brings the inspectors for the selected runtime systems
together. Consult the [plugin catalog](docs/plugins/catalog.md) and
[dependency map](docs/architecture/modules-and-dependencies.md) before enabling
a subset.

## Architecture at a glance

PGX is organized as Unreal Engine plugins rather than an all-or-nothing game
template.

- Runtime code and editor tools live in separate modules.
- Configuration and communication use Unreal Data Assets, subsystems, and
  Gameplay Tags.
- The source exposes C++ and Blueprint APIs together with inspectors and
  validators for the included systems.

Plugins have explicit dependency boundaries, but this candidate was built only
as the complete 12-plugin set. Validate any smaller selection against its
descriptors and module rules.

## Explore the preview

1. Browse the [`Plugins/`](Plugins/) directory.
2. Open a plugin descriptor to inspect its modules and dependencies.
3. Review its `Source/` tree and any documentation shipped with that plugin.
4. Check the public [`CHANGELOG`](CHANGELOG.md) for release-level changes.
5. Use the repository issue forms for reproducible questions or findings.

For the verified Windows editor-build path, follow the
[Quickstart](docs/getting-started/quickstart.md). A maintained example project
and packaged-build guide are still in preparation.

## Documentation

Documentation distributed with a release lives beside the relevant public
source whenever possible. This keeps architectural and API guidance tied to the
snapshot it describes.

Start with:

- [`docs/getting-started/quickstart.md`](docs/getting-started/quickstart.md): preview evaluation checklist
- [`docs/architecture/overview.md`](docs/architecture/overview.md): monorepo and plugin boundaries
- [`docs/architecture/system-map.md`](docs/architecture/system-map.md): layers and cross-plugin flows
- [`docs/architecture/modules-and-dependencies.md`](docs/architecture/modules-and-dependencies.md): exact plugin and module topology
- [`docs/plugins/catalog.md`](docs/plugins/catalog.md): responsibilities and extension points for all 12 plugins
- [`docs/validation/verification.md`](docs/validation/verification.md): verified checks and their limits
- [Project Wiki](https://github.com/platanogames/PGX-Framework/wiki): extended architecture and workflow guides
- [`Plugins/`](Plugins/): plugin descriptors, source, and included plugin docs
- [`CHANGELOG.md`](CHANGELOG.md): public changes and release history
- [`KNOWN_ISSUES.md`](KNOWN_ISSUES.md): verified preview limitations
- [`ROADMAP.md`](ROADMAP.md): public milestones toward a stable contract
- [`docs/releasing/public-release-model.md`](docs/releasing/public-release-model.md): release gates and snapshot policy
- [`CONTRIBUTING.md`](CONTRIBUTING.md): contribution workflow
- [`SUPPORT.md`](SUPPORT.md): information for useful support requests
- [`SECURITY.md`](SECURITY.md): private vulnerability reporting guidance

## Toward 1.0

The `0.x` series is the period in which PGX will consolidate its public
contract. Work toward `1.0` includes:

- reproducible installation from a clean clone;
- a tested Unreal Engine compatibility matrix;
- maintained examples and validation workflows;
- a documented public API and deprecation policy;
- stable contributor and release processes;
- migration notes for breaking changes.

`1.0` will mean that the selected public contract is stable. It will not mean
that every possible PGX system or future extension is finished.

## Contributing

PGX is being published before `1.0` so developers can review its architecture
and APIs before those interfaces become costly to change.

Before opening an issue or pull request, read [`CONTRIBUTING.md`](CONTRIBUTING.md).
Keep reports reproducible and remove credentials, private project code,
licensed assets, and unrelated logs from examples.

## Security and support

Do not disclose suspected vulnerabilities in a public issue. Follow
[`SECURITY.md`](SECURITY.md) for the private reporting path.

For integration questions and support requests, follow [`SUPPORT.md`](SUPPORT.md)
and include the exact PGX revision, Unreal Engine version, target platform,
reproduction steps, expected result, and observed result.

## License and provenance

PGX Framework is distributed under the [Apache License 2.0](LICENSE.md).

Third-party material, when present, remains governed by its respective terms.
See [`NOTICE`](NOTICE) and the headers in individual files for the applicable
attribution and licensing boundary.

Copyright 2024-2026 Platano Games.
