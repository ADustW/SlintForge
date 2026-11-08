---
title: ForgeFlowControls
description: ForgeFlowControls public API, behavior, example, and host integration.
---

# ForgeFlowControls

`ForgeFlowControls` is a public component in the **ForgeFlow** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-flow-controls" name="ForgeFlowControls" />

## Purpose and usage

- **Use when:** composing a forgeflow screen that needs the behavior represented by `ForgeFlowControls`.
- **Inheritance:** `ForgeFlowControls` → `Rectangle`.
- **Source:** `ui/flow/controls.slint:4`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeFlowControls } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `zoom` | `in` | `float` | `1` | Constrained by its type and default expression | Host-owned input |
| `min-zoom` | `in` | `float` | `0.1` | Constrained by its type and default expression | Host-owned input |
| `max-zoom` | `in` | `float` | `4` | Constrained by its type and default expression | Host-owned input |
| `can-undo` | `in` | `bool` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `can-redo` | `in` | `bool` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `read-only` | `in` | `bool` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `chinese` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `command` | `(string)` | Emitted by the component and handled by the host. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses its declared colors and inherited Slint styling.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-flow-controls.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/flow/forge-flow-controls)
