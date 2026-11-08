---
title: ForgeSplitLayout
description: ForgeSplitLayout public API, behavior, example, and host integration.
---

# ForgeSplitLayout

`ForgeSplitLayout` is a public component in the **Layouts** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-split-layout" name="ForgeSplitLayout" />

## Purpose and usage

- **Use when:** composing a layouts screen that needs the behavior represented by `ForgeSplitLayout`.
- **Inheritance:** `ForgeSplitLayout` → `Rectangle`.
- **Source:** `ui/layouts/split-layout.slint:6`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeSplitLayout } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `ratio` | `in-out` | `float` | `0.5` | Constrained by its type and default expression | Shared by host and component |
| `orientation` | `in` | `ForgeSplitOrientation` | `ForgeSplitOrientation.horizontal` | Constrained by its type and default expression | Host-owned input |
| `divider-size` | `in` | `length` | `8px` | Constrained by its type and default expression | Host-owned input |
| `first-width` | `out` | `length` | `root.orientation == ForgeSplitOrientation.horizontal ? Math.max(0px, Math.min(root.width - root.effective-divider, root.width * root.effective-ratio - root.effective-divider / 2)) : root.width` | Constrained by its type and default expression | Read-only component output |
| `first-height` | `out` | `length` | `root.orientation == ForgeSplitOrientation.vertical ? Math.max(0px, Math.min(root.height - root.effective-divider, root.height * root.effective-ratio - root.effective-divider / 2)) : root.height` | Constrained by its type and default expression | Read-only component output |
| `second-x` | `out` | `length` | `root.orientation == ForgeSplitOrientation.horizontal ? root.first-width + root.effective-divider : 0px` | Constrained by its type and default expression | Read-only component output |
| `second-y` | `out` | `length` | `root.orientation == ForgeSplitOrientation.vertical ? root.first-height + root.effective-divider : 0px` | Constrained by its type and default expression | Read-only component output |
| `second-width` | `out` | `length` | `root.orientation == ForgeSplitOrientation.horizontal ? Math.max(0px, root.width - root.second-x) : root.width` | Constrained by its type and default expression | Read-only component output |
| `second-height` | `out` | `length` | `root.orientation == ForgeSplitOrientation.vertical ? Math.max(0px, root.height - root.second-y) : root.height` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `ratio-changed` | `(float)` | Emitted by the component and handled by the host. |
| Default slot | `default` | `@children` | Hosts Slint children supplied by the caller. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-split-layout.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/layouts/forge-split-layout)
