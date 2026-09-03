# Public roadmap

PGX evolves through versioned Development Preview snapshots. Dates and scope remain provisional until a release passes its export, sanitation, build, automation, documentation, provenance, metadata, and publication gates.

## Published release `v0.1.1`

Completed:

- Unreal Engine 5.7.4 plugin constellation and sample project.
- Editor and Game builds.
- Full PGX Editor, Game, and PGXDemo automation.
- Sanitizer, secret, dependency-closure, provenance, and documentation gates.
- Per-version release metadata, checksums, and deterministic archive assembly.
- Published `release` catalog, immutable tag, pre-release, and version-specific archive.
- Verified remote branch, archive checksum, links, and fresh-clone readback.

## Later previews

- Expand executable examples for partially implemented plugins.
- Stabilize public API contracts and compatibility policy.
- Improve packaging and version-specific download workflows.
- Incorporate community feedback without mutating published snapshots.

## Authoring surfaces

v0.1.1 authors in C++ and Blueprints over DataAssets, through the same flow:
enable the plugin, fill the DataAsset, call the API or place Blueprint nodes.

Verse is the next authoring surface, aligned with Epic's direction for Unreal
Engine. No published snapshot contains a Verse runtime and no date is
committed. When it lands, it builds on the same DataAsset flow. It is not a
genre kit and it is not part of any vertical roadmap page.

See [`RELEASES.json`](RELEASES.json) for machine-readable status.
