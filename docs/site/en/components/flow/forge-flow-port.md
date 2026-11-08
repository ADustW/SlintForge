---
title: ForgeFlowPort
description: ForgeFlowPort public API, behavior, example, and host integration.
---

# ForgeFlowPort

`ForgeFlowPort` is a public component in the **ForgeFlow** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-flow-port" name="ForgeFlowPort" />

## Purpose and usage

- **Use when:** composing a forgeflow screen that needs the behavior represented by `ForgeFlowPort`.
- **Inheritance:** `ForgeFlowPort` → `Rectangle`.
- **Source:** `ui/flow/node.slint:5`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeFlowPort } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `port` | `in` | `ForgeFlowPortData` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `connected` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `read-only` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `accent` | `in` | `color` | `ForgeFlowTheme.accent` | Constrained by its type and default expression | Host-owned input |
| `candidate` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `keyboard-enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `name` | `in` | `string` | `root.port.id` | Constrained by its type and default expression | Host-owned input |
| `node-id` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `gesture` | `(string, float, float)` | Emitted by the component and handled by the host. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses its declared colors and inherited Slint styling.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-flow-port.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/flow/forge-flow-port)
