# Asset provenance

This manifest records the seven project-authored Unreal packages included with PGXDemo.

## Generation record

- Receipt schema: `1.0.0`
- Project: `PGXDemo`
- Engine association: `5.7`
- Engine build: `5.7.4`
- Generator commandlet: `PGXDemoAuthor`
- Generator source: `Source/PGXDemoEditor/Private/PGXDemoAuthorCommandlet.cpp`
- Generator source SHA-256: `c26c928f12f451659215119fecfd61ae9feeca2f69579b8b164e3ba89b17d5b9`

## Included packages

| Package | Class | File | SHA-256 | Provenance | License | Dependencies |
|---|---|---|---|---|---|---|
| `/Game/Maps/PGXDemo` | `World` | `Content/Maps/PGXDemo.umap` | `41e2c206f867e2d6ab3f4e0f864aebeec3bdecf13375ce8e450d19bbbb4ddcf0` | `generated-by-PGXDemoAuthor-from-public-source` | `Apache-2.0` | `/Engine/BasicShapes/Cube` (engine-reference-only) |
| `/Game/PGXDemo/Config/DA_PGXDemoMessage` | `PGXMessageConfig` | `Content/PGXDemo/Config/DA_PGXDemoMessage.uasset` | `521a883e5fbbecdb54494e34ef52c91ef26ad969f525d63359688ab407ad15ce` | `generated-by-PGXDemoAuthor-from-public-source` | `Apache-2.0` | None |
| `/Game/PGXDemo/Config/DA_PGXDemoGameFlow` | `PGXGameFlowConfig` | `Content/PGXDemo/Config/DA_PGXDemoGameFlow.uasset` | `14f373bcac2f60f5847fd8096350267f08f177d8b7035f20c3fd3aea3904af0f` | `generated-by-PGXDemoAuthor-from-public-source` | `Apache-2.0` | None |
| `/Game/PGXDemo/Config/DA_PGXDemoGlobalRules` | `PGXFlowRulesConfig` | `Content/PGXDemo/Config/DA_PGXDemoGlobalRules.uasset` | `05906de189d454dd013ccc87341f2b5a934c44042dcd653b00e7c9aea280b498` | `generated-by-PGXDemoAuthor-from-public-source` | `Apache-2.0` | None |
| `/Game/PGXDemo/Config/DT_PGXDemoFlowRules` | `DataTable` | `Content/PGXDemo/Config/DT_PGXDemoFlowRules.uasset` | `fb8b9c902d8abf102a802af45ea0cdf99db5f6f9455cea5b5e80c04e04a63acc` | `generated-by-PGXDemoAuthor-from-public-source` | `Apache-2.0` | None |
| `/Game/PGXDemo/Config/DA_PGXDemoSave` | `PGXSaveConfig` | `Content/PGXDemo/Config/DA_PGXDemoSave.uasset` | `7f4881ab3ab26ba3f9354dcdcf2a89cc6aed283820ae58f7391504533a5815b9` | `generated-by-PGXDemoAuthor-from-public-source` | `Apache-2.0` | None |
| `/Game/PGXDemo/Config/DT_PGXDemoSaveContexts` | `DataTable` | `Content/PGXDemo/Config/DT_PGXDemoSaveContexts.uasset` | `6872e7c54258c1aad2f8a2097093b6db36990aa6925fe726c47ea8edf2e308db` | `generated-by-PGXDemoAuthor-from-public-source` | `Apache-2.0` | None |

## Licensing and engine references

These seven project-authored packages are identified as Apache-2.0 in the accepted generation receipt.
The demo map references `/Engine/BasicShapes/Cube` as an engine prerequisite. That Engine asset is not redistributed by this repository.
