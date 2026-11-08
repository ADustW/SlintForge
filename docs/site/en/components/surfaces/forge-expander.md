---
title: ForgeExpander
description: ForgeExpander public API, behavior, example, and host integration.
---

# ForgeExpander

`ForgeExpander` is a public component in the **Surfaces** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-expander" name="ForgeExpander" />

## Purpose and usage

- **Use when:** composing a surfaces screen that needs the behavior represented by `ForgeExpander`.
- **Inheritance:** `ForgeExpander` → `Rectangle`.
- **Source:** `ui/surfaces/expander.slint:5`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeExpander } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `title` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `expanded` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `content-height` | `in` | `length` | `0px` | Constrained by its type and default expression | Host-owned input |
| `content-padding` | `in` | `length` | `12px` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `natural-content-height` | `out` | `length` | `slot.preferred-height + 2 * Math.max(0px, root.content-padding)` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `toggled` | `(bool)` | Emitted by the component and handled by the host. |
| Public function | `toggle` | `()` | Called by the host and handled synchronously by the component. |
| Default slot | `default` | `@children` | Hosts Slint children supplied by the caller. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-expander.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/surfaces/forge-expander)
