---
title: ForgeDataGrid
description: ForgeDataGrid 的公共 API、行为、示例与宿主接入说明。
---

# ForgeDataGrid

`ForgeDataGrid` 是 **数据展示** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-data-grid" name="ForgeDataGrid" />

## 用途与适用场景

- **适用场景：** 在 数据展示 页面中组合 `ForgeDataGrid` 所代表的交互或展示能力。
- **继承关系：**`ForgeDataGrid` → `Rectangle`。
- **源码位置：**`ui/data/data-grid.slint:64`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeDataGrid } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `columns` | `in-out` | `[ForgeGridColumn]` | `类型默认值` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `rows` | `in-out` | `[ForgeGridRow]` | `类型默认值` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `tree` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `tree-column` | `in` | `int` | `0` | 由类型与默认表达式约束 | 宿主输入 |
| `selection-mode` | `in` | `ForgeGridSelectionMode` | `multiple` | 由类型与默认表达式约束 | 宿主输入 |
| `enabled` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `editable` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `show-row-numbers` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `alternating-rows` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `row-height` | `in` | `length` | `36px * ForgeTypography.scale` | 由类型与默认表达式约束 | 宿主输入 |
| `header-height` | `in` | `length` | `40px * ForgeTypography.scale` | 由类型与默认表达式约束 | 宿主输入 |
| `sort-column` | `in-out` | `int` | `-1` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `sort-descending` | `in-out` | `bool` | `false` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `filter` | `in-out` | `string` | `""` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `current-row-id` | `in-out` | `string` | `""` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `current-row` | `out` | `int` | `ForgeGridServices.connected ? ForgeGridServices.find-row(root.scene.rows, root.current-row-id) : -1` | 由类型与默认表达式约束 | 组件只读输出 |
| `visible-row-count` | `out` | `int` | `root.scene.rows.length` | 由类型与默认表达式约束 | 组件只读输出 |
| `selected-row-count` | `out` | `int` | `root.scene.selected-count` | 由类型与默认表达式约束 | 组件只读输出 |
| `scene` | `out` | `ForgeGridScene` | `ForgeGridServices.connected ? ForgeGridServices.prepare(root.rows, root.columns, root.tree, root.sort-column, root.sort-descending, root.filter, root.revision) : {status: 3, rows: [], columns: [], total-width: 0, frozen-width: 0, selected-count: 0, all-visible-selected: false}` | 由类型与默认表达式约束 | 组件只读输出 |
| `editing` | `out` | `bool` | `root.edit-row-id != ""` | 由类型与默认表达式约束 | 组件只读输出 |
| `edit-message` | `out` | `string` | `root.edit-error` | 由类型与默认表达式约束 | 组件只读输出 |
| `scroll-x` | `out` | `length` | `list.content-x` | 由类型与默认表达式约束 | 组件只读输出 |
| `scroll-y` | `out` | `length` | `list.content-y` | 由类型与默认表达式约束 | 组件只读输出 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `row-clicked` | `(string)` | 由组件触发，宿主负责处理。 |
| 回调 | `row-activated` | `(string)` | 由组件触发，宿主负责处理。 |
| 回调 | `row-context-requested` | `(string)` | 由组件触发，宿主负责处理。 |
| 回调 | `selection-changed` | `()` | 由组件触发，宿主负责处理。 |
| 回调 | `sort-changed` | `(string, bool)` | 由组件触发，宿主负责处理。 |
| 回调 | `column-resized` | `(string, float)` | 由组件触发，宿主负责处理。 |
| 回调 | `cell-edited` | `(string, string, string)` | 由组件触发，宿主负责处理。 |
| 回调 | `expansion-changed` | `(string, bool)` | 由组件触发，宿主负责处理。 |
| 回调 | `rows-removed` | `(int)` | 由组件触发，宿主负责处理。 |
| 公开函数 | `refresh` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `cancel-edit` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `focus-row` | `(index: int)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `select-row` | `(index: int, toggle: bool, range: bool)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `select-all` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `toggle-visible-selection` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `clear-selection` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `delete-selected` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `sort-by` | `(index: int)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `resize-column` | `(index: int, width: float)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `scroll-to-row` | `(index: int)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `toggle-row` | `(index: int)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `expand-all` | `(expanded: bool)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `begin-edit` | `(index: int, column: int)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `commit-edit` | `()` | 由宿主调用，组件同步处理。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 源码包含焦点或按键事件处理。
- **鼠标/触控：** 源码包含指针或触控处理。
- **可访问性：** 已声明可访问性元数据；宿主仍需提供有意义的标签。
- **本地化：** 组件包含翻译文本或语言输入。
- **主题：** 使用 SlintForge 主题或排版令牌。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-data-grid.slint{qml}

## 宿主接入

本组件引用 `ForgeGridServices`。先初始化运行库，再绑定由 Slint 生成的全局服务类型：

```rust
slintforge::runtime::initialize()?;
slintforge::bind_grid_services!(ui, /* generated Slint service/result types */);
```

宏的类型参数应替换为消费工程 `slint!`/`include_modules!` 生成的类型。

```cpp
#include <ForgeGrids.hpp>

forge_bind_grid_services(ui);
```


## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/data/forge-data-grid)
