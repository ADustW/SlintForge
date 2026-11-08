---
title: ForgeDrawer
description: ForgeDrawer public API, behavior, example, and host integration.
---

# ForgeDrawer

`ForgeDrawer` is a public component in the **Popups** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-drawer" name="ForgeDrawer" />

## Purpose and usage

- **Use when:** composing a popups screen that needs the behavior represented by `ForgeDrawer`.
- **Inheritance:** `ForgeDrawer` → `Rectangle`.
- **Source:** `ui/popups/drawer.slint:6`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeDrawer } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `open` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `title` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `panel-width` | `in` | `length` | `360px` | Constrained by its type and default expression | Host-owned input |
| `edge` | `in` | `ForgeDrawerEdge` | `right` | Constrained by its type and default expression | Host-owned input |
| `dismiss-on-outside` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `dismissed` | `()` | Emitted by the component and handled by the host. |
| Public function | `dismiss` | `()` | Called by the host and handled synchronously by the component. |
| Default slot | `default` | `@children` | Hosts Slint children supplied by the caller. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-drawer.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/popups/forge-drawer)
