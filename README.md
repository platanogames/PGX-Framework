# PGX Framework

> **Development Preview** — PGX is under active development. Published snapshots are intended for architecture review, API exploration, and evaluation. APIs may change incompatibly, and no snapshot is recommended for production use yet.

PGX is an open-source, modular Unreal Engine gameplay framework maintained as a versioned monorepo. Every release is stored as a self-contained snapshot so developers can inspect, compare, and evaluate exact plugin, documentation, and sample-project surfaces without switching branches.

[English](README.md) | [Español](README.es.md)

## Releases

| Version | Status | Unreal Engine | Snapshot |
|---|---|---|---|
| [`v0.1.1`](versions/v0.1.1/README.md) | **Ready for publication** | 5.7.4 | [Plugins](versions/v0.1.1/Plugins/) · [Documentation](versions/v0.1.1/docs/) · [PGXDemo](versions/v0.1.1/Samples/PGXDemo/) |
| [`v0.1.0`](versions/v0.1.0/README.md) | Published | 5.6 | [Plugins](versions/v0.1.0/Plugins/) · [Documentation](versions/v0.1.0/docs/) |

`v0.1.1` has passed its local release gates and is assembled in this catalog. Its remote publication, tag, release artifact, and GitHub readback are not claimed until the publication step completes.

Machine-readable catalog metadata is available in [`RELEASES.json`](RELEASES.json). Each version also contains its own `RELEASE.json` and [`SHA256SUMS`](versions/v0.1.1/SHA256SUMS).

## Current snapshot: `v0.1.1`

The release-ready snapshot contains **26 plugins**, their public source and documentation, and a ready-to-open Unreal Engine 5.7.4 demonstration project.

Validated gates:

- Unreal Editor build: **792 actions**, successful.
- Unreal Game build: **414 actions**, successful.
- PGXDemo automation: **3/3**.
- Editor automation: **242/242**.
- Game automation: **115/115**.
- Demonstration assets: **7/7** verified.
- Sanitizer and secret findings: **0**.
- Missing dependency-closure entries and dependency cycles: **0**.

See the [version README](versions/v0.1.1/README.md), [verification boundary](versions/v0.1.1/docs/validation/verification.md), and [known issues](versions/v0.1.1/KNOWN_ISSUES.md) before evaluating it.

## Using a snapshot

1. Select a version from the table above.
2. Read that version's README, known issues, and verification boundary.
3. Copy the required plugins from `versions/<version>/Plugins/` into your Unreal project's `Plugins/` directory, or open the included sample project where available.
4. Enable only the plugins required by your project and review their declared dependencies.
5. Generate project files and build against the Unreal Engine version documented by that snapshot.
6. Verify downloaded files against the version's `SHA256SUMS` file.

## Repository policy

- The `release` branch is the version catalog.
- Published snapshots under `versions/` are immutable.
- New releases are generated from sanitized, validated exports.
- Tags identify publication events; published tags are never moved.
- Private development infrastructure and internal working documents are not part of this repository.

## Project links

- [Release history](CHANGELOG.md)
- [Known issues index](KNOWN_ISSUES.md)
- [Public roadmap](ROADMAP.md)
- [Contributing](CONTRIBUTING.md)
- [Support](SUPPORT.md)
- [Security policy](SECURITY.md)
- [Code of conduct](CODE_OF_CONDUCT.md)

## License

PGX Framework is distributed under the [Apache License 2.0](LICENSE.md). Third-party components remain subject to their respective terms; see [`NOTICE`](NOTICE) and the notices within each release snapshot.
