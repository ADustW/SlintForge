# Grids 公共类型

本子系统的公共结构体、枚举和全局对象。

| 符号 | 类型 | 成员 / 枚举值 | 源码 |
| --- | --- | --- | --- |
| `ForgeGridSelectionMode` | 枚举 | `none`, `single`, `multiple` | `ui/types/grids.slint:2` |
| `ForgeGridCell` | 结构体 | `text: string`, `value: float` | `ui/types/grids.slint:3` |
| `ForgeGridColumn` | 结构体 | `id: string`, `title: string`, `width: float`, `min-width: float`, `max-width: float`, `numeric: bool`, `sortable: bool`, `editable: bool`, `hidden: bool`, `frozen: bool` | `ui/types/grids.slint:4` |
| `ForgeGridRow` | 结构体 | `id: string`, `parent-id: string`, `cells: [ForgeGridCell]`, `expanded: bool`, `selected: bool`, `disabled: bool` | `ui/types/grids.slint:8` |
| `ForgeGridViewRow` | 结构体 | `id: string`, `parent-id: string`, `cells: [ForgeGridCell]`, `source-index: int`, `depth: int`, `has-children: bool`, `expanded: bool`, `selected: bool`, `disabled: bool` | `ui/types/grids.slint:13` |
| `ForgeGridViewColumn` | 结构体 | `column: ForgeGridColumn`, `source-index: int`, `x: float`, `width: float` | `ui/types/grids.slint:12` |
| `ForgeGridScene` | 结构体 | `status: int`, `rows: [ForgeGridViewRow]`, `columns: [ForgeGridViewColumn]`, `total-width: float`, `frozen-width: float`, `selected-count: int`, `all-visible-selected: bool` | `ui/types/grids.slint:17` |
| `ForgeGridEditResult` | 结构体 | `status: int`, `rows: [ForgeGridRow]` | `ui/types/grids.slint:21` |
| `ForgeGridServices` | 全局对象 | `connected` | `ui/types/grids.slint:22` |
