---
title: ForgeTour
description: ForgeTour public API, behavior, example, and host integration.
---

# ForgeTour

`ForgeTour` is a public component in the **Other** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-tour" name="ForgeTour" />

## Purpose and usage

- **Use when:** composing a other screen that needs the behavior represented by `ForgeTour`.
- **Inheritance:** `ForgeTour` → `Rectangle`.
- **Source:** `ui/other/tour.slint:5`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeTour } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `steps` | `in` | `[ForgeTourStep]` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `active` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `current-step` | `in-out` | `int` | `0` | Constrained by its type and default expression | Shared by host and component |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `finished` | `()` | Emitted by the component and handled by the host. |
| Callback | `dismissed` | `()` | Emitted by the component and handled by the host. |
| Callback | `step-changed` | `(int)` | Emitted by the component and handled by the host. |
| Public function | `start` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `next` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `previous` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `dismiss` | `()` | Called by the host and handled synchronously by the component. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-tour.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/other/forge-tour)
