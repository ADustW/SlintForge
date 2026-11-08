---
title: ForgeDelayButton
description: ForgeDelayButton public API, behavior, example, and host integration.
---

# ForgeDelayButton

`ForgeDelayButton` is a public component in the **Inputs** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-delay-button" name="ForgeDelayButton" />

## Purpose and usage

- **Use when:** composing a inputs screen that needs the behavior represented by `ForgeDelayButton`.
- **Inheritance:** `ForgeDelayButton` → `Rectangle`.
- **Source:** `ui/inputs/action-buttons.slint:55`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeDelayButton } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `text` | `in` | `string` | `@tr("按住确认")` | Constrained by its type and default expression | Host-owned input |
| `delay` | `in` | `duration` | `1200ms` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `mode` | `in` | `ForgeDelayButtonMode` | `hold-release` | Constrained by its type and default expression | Host-owned input |
| `primary` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `pill` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `show-progress` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `running` | `out` | `bool` | `root.active` | Constrained by its type and default expression | Read-only component output |
| `pressed` | `out` | `bool` | `touch.pressed \|\| root.key-held` | Constrained by its type and default expression | Read-only component output |
| `has-focus` | `out` | `bool` | `focus.has-focus` | Constrained by its type and default expression | Read-only component output |
| `progress` | `out` | `float` | `!root.active ? 0 : root.delay <= 0ms ? 1 : min(1, root.elapsed / root.delay)` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `clicked` | `()` | Emitted by the component and handled by the host. |
| Callback | `canceled` | `()` | Emitted by the component and handled by the host. |
| Public function | `focus-button` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `cancel` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `begin` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `finish` | `()` | Called by the host and handled synchronously by the component. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-delay-button.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/inputs/forge-delay-button)
