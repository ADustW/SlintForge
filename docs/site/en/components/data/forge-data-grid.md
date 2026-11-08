---
title: ForgeDataGrid
description: ForgeDataGrid public API, behavior, example, and host integration.
---

# ForgeDataGrid

`ForgeDataGrid` is a public component in the **Data Display** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-data-grid" name="ForgeDataGrid" />

## Purpose and usage

- **Use when:** composing a data display screen that needs the behavior represented by `ForgeDataGrid`.
- **Inheritance:** `ForgeDataGrid` → `Rectangle`.
- **Source:** `ui/data/data-grid.slint:64`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgeDataGrid } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `columns` | `in-out` | `[ForgeGridColumn]` | `Type default` | Constrained by its type and default expression | Shared by host and component |
| `rows` | `in-out` | `[ForgeGridRow]` | `Type default` | Constrained by its type and default expression | Shared by host and component |
| `tree` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `tree-column` | `in` | `int` | `0` | Constrained by its type and default expression | Host-owned input |
| `selection-mode` | `in` | `ForgeGridSelectionMode` | `multiple` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `editable` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-row-numbers` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `alternating-rows` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `row-height` | `in` | `length` | `36px * ForgeTypography.scale` | Constrained by its type and default expression | Host-owned input |
| `header-height` | `in` | `length` | `40px * ForgeTypography.scale` | Constrained by its type and default expression | Host-owned input |
| `sort-column` | `in-out` | `int` | `-1` | Constrained by its type and default expression | Shared by host and component |
| `sort-descending` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `filter` | `in-out` | `string` | `""` | Constrained by its type and default expression | Shared by host and component |
| `current-row-id` | `in-out` | `string` | `""` | Constrained by its type and default expression | Shared by host and component |
| `current-row` | `out` | `int` | `ForgeGridServices.connected ? ForgeGridServices.find-row(root.scene.rows, root.current-row-id) : -1` | Constrained by its type and default expression | Read-only component output |
| `visible-row-count` | `out` | `int` | `root.scene.rows.length` | Constrained by its type and default expression | Read-only component output |
| `selected-row-count` | `out` | `int` | `root.scene.selected-count` | Constrained by its type and default expression | Read-only component output |
| `scene` | `out` | `ForgeGridScene` | `ForgeGridServices.connected ? ForgeGridServices.prepare(root.rows, root.columns, root.tree, root.sort-column, root.sort-descending, root.filter, root.revision) : {status: 3, rows: [], columns: [], total-width: 0, frozen-width: 0, selected-count: 0, all-visible-selected: false}` | Constrained by its type and default expression | Read-only component output |
| `editing` | `out` | `bool` | `root.edit-row-id != ""` | Constrained by its type and default expression | Read-only component output |
| `edit-message` | `out` | `string` | `root.edit-error` | Constrained by its type and default expression | Read-only component output |
| `scroll-x` | `out` | `length` | `list.content-x` | Constrained by its type and default expression | Read-only component output |
| `scroll-y` | `out` | `length` | `list.content-y` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `row-clicked` | `(string)` | Emitted by the component and handled by the host. |
| Callback | `row-activated` | `(string)` | Emitted by the component and handled by the host. |
| Callback | `row-context-requested` | `(string)` | Emitted by the component and handled by the host. |
| Callback | `selection-changed` | `()` | Emitted by the component and handled by the host. |
| Callback | `sort-changed` | `(string, bool)` | Emitted by the component and handled by the host. |
| Callback | `column-resized` | `(string, float)` | Emitted by the component and handled by the host. |
| Callback | `cell-edited` | `(string, string, string)` | Emitted by the component and handled by the host. |
| Callback | `expansion-changed` | `(string, bool)` | Emitted by the component and handled by the host. |
| Callback | `rows-removed` | `(int)` | Emitted by the component and handled by the host. |
| Public function | `refresh` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `cancel-edit` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `focus-row` | `(index: int)` | Called by the host and handled synchronously by the component. |
| Public function | `select-row` | `(index: int, toggle: bool, range: bool)` | Called by the host and handled synchronously by the component. |
| Public function | `select-all` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `toggle-visible-selection` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `clear-selection` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `delete-selected` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `sort-by` | `(index: int)` | Called by the host and handled synchronously by the component. |
| Public function | `resize-column` | `(index: int, width: float)` | Called by the host and handled synchronously by the component. |
| Public function | `scroll-to-row` | `(index: int)` | Called by the host and handled synchronously by the component. |
| Public function | `toggle-row` | `(index: int)` | Called by the host and handled synchronously by the component. |
| Public function | `expand-all` | `(expanded: bool)` | Called by the host and handled synchronously by the component. |
| Public function | `begin-edit` | `(index: int, column: int)` | Called by the host and handled synchronously by the component. |
| Public function | `commit-edit` | `()` | Called by the host and handled synchronously by the component. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** Explicit accessible metadata is present; the host must still provide meaningful labels.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-data-grid.slint{qml}

## Host integration

This component references `ForgeGridServices`. Initialize the runtime and bind the generated Slint global service type:

```rust
slintforge::runtime::initialize()?;
slintforge::bind_grid_services!(ui, /* generated Slint service/result types */);
```

Replace macro type arguments with the types generated by the consumer's `slint!`/`include_modules!` call.

```cpp
#include <ForgeGrids.hpp>

forge_bind_grid_services(ui);
```


## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/data/forge-data-grid)
