---
title: ForgeFileTree
description: ForgeFileTree public API, behavior, example, and host integration.
---

# ForgeFileTree

`ForgeFileTree` is a public component in the **Data Display** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-file-tree" name="ForgeFileTree" />

## Purpose and usage

- **Use when:** composing a data display screen that needs the behavior represented by `ForgeFileTree`.
- **Inheritance:** `ForgeFileTree` → `Rectangle`.
- **Source:** `ui/data/file-tree.slint:5`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeFileTree } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `items` | `in` | `[ForgeTreeItem]` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `multi-select` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `expansion-changed` | `(string, bool)` | Emitted by the component and handled by the host. |
| Callback | `selection-changed` | `(string, bool)` | Emitted by the component and handled by the host. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** No direct key-event handler is declared in this component source.
- **Pointer:** No direct pointer handler is declared in this component source.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** No internal translatable text was detected; caller-supplied strings remain host-owned.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-file-tree.slint{qml}

## Host integration

No additional host service is required.

## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/data/forge-file-tree)
