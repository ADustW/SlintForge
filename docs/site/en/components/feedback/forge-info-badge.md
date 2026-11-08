---
title: ForgeInfoBadge
description: ForgeInfoBadge public API, behavior, example, and host integration.
---

# ForgeInfoBadge

`ForgeInfoBadge` is a public component in the **Feedback** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-info-badge" name="ForgeInfoBadge" />

## Purpose and usage

- **Use when:** composing a feedback screen that needs the behavior represented by `ForgeInfoBadge`.
- **Inheritance:** `ForgeInfoBadge` → `Rectangle`.
- **Source:** `ui/feedback/info-badge.slint:4`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeInfoBadge } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `value` | `in` | `int` | `0` | Constrained by its type and default expression | Host-owned input |
| `maximum` | `in` | `int` | `99` | Constrained by its type and default expression | Host-owned input |
| `dot` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `show-zero` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `display-text` | `out` | `string` | `root.value > Math.max(0, root.maximum) ? Math.max(0, root.maximum) + "+" : "\{Math.max(0, root.value)}"` | Constrained by its type and default expression | Read-only component output |

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

<<< ../../../../../examples/api-components/forge-info-badge.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/feedback/forge-info-badge)
