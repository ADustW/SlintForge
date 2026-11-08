---
title: ForgeMarquee
description: ForgeMarquee public API, behavior, example, and host integration.
---

# ForgeMarquee

`ForgeMarquee` is a public component in the **Surfaces** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-marquee" name="ForgeMarquee" />

## Purpose and usage

- **Use when:** composing a surfaces screen that needs the behavior represented by `ForgeMarquee`.
- **Inheritance:** `ForgeMarquee` → `Rectangle`.
- **Source:** `ui/surfaces/marquee.slint:4`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeMarquee } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `text` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `cycle-duration` | `in` | `duration` | `8s` | Constrained by its type and default expression | Host-owned input |
| `running` | `in-out` | `bool` | `true` | Constrained by its type and default expression | Shared by host and component |
| `text-width` | `out` | `length` | `caption.preferred-width` | Constrained by its type and default expression | Read-only component output |
| `overflowing` | `out` | `bool` | `root.text-width > Math.max(0px, root.width - 24px)` | Constrained by its type and default expression | Read-only component output |
| `text-position` | `out` | `length` | `root.overflowing && root.cycle-duration > 0s ? 12px - (root.text-width + 24px) * Math.mod(root.elapsed, root.cycle-duration) / root.cycle-duration : 12px` | Constrained by its type and default expression | Read-only component output |

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

<<< ../../../../../examples/api-components/forge-marquee.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/surfaces/forge-marquee)
