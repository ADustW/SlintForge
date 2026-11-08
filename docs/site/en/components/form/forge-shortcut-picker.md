---
title: ForgeShortcutPicker
description: ForgeShortcutPicker public API, behavior, example, and host integration.
---

# ForgeShortcutPicker

`ForgeShortcutPicker` is a public component in the **Forms** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-shortcut-picker" name="ForgeShortcutPicker" />

## Purpose and usage

- **Use when:** composing a forms screen that needs the behavior represented by `ForgeShortcutPicker`.
- **Inheritance:** `ForgeShortcutPicker` → `Rectangle`.
- **Source:** `ui/form/shortcut-picker.slint:5`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeShortcutPicker } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `language` | `in` | `string` | `"zh"` | Constrained by its type and default expression | Host-owned input |
| `dark` | `in` | `bool` | `Palette.color-scheme == ColorScheme.dark` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `shortcut` | `in-out` | `string` | `Type default` | Constrained by its type and default expression | Shared by host and component |
| `recording` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `is-open` | `out` | `bool` | `popup.is-open` | Constrained by its type and default expression | Read-only component output |
| `draft-shortcut` | `out` | `string` | `root.draft` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `shortcut-changed` | `(string)` | Emitted by the component and handled by the host. |
| Callback | `accepted` | `(string)` | Emitted by the component and handled by the host. |
| Callback | `canceled` | `()` | Emitted by the component and handled by the host. |
| Public function | `start-recording` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `open` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `close` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `reset` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `confirm` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `cancel` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `clear` | `()` | Called by the host and handled synchronously by the component. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-shortcut-picker.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/form/forge-shortcut-picker)
