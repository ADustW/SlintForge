---
title: ForgeRatingControl
description: ForgeRatingControl public API, behavior, example, and host integration.
---

# ForgeRatingControl

`ForgeRatingControl` is a public component in the **Feedback** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-rating-control" name="ForgeRatingControl" />

## Purpose and usage

- **Use when:** composing a feedback screen that needs the behavior represented by `ForgeRatingControl`.
- **Inheritance:** `ForgeRatingControl` → `Rectangle`.
- **Source:** `ui/feedback/rating.slint:3`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeRatingControl } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `value` | `in-out` | `int` | `0` | Constrained by its type and default expression | Shared by host and component |
| `maximum` | `in` | `int` | `5` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `read-only` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `value-changed` | `(int)` | Emitted by the component and handled by the host. |
| Public function | `set-value` | `(candidate: int)` | Called by the host and handled synchronously by the component. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses its declared colors and inherited Slint styling.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-rating-control.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/feedback/forge-rating-control)
