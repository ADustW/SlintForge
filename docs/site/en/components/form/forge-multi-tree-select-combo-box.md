---
title: ForgeMultiTreeSelectComboBox
description: ForgeMultiTreeSelectComboBox public API, behavior, example, and host integration.
---

# ForgeMultiTreeSelectComboBox

`ForgeMultiTreeSelectComboBox` is a public component in the **Forms** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-multi-tree-select-combo-box" name="ForgeMultiTreeSelectComboBox" />

## Purpose and usage

- **Use when:** composing a forms screen that needs the behavior represented by `ForgeMultiTreeSelectComboBox`.
- **Inheritance:** `ForgeMultiTreeSelectComboBox` → `Rectangle`.
- **Source:** `ui/form/multi-select.slint:47`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeMultiTreeSelectComboBox } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `items` | `in` | `[ForgeTreeItem]` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `selection-text` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `expansion-changed` | `(string, bool)` | Emitted by the component and handled by the host. |
| Callback | `selection-changed` | `(string, bool)` | Emitted by the component and handled by the host. |
| Public function | `show` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `close` | `()` | Called by the host and handled synchronously by the component. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-multi-tree-select-combo-box.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/form/forge-multi-tree-select-combo-box)
