---
title: ForgeStaggeredGridView
description: ForgeStaggeredGridView public API, behavior, example, and host integration.
---

# ForgeStaggeredGridView

`ForgeStaggeredGridView` is a public component in the **Layouts** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-staggered-grid-view" name="ForgeStaggeredGridView" />

## Purpose and usage

- **Use when:** composing a layouts screen that needs the behavior represented by `ForgeStaggeredGridView`.
- **Inheritance:** `ForgeStaggeredGridView` → `Rectangle`.
- **Source:** `ui/layouts/staggered-grid-view.slint:43`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeStaggeredGridView } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `items` | `in` | `[ForgeStaggeredItem]` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `min-column-width` | `in` | `length` | `220px` | Constrained by its type and default expression | Host-owned input |
| `max-columns` | `in` | `int` | `4` | Constrained by its type and default expression | Host-owned input |
| `gap` | `in` | `length` | `12px` | Constrained by its type and default expression | Host-owned input |
| `content-padding` | `in` | `length` | `12px` | Constrained by its type and default expression | Host-owned input |
| `content-height` | `in` | `length` | `0px` | Constrained by its type and default expression | Host-owned input |
| `column-count` | `out` | `int` | `Math.max(1, Math.min(Math.max(1, root.max-columns), Math.floor((root.available-width + root.gap) / (Math.max(1px, root.min-column-width) + root.gap))))` | Constrained by its type and default expression | Read-only component output |
| `column-width` | `out` | `length` | `Math.max(0px, (root.available-width - (root.column-count - 1) * root.gap) / root.column-count)` | Constrained by its type and default expression | Read-only component output |
| `vertical-scrollbar-visible` | `out` | `bool` | `scroll.content-height > scroll.visible-height` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `layout-requested` | `(int)` | Emitted by the component and handled by the host. |
| Callback | `selected` | `(string)` | Emitted by the component and handled by the host. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-staggered-grid-view.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/layouts/forge-staggered-grid-view)
