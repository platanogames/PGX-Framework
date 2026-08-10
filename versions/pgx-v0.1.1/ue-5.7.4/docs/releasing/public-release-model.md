# Public Release Model

PGX uses two coordinated repositories:

- a private canonical monorepo for active development;
- this public monorepo for reviewed, versioned release snapshots.

The public repository distributes releases and accepts contributions; it is not
a mirror of day-to-day development. A release is exported from an explicit
allowlist and accepted only after the resulting artifact has been inspected
independently of the source worktree.

## Repository layout

The public repository has one `release` branch. Each accepted snapshot lives at
`versions/pgx-vX.Y.Z/ue-<engine-version>/` and contains its own plugins, documentation and samples. An
older version directory is preserved when a newer version is added.

Git tags and GitHub Releases mark immutable publication boundaries. The
version-specific artifact attached to a GitHub Release is the preferred
installation download. GitHub's automatic source archives contain the entire
cumulative catalog, not only the tagged version directory.

## Required gates

Before a source-bearing tag is published, maintainers verify:

1. **Scope:** every included path is public and every excluded path remains absent.
2. **Dependency closure:** selected plugins include every required PGX module or
   document an optional boundary that was actually compiled.
3. **Provenance and licensing:** source ownership and third-party notices are complete.
4. **Sanitization:** no credentials, local paths, private workflow data, generated
   artifacts, or internal development material remain.
5. **Clean-clone build:** the documented target builds from the candidate artifact.
6. **Verification:** focused automated and editor-level checks pass for the stated
   Unreal Engine version.
7. **Documentation:** installation, limitations, migration notes, and the release
   manifest match the artifact.
8. **Readback:** the final tag or archive is downloaded again and compared with the
   approved candidate.

Creating files, a tag, or an upload is not by itself evidence that these gates
passed.

## Immutability

Published tags and release assets are not rewritten. A correction is delivered
through a new version with an updated changelog and, when necessary, migration
notes.

## Public contributions

Accepted public contributions retain their authorship. They are reconciled with
the canonical development line and return in a later public snapshot after the
same release gates.
