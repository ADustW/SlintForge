---
title: ForgeFlowPanel
description: ForgeFlowPanel public API, behavior, example, and host integration.
---

# ForgeFlowPanel

`ForgeFlowPanel` is a public component in the **ForgeFlow** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-flow-panel" name="ForgeFlowPanel" />

## Purpose and usage

- **Use when:** composing a forgeflow screen that needs the behavior represented by `ForgeFlowPanel`.
- **Inheritance:** `ForgeFlowPanel` → `Rectangle`.
- **Source:** `ui/flow/panel.slint:4`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeFlowPanel } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `horizontal-anchor` | `in` | `int` | `2` | Constrained by its type and default expression | Host-owned input |
| `vertical-anchor` | `in` | `int` | `0` | Constrained by its type and default expression | Host-owned input |
| `margin` | `in` | `length` | `12px` | Constrained by its type and default expression | Host-owned input |
| `canvas-width` | `in` | `length` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `canvas-height` | `in` | `length` | `Type default` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Default slot | `default` | `@children` | Hosts Slint children supplied by the caller. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses its declared colors and inherited Slint styling.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-flow-panel.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/flow/forge-flow-panel)
