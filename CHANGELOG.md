# Changelog

Public release changes for PGX Framework are recorded here.

PGX follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and uses
Semantic Versioning for tagged public snapshots. During the `0.x` Development
Preview, incompatible API changes may occur between releases and will be called
out explicitly.

## [Unreleased]

### Added

- Bilingual Development Preview documentation.
- Public monorepo structure for the 12-plugin, 23-module preview.
- Release-boundary documentation, known-issues tracking, roadmap, and third-party notices.
- Architecture maps, dependency reference, runtime flows, plugin catalog and
  verification boundary.

### Changed

- The public distribution model now uses reviewed, immutable release snapshots
  produced from the canonical development monorepo.

### Security

- Public release preparation now requires path, provenance, dependency, license,
  secret, and clean-clone validation before a tag can be created.

## Release policy

The first source-bearing Development Preview will receive a version and release
date only after final review of the exact public bytes. Until then, this section
describes a verified local release candidate rather than a published release.
