---
title: ForgeShimmer
description: ForgeShimmer public API, behavior, example, and host integration.
---

# ForgeShimmer

`ForgeShimmer` is a public component in the **Feedback** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-shimmer" name="ForgeShimmer" />

## Purpose and usage

- **Use when:** composing a feedback screen that needs the behavior represented by `ForgeShimmer`.
- **Inheritance:** `ForgeShimmer` → `Rectangle`.
- **Source:** `ui/feedback/shimmer.slint:3`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeShimmer } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `running` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `cycle` | `in` | `duration` | `1600ms` | Constrained by its type and default expression | Host-owned input |
| `phase` | `out` | `float` | `root.cycle <= 0ms ? 0 : mod(root.elapsed, root.cycle) / root.cycle` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| — | — | — | No direct public callbacks, functions, or slots. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-shimmer.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/feedback/forge-shimmer)
