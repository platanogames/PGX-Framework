# Public roadmap

PGX evolves through versioned Development Preview snapshots. Dates and scope remain provisional until a release passes its export, sanitation, build, automation, documentation, provenance, metadata, and publication gates.

## `v0.1.1`: ready for publication

Completed locally:

- Unreal Engine 5.7.4 plugin constellation and sample project.
- Editor and Game builds.
- Full PGX Editor, Game, and PGXDemo automation.
- Sanitizer, secret, dependency-closure, provenance, and documentation gates.
- Per-version release metadata, checksums, and deterministic archive assembly.

Remaining publication boundary:

- Commit and push the `release` catalog.
- Create and verify the intended tag and release artifact.
- Confirm remote branch, archive checksum, links, and GitHub readback.

## Later previews

- Expand executable examples for partially implemented plugins.
- Stabilize public API contracts and compatibility policy.
- Improve packaging and version-specific download workflows.
- Incorporate community feedback without mutating published snapshots.

See [`RELEASES.json`](RELEASES.json) for machine-readable status.
