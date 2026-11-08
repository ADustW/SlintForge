---
title: ForgeIconButton
description: ForgeIconButton public API, behavior, example, and host integration.
---

# ForgeIconButton

`ForgeIconButton` is a public component in the **Basic** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-icon-button" name="ForgeIconButton" />

## Purpose and usage

- **Use when:** composing a basic screen that needs the behavior represented by `ForgeIconButton`.
- **Inheritance:** `ForgeIconButton` → `Rectangle`.
- **Source:** `ui/basic/icon-button.slint:4`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeIconButton } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `icon` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `label` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `selected` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `has-focus` | `out` | `bool` | `focus.has-focus` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `clicked` | `()` | Emitted by the component and handled by the host. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-icon-button.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/basic/forge-icon-button)
