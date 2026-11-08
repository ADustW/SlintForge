---
title: ForgeFlowCanvas
description: ForgeFlowCanvas public API, behavior, example, and host integration.
---

# ForgeFlowCanvas

`ForgeFlowCanvas` is a public component in the **ForgeFlow** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-flow-canvas" name="ForgeFlowCanvas" />

## Purpose and usage

- **Use when:** composing a forgeflow screen that needs the behavior represented by `ForgeFlowCanvas`.
- **Inheritance:** `ForgeFlowCanvas` → `Rectangle`.
- **Source:** `ui/flow/canvas.slint:12`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeFlowCanvas } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `mode` | `in` | `string` | `"graph"` | Constrained by its type and default expression | Host-owned input |
| `demo` | `in` | `int` | `-1` | Constrained by its type and default expression | Host-owned input |
| `read-only` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `grid` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `background-variant` | `in-out` | `int` | `0` | Constrained by its type and default expression | Shared by host and component |
| `pan-on-drag` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `show-node-toolbar` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `connection-target` | `out` | `string` | `root.candidate.op == "" ? "" : (root.pending.target == "reconnect" && root.pending.number == 0 ? root.candidate.id : root.candidate.target)` | Constrained by its type and default expression | Read-only component output |
| `connection-port` | `out` | `string` | `root.candidate.op == "" ? "" : (root.pending.target == "reconnect" && root.pending.number == 0 ? root.candidate.port : root.candidate.target-port)` | Constrained by its type and default expression | Read-only component output |
| `show-search` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-context-menu` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `properties-open` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `search-open` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `snap` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `chinese` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-controls` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-minimap` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-edges` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `animations-enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-edit-controls` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `show-edge-editor` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-alignment-controls` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `edge-style` | `in-out` | `int` | `1` | Constrained by its type and default expression | Shared by host and component |
| `edge-arrow` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `initial-options-json` | `in` | `string` | `"{}"` | Constrained by its type and default expression | Host-owned input |
| `state` | `in-out` | `ForgeFlowState` | `Type default` | Constrained by its type and default expression | Shared by host and component |
| `viewport` | `in-out` | `ForgeFlowViewport` | `{x: 40, y: 40, zoom: 1}` | Constrained by its type and default expression | Shared by host and component |
| `error` | `out` | `string` | `Type default` | Constrained by its type and default expression | Read-only component output |
| `dirty` | `out` | `bool` | `root.state.dirty` | Constrained by its type and default expression | Read-only component output |
| `disposed` | `out` | `bool` | `false` | Constrained by its type and default expression | Read-only component output |
| `connection-path` | `out` | `string` | `root.linking ? root.preview-edge.path : ""` | Constrained by its type and default expression | Read-only component output |
| `pan-mode` | `out` | `bool` | `root.space-down \|\| root.pan-on-drag` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `dirty-changed` | `(bool)` | Emitted by the component and handled by the host. |
| Callback | `save-finished` | `(string, string)` | Emitted by the component and handled by the host. |
| Callback | `image-exported` | `(string, string)` | Emitted by the component and handled by the host. |
| Callback | `changed-document` | `(ForgeFlowState)` | Emitted by the component and handled by the host. |
| Callback | `connection-allowed` | `(ForgeFlowAction) -> bool` | Emitted by the component and handled by the host. |
| Callback | `edit-allowed` | `(ForgeFlowAction) -> bool` | Emitted by the component and handled by the host. |
| Callback | `edited` | `(ForgeFlowAction, ForgeFlowState)` | Emitted by the component and handled by the host. |
| Callback | `edit-rejected` | `(ForgeFlowAction, string)` | Emitted by the component and handled by the host. |
| Callback | `interaction` | `(string, ForgeFlowAction)` | Emitted by the component and handled by the host. |
| Public function | `initialize` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `dispose` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `reopen` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `select-next` | `(reverse: bool, edges: bool)` | Called by the host and handled synchronously by the component. |
| Public function | `commit-view` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `zoom-at` | `(value: float, px: float, py: float)` | Called by the host and handled synchronously by the component. |
| Public function | `fit` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `fit-selected` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `focus-node` | `(id: string)` | Called by the host and handled synchronously by the component. |
| Public function | `fit-bounds` | `(b: ForgeFlowRect)` | Called by the host and handled synchronously by the component. |
| Public function | `wheel` | `(dx: float, dy: float, px: float, py: float, control: bool) -> bool` | Called by the host and handled synchronously by the component. |
| Public function | `change-edge-style` | `(style: int)` | Called by the host and handled synchronously by the component. |
| Public function | `act` | `(a: ForgeFlowAction)` | Called by the host and handled synchronously by the component. |
| Public function | `finish-connection` | `(x: float, y: float)` | Called by the host and handled synchronously by the component. |
| Public function | `edge-at` | `(x: float, y: float) -> string` | Called by the host and handled synchronously by the component. |
| Public function | `track-pointer` | `(x: float, y: float)` | Called by the host and handled synchronously by the component. |
| Public function | `update-connection` | `(x: float, y: float)` | Called by the host and handled synchronously by the component. |
| Public function | `save` | `(path: string)` | Called by the host and handled synchronously by the component. |
| Public function | `export-image` | `(path: string, selected: bool, options: string)` | Called by the host and handled synchronously by the component. |
| Public function | `load` | `(path: string)` | Called by the host and handled synchronously by the component. |
| Public function | `document-json` | `() -> string` | Called by the host and handled synchronously by the component. |
| Public function | `focus-canvas` | `()` | Called by the host and handled synchronously by the component. |
| Default slot | `default` | `@children` | Hosts Slint children supplied by the caller. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-flow-canvas.slint{qml}

## Host integration

This component references `ForgeFlowServices`. Initialize the runtime and bind the generated Slint global service type:

```rust
slintforge::runtime::initialize()?;
slintforge::bind_flow_services!(ui, /* generated Slint service/result types */);
```

Replace macro type arguments with the types generated by the consumer's `slint!`/`include_modules!` call.

```cpp
#include <ForgeFlowBindings.hpp>

forge_bind_flow_services(ui);
```


## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/flow/forge-flow-canvas)
