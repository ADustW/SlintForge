---
title: ForgeDatePickerPopup
description: ForgeDatePickerPopup public API, behavior, example, and host integration.
---

# ForgeDatePickerPopup

`ForgeDatePickerPopup` is a public component in the **Forms** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-date-picker-popup" name="ForgeDatePickerPopup" />

## Purpose and usage

- **Use when:** composing a forms screen that needs the behavior represented by `ForgeDatePickerPopup`.
- **Inheritance:** `ForgeDatePickerPopup` → `Rectangle`.
- **Source:** `ui/form/date-picker.slint:8`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeDatePickerPopup } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `language` | `in` | `string` | `"zh"` | Constrained by its type and default expression | Host-owned input |
| `title` | `in` | `string` | `root.language == "en" ? "Select date" : "选择日期"` | Constrained by its type and default expression | Host-owned input |
| `date` | `in` | `Date` | `{year: 2026, month: 10, day: 12}` | Constrained by its type and default expression | Host-owned input |
| `minimum-date` | `in` | `Date` | `{year: 1, month: 1, day: 1}` | Constrained by its type and default expression | Host-owned input |
| `maximum-date` | `in` | `Date` | `{year: 9999, month: 12, day: 31}` | Constrained by its type and default expression | Host-owned input |
| `dark` | `in` | `bool` | `Palette.color-scheme == ColorScheme.dark` | Constrained by its type and default expression | Host-owned input |
| `show-time` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `confirm-on-selection` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `time` | `in` | `Time` | `{hour: 9, minute: 0, second: 0}` | Constrained by its type and default expression | Host-owned input |
| `minute-increment` | `in` | `int` | `1` | Constrained by its type and default expression | Host-owned input |
| `is-open` | `out` | `bool` | `popup.is-open` | Constrained by its type and default expression | Read-only component output |
| `current-date` | `out` | `Date` | `root.draft` | Constrained by its type and default expression | Read-only component output |
| `current-time` | `out` | `Time` | `root.draft-time` | Constrained by its type and default expression | Read-only component output |
| `date-text` | `out` | `string` | `ForgeDateTime.format-date(root.draft, root.language)` | Constrained by its type and default expression | Read-only component output |
| `month-text` | `out` | `string` | `ForgeDateTime.format-month(root.display-year, root.display-month, root.language)` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `accepted` | `(Date)` | Emitted by the component and handled by the host. |
| Callback | `time-accepted` | `(Time)` | Emitted by the component and handled by the host. |
| Callback | `canceled` | `()` | Emitted by the component and handled by the host. |
| Public function | `open` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `close` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `set-time` | `(value: Time)` | Called by the host and handled synchronously by the component. |
| Public function | `choose-day` | `(day: int)` | Called by the host and handled synchronously by the component. |
| Public function | `browse-month` | `(delta: int)` | Called by the host and handled synchronously by the component. |
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

<<< ../../../../../examples/api-components/forge-date-picker-popup.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/form/forge-date-picker-popup)
