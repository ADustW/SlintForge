---
title: ForgeSkeletonView
description: ForgeSkeletonView public API, behavior, example, and host integration.
---

# ForgeSkeletonView

`ForgeSkeletonView` is a public component in the **Feedback** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-skeleton-view" name="ForgeSkeletonView" />

## Purpose and usage

- **Use when:** composing a feedback screen that needs the behavior represented by `ForgeSkeletonView`.
- **Inheritance:** `ForgeSkeletonView` → `Rectangle`.
- **Source:** `ui/feedback/shimmer.slint:18`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeSkeletonView } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `loading` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `animate` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `rows` | `in` | `int` | `3` | Constrained by its type and default expression | Host-owned input |

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

<<< ../../../../../examples/api-components/forge-skeleton-view.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/feedback/forge-skeleton-view)
