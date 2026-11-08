---
title: ForgeInputValidation
description: ForgeInputValidation public API, behavior, example, and host integration.
---

# ForgeInputValidation

`ForgeInputValidation` is a public component in the **Inputs** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-input-validation" name="ForgeInputValidation" />

## Purpose and usage

- **Use when:** composing a inputs screen that needs the behavior represented by `ForgeInputValidation`.
- **Inheritance:** `ForgeInputValidation` → `Rectangle`.
- **Source:** `ui/inputs/validation.slint:4`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeInputValidation } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `text` | `in-out` | `string` | `Type default` | Constrained by its type and default expression | Shared by host and component |
| `placeholder-text` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `required` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `minimum-length` | `in` | `int` | `0` | Constrained by its type and default expression | Host-owned input |
| `maximum-length` | `in` | `int` | `0` | Constrained by its type and default expression | Host-owned input |
| `error-message` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `touched` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `valid` | `out` | `bool` | `(!root.required \|\| root.text != "") && root.text.character-count >= Math.max(0, root.minimum-length) && (root.maximum-length <= 0 \|\| root.text.character-count <= root.maximum-length) && root.error-message == ""` | Constrained by its type and default expression | Read-only component output |
| `validation-message` | `out` | `string` | `root.valid ? "" : root.error-message != "" ? root.error-message : root.text == "" ? @tr("此项不能为空") : @tr("输入长度不符合要求")` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `edited` | `(string)` | Emitted by the component and handled by the host. |
| Callback | `accepted` | `(string)` | Emitted by the component and handled by the host. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-input-validation.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/inputs/forge-input-validation)
