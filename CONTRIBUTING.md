# Contributing to PGX Framework

Thank you for helping improve PGX during its Development Preview.

## Before contributing

1. Read the [README](README.md), [known issues](KNOWN_ISSUES.md), and the notes
   for the release you are using.
2. Search existing issues and pull requests.
3. Keep the change focused on one problem.
4. Remove credentials, private source, licensed assets, complete project
   archives, and unrelated logs from every report and fixture.

## Issues

A useful report includes:

- the exact PGX revision or tag;
- Unreal Engine version and target platform;
- the smallest reproducible example;
- expected and observed behaviour;
- sanitized logs, screenshots, or call stacks when relevant.

Use a [security report](SECURITY.md), not a public issue, for suspected
vulnerabilities.

## Pull requests

Public pull requests are welcome for the source and documentation present in
this repository.

- Use a concise imperative title.
- Explain the problem, scope, design trade-offs, and compatibility impact.
- Add or update focused tests for the changed API or behavior.
- Document breaking changes and migration steps.
- Keep generated output, editor caches, binaries, and unrelated formatting out
  of the diff.
- Confirm that you have the right to contribute every included file.

PGX uses a private canonical development monorepo and publishes reviewed public
release snapshots. Accepted public contributions are reconciled with that line
while preserving their authorship, then included in a later release after the
same validation gates.

## Review expectations

Preview APIs may change, but every change should still be testable and
traceable. Maintainers may request a smaller reproduction, additional tests,
or a revised publication scope before merging.

By contributing, you agree that your contribution is licensed under the
repository's [Apache License 2.0](LICENSE.md).
