---
title: ForgeMultiSelectComboBox
description: ForgeMultiSelectComboBox public API, behavior, example, and host integration.
---

# ForgeMultiSelectComboBox

`ForgeMultiSelectComboBox` is a public component in the **Forms** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-multi-select-combo-box" name="ForgeMultiSelectComboBox" />

## Purpose and usage

- **Use when:** composing a forms screen that needs the behavior represented by `ForgeMultiSelectComboBox`.
- **Inheritance:** `ForgeMultiSelectComboBox` → `Rectangle`.
- **Source:** `ui/form/multi-select.slint:7`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeMultiSelectComboBox } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `items` | `in-out` | `[ForgeChoiceItem]` | `Type default` | Constrained by its type and default expression | Shared by host and component |
| `placeholder-text` | `in` | `string` | `@tr("选择多个项目")` | Constrained by its type and default expression | Host-owned input |
| `selection-text` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `open` | `out` | `bool` | `popup.is-open` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
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

<<< ../../../../../examples/api-components/forge-multi-select-combo-box.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/form/forge-multi-select-combo-box)
