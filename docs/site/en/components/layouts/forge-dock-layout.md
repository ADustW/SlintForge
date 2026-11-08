---
title: ForgeDockLayout
description: ForgeDockLayout public API, behavior, example, and host integration.
---

# ForgeDockLayout

`ForgeDockLayout` is a public component in the **Layouts** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-dock-layout" name="ForgeDockLayout" />

## Purpose and usage

- **Use when:** composing a layouts screen that needs the behavior represented by `ForgeDockLayout`.
- **Inheritance:** `ForgeDockLayout` → `Rectangle`.
- **Source:** `ui/layouts/dock-layout.slint:7`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeDockLayout } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `top-height` | `in-out` | `length` | `96px` | Constrained by its type and default expression | Shared by host and component |
| `bottom-height` | `in-out` | `length` | `96px` | Constrained by its type and default expression | Shared by host and component |
| `left-width` | `in-out` | `length` | `180px` | Constrained by its type and default expression | Shared by host and component |
| `right-width` | `in-out` | `length` | `180px` | Constrained by its type and default expression | Shared by host and component |
| `show-top` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-bottom` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-left` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-right` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `divider-size` | `in` | `length` | `8px` | Constrained by its type and default expression | Host-owned input |
| `min-center-width` | `in` | `length` | `160px` | Constrained by its type and default expression | Host-owned input |
| `min-center-height` | `in` | `length` | `120px` | Constrained by its type and default expression | Host-owned input |
| `top-rect` | `out` | `ForgeDockRect` | `{ x: 0px, y: 0px, width: root.width, height: root.actual-top }` | Constrained by its type and default expression | Read-only component output |
| `bottom-rect` | `out` | `ForgeDockRect` | `{ x: 0px, y: root.height - root.actual-bottom, width: root.width, height: root.actual-bottom }` | Constrained by its type and default expression | Read-only component output |
| `left-rect` | `out` | `ForgeDockRect` | `{ x: 0px, y: root.middle-y, width: root.actual-left, height: root.middle-height }` | Constrained by its type and default expression | Read-only component output |
| `right-rect` | `out` | `ForgeDockRect` | `{ x: root.width - root.actual-right, y: root.middle-y, width: root.actual-right, height: root.middle-height }` | Constrained by its type and default expression | Read-only component output |
| `center-rect` | `out` | `ForgeDockRect` | `{ x: root.center-x, y: root.middle-y, width: root.center-width, height: root.middle-height }` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `pane-size-changed` | `(ForgeDockEdge, length)` | Emitted by the component and handled by the host. |
| Default slot | `default` | `@children` | Hosts Slint children supplied by the caller. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-dock-layout.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/layouts/forge-dock-layout)
