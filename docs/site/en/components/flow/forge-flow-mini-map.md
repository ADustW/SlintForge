---
title: ForgeFlowMiniMap
description: ForgeFlowMiniMap public API, behavior, example, and host integration.
---

# ForgeFlowMiniMap

`ForgeFlowMiniMap` is a public component in the **ForgeFlow** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-flow-mini-map" name="ForgeFlowMiniMap" />

## Purpose and usage

- **Use when:** composing a forgeflow screen that needs the behavior represented by `ForgeFlowMiniMap`.
- **Inheritance:** `ForgeFlowMiniMap` → `Rectangle`.
- **Source:** `ui/flow/controls.slint:45`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeFlowMiniMap } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `nodes` | `in` | `[ForgeFlowNodeData]` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `bounds` | `in` | `ForgeFlowRect` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `viewport` | `in` | `ForgeFlowViewport` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `canvas-width` | `in` | `float` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `canvas-height` | `in` | `float` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `viewport-color` | `in` | `color` | `ForgeFlowTheme.accent` | Constrained by its type and default expression | Host-owned input |
| `viewport-fill` | `in` | `color` | `root.viewport-color.with-alpha(0.04)` | Constrained by its type and default expression | Host-owned input |
| `node-radius` | `in` | `length` | `2px` | Constrained by its type and default expression | Host-owned input |
| `zoom-step` | `in` | `float` | `1.12` | Constrained by its type and default expression | Host-owned input |
| `wheel-enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `wheel-requires-control` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `navigate` | `(float, float)` | Emitted by the component and handled by the host. |
| Callback | `zoom-by` | `(float)` | Emitted by the component and handled by the host. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses its declared colors and inherited Slint styling.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-flow-mini-map.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/flow/forge-flow-mini-map)
