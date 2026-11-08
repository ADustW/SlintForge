---
title: ForgeFlowEdge
description: ForgeFlowEdge public API, behavior, example, and host integration.
---

# ForgeFlowEdge

`ForgeFlowEdge` is a public component in the **ForgeFlow** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-flow-edge" name="ForgeFlowEdge" />

## Purpose and usage

- **Use when:** composing a forgeflow screen that needs the behavior represented by `ForgeFlowEdge`.
- **Inheritance:** `ForgeFlowEdge` → `Rectangle`.
- **Source:** `ui/flow/edge.slint:5`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeFlowEdge } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `edge` | `in` | `ForgeFlowEdgeData` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `default-color` | `in` | `color` | `ForgeFlowTheme.edge` | Constrained by its type and default expression | Host-owned input |
| `show-label` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `hovered` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `animations-enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `activated` | `()` | Emitted by the component and handled by the host. |
| Default slot | `default` | `@children` | Hosts Slint children supplied by the caller. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses its declared colors and inherited Slint styling.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-flow-edge.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/flow/forge-flow-edge)
