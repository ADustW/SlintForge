---
title: ForgeNumberBox
description: ForgeNumberBox public API, behavior, example, and host integration.
---

# ForgeNumberBox

`ForgeNumberBox` is a public component in the **Forms** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-number-box" name="ForgeNumberBox" />

## Purpose and usage

- **Use when:** composing a forms screen that needs the behavior represented by `ForgeNumberBox`.
- **Inheritance:** `ForgeNumberBox` → `Rectangle`.
- **Source:** `ui/form/number-box.slint:3`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeNumberBox } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `value` | `in-out` | `float` | `0` | Constrained by its type and default expression | Shared by host and component |
| `minimum` | `in` | `float` | `0` | Constrained by its type and default expression | Host-owned input |
| `maximum` | `in` | `float` | `100` | Constrained by its type and default expression | Host-owned input |
| `step` | `in` | `float` | `0.1` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `input-valid` | `out` | `bool` | `field.text.is-float()` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `value-changed` | `(float)` | Emitted by the component and handled by the host. |
| Public function | `commit` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `set-value` | `(candidate: float)` | Called by the host and handled synchronously by the component. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses its declared colors and inherited Slint styling.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-number-box.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/form/forge-number-box)
