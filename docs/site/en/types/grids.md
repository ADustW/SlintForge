# Grids Shared Types

Public structs, enums, and globals for this subsystem.

| Symbol | Kind | Members / values | Source |
| --- | --- | --- | --- |
| `ForgeGridSelectionMode` | Enum | `none`, `single`, `multiple` | `ui/types/grids.slint:2` |
| `ForgeGridCell` | Struct | `text: string`, `value: float` | `ui/types/grids.slint:3` |
| `ForgeGridColumn` | Struct | `id: string`, `title: string`, `width: float`, `min-width: float`, `max-width: float`, `numeric: bool`, `sortable: bool`, `editable: bool`, `hidden: bool`, `frozen: bool` | `ui/types/grids.slint:4` |
| `ForgeGridRow` | Struct | `id: string`, `parent-id: string`, `cells: [ForgeGridCell]`, `expanded: bool`, `selected: bool`, `disabled: bool` | `ui/types/grids.slint:8` |
| `ForgeGridViewRow` | Struct | `id: string`, `parent-id: string`, `cells: [ForgeGridCell]`, `source-index: int`, `depth: int`, `has-children: bool`, `expanded: bool`, `selected: bool`, `disabled: bool` | `ui/types/grids.slint:13` |
| `ForgeGridViewColumn` | Struct | `column: ForgeGridColumn`, `source-index: int`, `x: float`, `width: float` | `ui/types/grids.slint:12` |
| `ForgeGridScene` | Struct | `status: int`, `rows: [ForgeGridViewRow]`, `columns: [ForgeGridViewColumn]`, `total-width: float`, `frozen-width: float`, `selected-count: int`, `all-visible-selected: bool` | `ui/types/grids.slint:17` |
| `ForgeGridEditResult` | Struct | `status: int`, `rows: [ForgeGridRow]` | `ui/types/grids.slint:21` |
| `ForgeGridServices` | Global | `connected` | `ui/types/grids.slint:22` |
