---
title: ForgeSegmentedControl
description: ForgeSegmentedControl public API, behavior, example, and host integration.
---

# ForgeSegmentedControl

`ForgeSegmentedControl` is a public component in the **Navigation** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-segmented-control" name="ForgeSegmentedControl" />

## Purpose and usage

- **Use when:** composing a navigation screen that needs the behavior represented by `ForgeSegmentedControl`.
- **Inheritance:** `ForgeSegmentedControl` → `Rectangle`.
- **Source:** `ui/navigation/segmented-control.slint:4`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeSegmentedControl } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `items` | `in` | `[ForgeChoiceItem]` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `selected-id` | `in-out` | `string` | `Type default` | Constrained by its type and default expression | Shared by host and component |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `selected` | `(string)` | Emitted by the component and handled by the host. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses its declared colors and inherited Slint styling.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-segmented-control.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/navigation/forge-segmented-control)
