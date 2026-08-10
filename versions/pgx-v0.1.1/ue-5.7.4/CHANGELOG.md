# Changelog

Public release changes for PGX Framework are recorded here.

PGX follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and uses
Semantic Versioning for tagged public snapshots. During the `0.x` Development
Preview, incompatible API changes may occur between releases and will be called
out explicitly.

## [Unreleased]

No changes recorded.

## [0.1.1] - 2026-08-10

### Added

- A versioned public catalog under `versions/pgx-vX.Y.Z/ue-<engine-version>/` on the single `release`
  branch.
- `Samples/PGXDemo`, a ready-to-open Unreal Engine 5.7.4 project that
  demonstrates Message, GameFlow, Save and InputBuffer.
- Editor and Game verification for the complete 26-plugin constellation.

### Changed

- Updated the supported evaluation boundary to Unreal Engine 5.7.4 on Windows
  Development targets.
- Moved version-specific plugins, documentation and samples beneath their
  immutable version root.
- Clarified that GitHub automatic source archives contain the cumulative
  catalog; the version-specific release artifact is the preferred download.

### Fixed

- Corrected command ownership, editor registration order, sample startup and
  public-layout test discovery.
- Corrected Ability asset tags, GameFlow sorting, Input configuration,
  Loading transitions and Spawn console dispatch.

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

`0.1.0` remains the first source-bearing public release. `0.1.1` is the current
Development Preview target. Every Git tag and GitHub Release remains an
immutable version boundary.
