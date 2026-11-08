---
title: ForgeTagInputBox
description: ForgeTagInputBox public API, behavior, example, and host integration.
---

# ForgeTagInputBox

`ForgeTagInputBox` is a public component in the **Inputs** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-tag-input-box" name="ForgeTagInputBox" />

## Purpose and usage

- **Use when:** composing a inputs screen that needs the behavior represented by `ForgeTagInputBox`.
- **Inheritance:** `ForgeTagInputBox` → `Rectangle`.
- **Source:** `ui/inputs/tags.slint:20`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeTagInputBox } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `tags` | `in` | `[string]` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `maximum-tags` | `in` | `int` | `20` | Constrained by its type and default expression | Host-owned input |
| `draft` | `in-out` | `string` | `Type default` | Constrained by its type and default expression | Shared by host and component |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `add-requested` | `(string)` | Emitted by the component and handled by the host. |
| Callback | `remove-requested` | `(int)` | Emitted by the component and handled by the host. |
| Public function | `submit` | `()` | Called by the host and handled synchronously by the component. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses its declared colors and inherited Slint styling.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-tag-input-box.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/inputs/forge-tag-input-box)
