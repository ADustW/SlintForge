---
title: ForgeFlowEdgeEditor
description: ForgeFlowEdgeEditor public API, behavior, example, and host integration.
---

# ForgeFlowEdgeEditor

`ForgeFlowEdgeEditor` is a public component in the **ForgeFlow** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-flow-edge-editor" name="ForgeFlowEdgeEditor" />

## Purpose and usage

- **Use when:** composing a forgeflow screen that needs the behavior represented by `ForgeFlowEdgeEditor`.
- **Inheritance:** `ForgeFlowEdgeEditor` → `Rectangle`.
- **Source:** `ui/flow/edge-editor.slint:5`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeFlowEdgeEditor } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `edge` | `in` | `ForgeFlowEdgeData` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `count` | `in` | `int` | `1` | Constrained by its type and default expression | Host-owned input |
| `read-only` | `in` | `bool` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `chinese` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `action` | `(ForgeFlowAction)` | Emitted by the component and handled by the host. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses its declared colors and inherited Slint styling.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-flow-edge-editor.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/flow/forge-flow-edge-editor)
