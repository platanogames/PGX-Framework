# PGXDemo — Unreal Engine 5.7.4 Development Preview

> **Development Preview:** this sample and the PGX APIs are under active development. Expect incomplete areas and breaking changes. Do not use this preview in production.

PGXDemo is a small, redistributable project that demonstrates four public runtime surfaces with a visible interaction loop:

- **PGX Message:** typed `PGX.Demo.Event.Interacted` broadcast and listener.
- **PGX GameFlow:** deterministic `Ready` / `Interacted` state transitions.
- **PGX Save:** an interaction counter saved and loaded from `DemoSlot`.
- **PGX Input:** tagged `UPGXInputBuffer` record-and-consume behavior.

Press **E** in the generated map. The HUD reports the count, current flow state and save result.

The project enables the exact public 26-plugin constellation. The other **22 plugins are load/link coverage only** in this minimal sample; PGXDemo does not claim to exercise their runtime behavior.

## Current Input limitation

`UPGXInputSettings::ActiveConfig` exists as a reserved settings shape, but the current runtime does not consume it. PGXDemo therefore binds the E key with the Engine input surface and demonstrates only the public PGX input buffer. It does not claim DataAsset-driven input-context activation.

## Author the seven packages

Binary Unreal packages are never fabricated or copied from another project. Build the Editor target, then run:

```text
UnrealEditor-Cmd.exe PGXDemo.uproject -run=PGXDemoAuthor -create -receipt=<absolute-output-json> -unattended -nop4 -NullRHI -stdout
UnrealEditor-Cmd.exe PGXDemo.uproject -run=PGXDemoAuthor -verify -receipt=<same-output-json> -unattended -nop4 -NullRHI -stdout
```

`-create` fails closed if any of the seven destination packages already exists. There is no force or overwrite mode. A second `-create` must fail without changing any bytes.

## Content policy

The generated map uses Engine primitives by reference only. No Marketplace, Megascans, third-party or private-project asset is copied into PGXDemo.
