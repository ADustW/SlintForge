---
title: ForgeFlowLayoutControls
description: ForgeFlowLayoutControls public API, behavior, example, and host integration.
---

# ForgeFlowLayoutControls

`ForgeFlowLayoutControls` is a public component in the **ForgeFlow** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-flow-layout-controls" name="ForgeFlowLayoutControls" />

## Purpose and usage

- **Use when:** composing a forgeflow screen that needs the behavior represented by `ForgeFlowLayoutControls`.
- **Inheritance:** `ForgeFlowLayoutControls` → `Rectangle`.
- **Source:** `ui/flow/layout-controls.slint:6`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeFlowLayoutControls } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `chinese` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `read-only` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `tree-mode` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `algorithm` | `in-out` | `int` | `0` | Constrained by its type and default expression | Shared by host and component |
| `direction` | `in-out` | `int` | `0` | Constrained by its type and default expression | Shared by host and component |
| `node-gap` | `in-out` | `int` | `32` | Constrained by its type and default expression | Shared by host and component |
| `rank-gap` | `in-out` | `int` | `80` | Constrained by its type and default expression | Shared by host and component |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `action` | `(ForgeFlowAction)` | Emitted by the component and handled by the host. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses its declared colors and inherited Slint styling.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-flow-layout-controls.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/flow/forge-flow-layout-controls)
