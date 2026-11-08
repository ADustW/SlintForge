---
title: ForgeContentDialog
description: ForgeContentDialog public API, behavior, example, and host integration.
---

# ForgeContentDialog

`ForgeContentDialog` is a public component in the **Popups** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-content-dialog" name="ForgeContentDialog" />

## Purpose and usage

- **Use when:** composing a popups screen that needs the behavior represented by `ForgeContentDialog`.
- **Inheritance:** `ForgeContentDialog` → `Rectangle`.
- **Source:** `ui/popups/content-dialog.slint:6`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeContentDialog } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `open` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `title` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `message` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `primary-text` | `in` | `string` | `@tr("确定")` | Constrained by its type and default expression | Host-owned input |
| `cancel-text` | `in` | `string` | `@tr("取消")` | Constrained by its type and default expression | Host-owned input |
| `primary-enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `dismiss-on-outside` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `primary-has-focus` | `out` | `bool` | `accept-button.has-focus` | Constrained by its type and default expression | Read-only component output |
| `cancel-has-focus` | `out` | `bool` | `cancel-button.has-focus` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `accepted` | `()` | Emitted by the component and handled by the host. |
| Callback | `canceled` | `()` | Emitted by the component and handled by the host. |
| Callback | `initial-focus-requested` | `() -> bool` | Emitted by the component and handled by the host. |
| Callback | `focus-next-requested` | `(reverse: bool) -> bool` | Emitted by the component and handled by the host. |
| Callback | `focus-return-requested` | `()` | Emitted by the component and handled by the host. |
| Public function | `focus-primary-button` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `focus-cancel-button` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `dismiss` | `()` | Called by the host and handled synchronously by the component. |
| Default slot | `default` | `@children` | Hosts Slint children supplied by the caller. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-content-dialog.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/popups/forge-content-dialog)
