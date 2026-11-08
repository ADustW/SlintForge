---
title: ForgeDial
description: ForgeDial public API, behavior, example, and host integration.
---

# ForgeDial

`ForgeDial` is a public component in the **Other** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-dial" name="ForgeDial" />

## Purpose and usage

- **Use when:** composing a other screen that needs the behavior represented by `ForgeDial`.
- **Inheritance:** `ForgeDial` → `Rectangle`.
- **Source:** `ui/other/dial.slint:4`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeDial } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `value` | `in-out` | `float` | `0` | Constrained by its type and default expression | Shared by host and component |
| `minimum` | `in` | `float` | `0` | Constrained by its type and default expression | Host-owned input |
| `maximum` | `in` | `float` | `100` | Constrained by its type and default expression | Host-owned input |
| `step` | `in` | `float` | `1` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `suffix` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `needle-angle` | `out` | `angle` | `(-135 + (root.maximum <= root.minimum ? 0 : (root.value - root.minimum) / (root.maximum - root.minimum)) * 270) * 1deg` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `value-changed` | `(float)` | Emitted by the component and handled by the host. |
| Public function | `set-value` | `(candidate: float)` | Called by the host and handled synchronously by the component. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-dial.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/other/forge-dial)
