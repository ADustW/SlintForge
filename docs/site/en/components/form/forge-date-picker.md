---
title: ForgeDatePicker
description: ForgeDatePicker public API, behavior, example, and host integration.
---

# ForgeDatePicker

`ForgeDatePicker` is a public component in the **Forms** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-date-picker" name="ForgeDatePicker" />

## Purpose and usage

- **Use when:** composing a forms screen that needs the behavior represented by `ForgeDatePicker`.
- **Inheritance:** `ForgeDatePicker` → `Rectangle`.
- **Source:** `ui/form/date-picker.slint:213`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeDatePicker } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `language` | `in` | `string` | `"zh"` | Constrained by its type and default expression | Host-owned input |
| `dark` | `in` | `bool` | `Palette.color-scheme == ColorScheme.dark` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-year` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-month` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-day` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `minimum-date` | `in` | `Date` | `{year: 1924, month: 1, day: 1}` | Constrained by its type and default expression | Host-owned input |
| `maximum-date` | `in` | `Date` | `{year: 2124, month: 12, day: 31}` | Constrained by its type and default expression | Host-owned input |
| `date` | `in-out` | `Date` | `{year: 2026, month: 10, day: 12}` | Constrained by its type and default expression | Shared by host and component |
| `has-value` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `is-open` | `out` | `bool` | `popup.is-open` | Constrained by its type and default expression | Read-only component output |
| `current-date` | `out` | `Date` | `root.draft` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `accepted` | `(Date)` | Emitted by the component and handled by the host. |
| Callback | `canceled` | `()` | Emitted by the component and handled by the host. |
| Public function | `open` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `close` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `set-year` | `(value: int)` | Called by the host and handled synchronously by the component. |
| Public function | `set-month` | `(value: int)` | Called by the host and handled synchronously by the component. |
| Public function | `set-day` | `(value: int)` | Called by the host and handled synchronously by the component. |
| Public function | `confirm` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `cancel` | `()` | Called by the host and handled synchronously by the component. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-date-picker.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/form/forge-date-picker)
