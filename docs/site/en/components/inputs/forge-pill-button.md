---
title: ForgePillButton
description: ForgePillButton public API, behavior, example, and host integration.
---

# ForgePillButton

`ForgePillButton` is a public component in the **Inputs** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-pill-button" name="ForgePillButton" />

## Purpose and usage

- **Use when:** composing a inputs screen that needs the behavior represented by `ForgePillButton`.
- **Inheritance:** `ForgePillButton` → `Rectangle`.
- **Source:** `ui/inputs/action-buttons.slint:6`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgePillButton } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `text` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `primary` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `pressed` | `out` | `bool` | `touch.pressed \|\| root.key-held` | Constrained by its type and default expression | Read-only component output |
| `has-focus` | `out` | `bool` | `focus.has-focus` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `clicked` | `()` | Emitted by the component and handled by the host. |
| Public function | `focus-button` | `()` | Called by the host and handled synchronously by the component. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-pill-button.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/inputs/forge-pill-button)
