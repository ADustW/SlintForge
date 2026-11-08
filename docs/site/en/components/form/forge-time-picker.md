---
title: ForgeTimePicker
description: ForgeTimePicker public API, behavior, example, and host integration.
---

# ForgeTimePicker

`ForgeTimePicker` is a public component in the **Forms** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-time-picker" name="ForgeTimePicker" />

## Purpose and usage

- **Use when:** composing a forms screen that needs the behavior represented by `ForgeTimePicker`.
- **Inheritance:** `ForgeTimePicker` → `Rectangle`.
- **Source:** `ui/form/time-picker.slint:104`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeTimePicker } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `language` | `in` | `string` | `"zh"` | Constrained by its type and default expression | Host-owned input |
| `dark` | `in` | `bool` | `Palette.color-scheme == ColorScheme.dark` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `use-24-hour-format` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `clock-mode` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `minute-increment` | `in` | `int` | `1` | Constrained by its type and default expression | Host-owned input |
| `show-seconds` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `time` | `in-out` | `Time` | `{hour: 12, minute: 0, second: 0}` | Constrained by its type and default expression | Shared by host and component |
| `has-value` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `is-open` | `out` | `bool` | `popup.is-open` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `accepted` | `(Time)` | Emitted by the component and handled by the host. |
| Callback | `canceled` | `()` | Emitted by the component and handled by the host. |
| Public function | `open` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `close` | `()` | Called by the host and handled synchronously by the component. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-time-picker.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/form/forge-time-picker)
