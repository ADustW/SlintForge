---
title: ForgeFlowNode
description: ForgeFlowNode public API, behavior, example, and host integration.
---

# ForgeFlowNode

`ForgeFlowNode` is a public component in the **ForgeFlow** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-flow-node" name="ForgeFlowNode" />

## Purpose and usage

- **Use when:** composing a forgeflow screen that needs the behavior represented by `ForgeFlowNode`.
- **Inheritance:** `ForgeFlowNode` → `Rectangle`.
- **Source:** `ui/flow/node.slint:45`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeFlowNode } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `node` | `in` | `ForgeFlowNodeData` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `read-only` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `tree` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `chinese` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `snap` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `pan-mode` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `resizable` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `reparent-on-drop` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `minimum-resize-width` | `in` | `float` | `160` | Constrained by its type and default expression | Host-owned input |
| `minimum-resize-height` | `in` | `float` | `112` | Constrained by its type and default expression | Host-owned input |
| `drag-area` | `in` | `ForgeFlowRect` | `{x: 0, y: 0, width: root.node.width, height: root.node.header-visible ? root.node.header-height : 0}` | Constrained by its type and default expression | Host-owned input |
| `auto-size` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `content-width` | `in` | `float` | `0` | Constrained by its type and default expression | Host-owned input |
| `content-height` | `in` | `float` | `0` | Constrained by its type and default expression | Host-owned input |
| `zoom` | `in` | `float` | `1` | Constrained by its type and default expression | Host-owned input |
| `candidate-port` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `accent` | `in` | `color` | `root.node.kind == "form" ? #7c5ce7 : root.node.kind == "device" ? #11998a : root.node.kind == "image" ? #d48727 : ForgeFlowTheme.accent` | Constrained by its type and default expression | Host-owned input |
| `card-background` | `in` | `color` | `root.node.is-group ? ForgeFlowTheme.raised.with-alpha(0.5) : ForgeFlowTheme.surface` | Constrained by its type and default expression | Host-owned input |
| `corner-radius` | `in` | `length` | `ForgeFlowTheme.radius` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `action` | `(ForgeFlowAction)` | Emitted by the component and handled by the host. |
| Public function | `resize-to-content` | `()` | Called by the host and handled synchronously by the component. |
| Default slot | `default` | `@children` | Hosts Slint children supplied by the caller. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses its declared colors and inherited Slint styling.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-flow-node.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/flow/forge-flow-node)
