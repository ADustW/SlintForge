---
title: ForgeAdaptiveGrid
description: ForgeAdaptiveGrid public API, behavior, example, and host integration.
---

# ForgeAdaptiveGrid

`ForgeAdaptiveGrid` is a public component in the **Layouts** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-adaptive-grid" name="ForgeAdaptiveGrid" />

## Purpose and usage

- **Use when:** composing a layouts screen that needs the behavior represented by `ForgeAdaptiveGrid`.
- **Inheritance:** `ForgeAdaptiveGrid` → `Rectangle`.
- **Source:** `ui/layouts/adaptive-grid.slint:4`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeAdaptiveGrid } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `min-item-width` | `in` | `length` | `180px` | Constrained by its type and default expression | Host-owned input |
| `column-gap` | `in` | `length` | `12px` | Constrained by its type and default expression | Host-owned input |
| `row-gap` | `in` | `length` | `12px` | Constrained by its type and default expression | Host-owned input |
| `content-padding` | `in` | `length` | `12px` | Constrained by its type and default expression | Host-owned input |
| `max-columns` | `in` | `int` | `0` | Constrained by its type and default expression | Host-owned input |
| `column-count` | `out` | `int` | `Math.max(1, root.max-columns > 0 ? Math.min(root.max-columns, Math.floor((root.available-width + root.column-gap) / (Math.max(1px, root.min-item-width) + root.column-gap))) : Math.floor((root.available-width + root.column-gap) / (Math.max(1px, root.min-item-width) + root.column-gap)))` | Constrained by its type and default expression | Read-only component output |
| `item-width` | `out` | `length` | `Math.max(0px, (root.available-width - (root.column-count - 1) * root.column-gap) / root.column-count)` | Constrained by its type and default expression | Read-only component output |
| `content-height` | `out` | `length` | `scroll.content-height` | Constrained by its type and default expression | Read-only component output |
| `vertical-scrollbar-visible` | `out` | `bool` | `scroll.content-height > scroll.visible-height` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Default slot | `default` | `@children` | Hosts Slint children supplied by the caller. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses its declared colors and inherited Slint styling.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-adaptive-grid.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/layouts/forge-adaptive-grid)
