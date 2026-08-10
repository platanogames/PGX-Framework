# Changelog

Public release changes for PGX Framework are recorded here.

PGX follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and uses
Semantic Versioning for tagged public snapshots. During the `0.x` Development
Preview, incompatible API changes may occur between releases and will be called
out explicitly.

## [Unreleased]

No changes recorded.

## [0.1.0] - 2026-08-09

### Added

- Bilingual Development Preview documentation.
- Public monorepo structure for the expanded 26-plugin, 48-module preview.
- Functional and structured preview systems for ability, AI, camera, colony,
  crafting, environment, input, interaction, inventory, spawning, trade, UI and vehicles.
- PGXSimHarness as an editor verification and demonstration tool.
- Release-boundary documentation, known-issues tracking, roadmap, and third-party notices.
- Architecture maps, dependency reference, runtime flows, plugin catalog and
  verification boundary.

### Changed

- The public distribution model now uses reviewed, immutable release snapshots
  produced from the canonical development monorepo.
- Historical `stub` labels now distinguish functional previews, structured
  previews, concept shells and true stubs.

### Security

- Public release preparation now requires path, provenance, dependency, license,
  secret, and clean-clone validation before a tag can be created.

## Release status

`0.1.0` is the first source-bearing public release. Its Git tag is `v0.1.0`, and
the GitHub release is marked as a pre-release / Development Preview.
