# PGX and Unreal Engine compatibility

Use the framework and engine directory together. A PGX version tag is an immutable release boundary; the paths below are navigation surfaces in the `release` branch catalog.

| PGX Framework | Compatible Unreal Engine | Precision | Canonical path | Engine-explicit artifact | Validation |
|---|---|---|---|---|---|
| `v0.1.0` | Unreal Engine 5.6 | Minor | [`versions/pgx-v0.1.0/ue-5.6/`](versions/pgx-v0.1.0/ue-5.6/) | `PGX-Framework-v0.1.0-UE5.6.zip` | The exact 5.6 patch was not recorded. The canonical release download is pending the remote publication lane. |
| `v0.1.1` | Unreal Engine 5.7.4 | Patch | [`versions/pgx-v0.1.1/ue-5.7.4/`](versions/pgx-v0.1.1/ue-5.7.4/) | `PGX-Framework-v0.1.1-UE5.7.4.zip` | Fully validated within the documented Windows Development boundary. Remote asset replacement and readback belong to the publication lane. |

Do not infer compatibility with another engine patch or minor version from these entries. Read the selected snapshot's README, verification boundary, and known issues before evaluation.
