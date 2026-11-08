---
title: ForgeNavigationView
description: ForgeNavigationView public API, behavior, example, and host integration.
---

# ForgeNavigationView

`ForgeNavigationView` is a public component in the **Navigation** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-navigation-view" name="ForgeNavigationView" />

## Purpose and usage

- **Use when:** composing a navigation screen that needs the behavior represented by `ForgeNavigationView`.
- **Inheritance:** `ForgeNavigationView` → `Rectangle`.
- **Source:** `ui/navigation/navigation-view.slint:9`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeNavigationView } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `items` | `in` | `[ForgeNavigationItem]` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `tree-items` | `in` | `[ForgeNavigationItem]` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `footer-items` | `in` | `[ForgeNavigationItem]` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `active-route-id` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `active-top-id` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `placement` | `in` | `ForgeNavPlacement` | `ForgeNavPlacement.left` | Constrained by its type and default expression | Host-owned input |
| `pane-state` | `in` | `ForgePaneState` | `ForgePaneState.expanded` | Constrained by its type and default expression | Host-owned input |
| `brand` | `in` | `string` | `"SlintForge Gallery"` | Constrained by its type and default expression | Host-owned input |
| `search-query` | `in` | `string` | `""` | Constrained by its type and default expression | Host-owned input |
| `can-go-back` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `dark-mode` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `show-app-bar` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-pane-header` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-pane-search` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `content-width` | `out` | `length` | `content.width` | Constrained by its type and default expression | Read-only component output |
| `content-height` | `out` | `length` | `content.height` | Constrained by its type and default expression | Read-only component output |
| `content-x` | `out` | `length` | `content.x` | Constrained by its type and default expression | Read-only component output |
| `content-y` | `out` | `length` | `body.y` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `navigate-requested` | `(string, [ForgeRouteParam])` | Emitted by the component and handled by the host. |
| Callback | `group-toggle-requested` | `(string)` | Emitted by the component and handled by the host. |
| Callback | `pane-state-requested` | `(ForgePaneState)` | Emitted by the component and handled by the host. |
| Callback | `search-requested` | `(string)` | Emitted by the component and handled by the host. |
| Callback | `back-requested` | `()` | Emitted by the component and handled by the host. |
| Callback | `theme-toggle-requested` | `()` | Emitted by the component and handled by the host. |
| Default slot | `default` | `@children` | Hosts Slint children supplied by the caller. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-navigation-view.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/navigation/forge-navigation-view)
