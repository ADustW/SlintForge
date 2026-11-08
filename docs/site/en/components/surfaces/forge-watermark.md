---
title: ForgeWatermark
description: ForgeWatermark public API, behavior, example, and host integration.
---

# ForgeWatermark

`ForgeWatermark` is a public component in the **Surfaces** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-watermark" name="ForgeWatermark" />

## Purpose and usage

- **Use when:** composing a surfaces screen that needs the behavior represented by `ForgeWatermark`.
- **Inheritance:** `ForgeWatermark` → `Rectangle`.
- **Source:** `ui/surfaces/watermark.slint:4`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeWatermark } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `mark` | `in` | `string` | `"SlintForge"` | Constrained by its type and default expression | Host-owned input |
| `mark-opacity` | `in` | `float` | `0.12` | Constrained by its type and default expression | Host-owned input |
| `tile-width` | `in` | `length` | `180px` | Constrained by its type and default expression | Host-owned input |
| `tile-height` | `in` | `length` | `120px` | Constrained by its type and default expression | Host-owned input |
| `columns` | `out` | `int` | `Math.ceil(Math.max(0px, root.width) / root.effective-width)` | Constrained by its type and default expression | Read-only component output |
| `rows` | `out` | `int` | `Math.ceil(Math.max(0px, root.height) / root.effective-height)` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Default slot | `default` | `@children` | Hosts Slint children supplied by the caller. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-watermark.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/surfaces/forge-watermark)
