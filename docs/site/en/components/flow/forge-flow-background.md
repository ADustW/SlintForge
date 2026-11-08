---
title: ForgeFlowBackground
description: ForgeFlowBackground public API, behavior, example, and host integration.
---

# ForgeFlowBackground

`ForgeFlowBackground` is a public component in the **ForgeFlow** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-flow-background" name="ForgeFlowBackground" />

## Purpose and usage

- **Use when:** composing a forgeflow screen that needs the behavior represented by `ForgeFlowBackground`.
- **Inheritance:** `ForgeFlowBackground` → `Rectangle`.
- **Source:** `ui/flow/background.slint:4`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeFlowBackground } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `viewport` | `in` | `ForgeFlowViewport` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `variant` | `in` | `int` | `0` | Constrained by its type and default expression | Host-owned input |
| `gap` | `in` | `float` | `20` | Constrained by its type and default expression | Host-owned input |
| `pattern-color` | `in` | `color` | `ForgeFlowTheme.grid` | Constrained by its type and default expression | Host-owned input |
| `canvas-color` | `in` | `color` | `ForgeFlowTheme.canvas` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| — | — | — | No direct public callbacks, functions, or slots. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses its declared colors and inherited Slint styling.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-flow-background.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/flow/forge-flow-background)
