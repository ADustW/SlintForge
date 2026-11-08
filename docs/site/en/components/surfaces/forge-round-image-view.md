---
title: ForgeRoundImageView
description: ForgeRoundImageView public API, behavior, example, and host integration.
---

# ForgeRoundImageView

`ForgeRoundImageView` is a public component in the **Surfaces** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-round-image-view" name="ForgeRoundImageView" />

## Purpose and usage

- **Use when:** composing a surfaces screen that needs the behavior represented by `ForgeRoundImageView`.
- **Inheritance:** `ForgeRoundImageView` → `Rectangle`.
- **Source:** `ui/surfaces/round-image-view.slint:3`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeRoundImageView } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `source` | `in` | `image` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `corner-radius` | `in` | `length` | `12px` | Constrained by its type and default expression | Host-owned input |
| `image-fit` | `in` | `ImageFit` | `cover` | Constrained by its type and default expression | Host-owned input |

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

<<< ../../../../../examples/api-components/forge-round-image-view.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/surfaces/forge-round-image-view)
