# DataGrid 与 TreeDataGrid

> **兼容提示：**本页继续保留 Grid 专题说明。逐组件中英文 API 见 [中文 API 站点源码](site/index.md) 与 [English API source](site/en/index.md)。

基线为 Slint 1.18.0、fluent 样式。`ForgeDataGrid` 和 `ForgeTreeDataGrid` 共用 `ui/data/data-grid.slint`；后者默认 `tree: true`。Rust/C++ 绑定同一份 Slint 契约，原生 `ListView` 负责可见行虚拟化和双向滚动，SDK 负责排序、筛选、树关系和数据操作。

Slint 原生 [StandardTableView](https://docs.slint.dev/latest/docs/slint/reference/std-widgets/views/standardtableview/) 提供文本表格、当前行、排序回调和列宽调整。本组件在此之外提供多选、单元格编辑、冻结/隐藏列和树形表格；普通文本表格仍可直接使用原生控件。

## 模型

| 类型 | 字段与规则 |
|---|---|
| `ForgeGridCell` | `text` 用于显示和编辑，`value` 用于数值排序。数值列应同时初始化这两个字段。 |
| `ForgeGridColumn` | 非空且唯一的 `id`、`title`；`width`、`min-width`、`max-width` 为未缩放的逻辑像素数；`numeric`、`sortable`、`editable`、`hidden`、`frozen`。布尔字段默认 false，需要明确启用。 |
| `ForgeGridRow` | 非空且唯一的 `id`、`parent-id`、按源列顺序排列的 `cells`、`expanded`、`selected`、`disabled`。缺少的单元格显示为空。 |
| `ForgeGridSelectionMode` | `none`、`single`、`multiple`；默认 multiple。模式控制用户操作；外部回传的 selected 标记由应用管理。 |
| `ForgeGridViewRow` | 计算后的可见行，保留 `source-index`、`id` 和源字段，增加 `depth`、`has-children`。 |
| `ForgeGridViewColumn` | 计算后的可见列，保留 `source-index`，增加 `x`、`width`。冻结列排在前面，每一组内部保留源顺序。 |
| `ForgeGridScene` | `status`：0 正常、1 空数据/空列/无匹配、2 数据关系无效、3 尚未绑定服务；还有可见 rows/columns、宽度、选择数量。 |

树的根节点 `parent-id` 为空，其他节点必须引用存在的行 ID；不允许重复 ID、孤立父节点或循环。输入行不必按层级排列。树排序只调整同级节点，筛选匹配所有单元格，保留祖先路径，并临时显示匹配路径；清除筛选后恢复原有展开标记。筛选期间展开按钮停用。文本匹配和排序进行 ASCII 大小写折叠，保留非 ASCII 字符；不依赖系统区域设置。数值按 `value` 排序，缺失/非有限数值排在最后，相等项保持原顺序。

## 属性与操作

`rows`、`columns`、`sort-column`、`sort-descending`、`filter` 和 `current-row-id` 可双向绑定。排序列索引、`tree-column` 和编辑的列索引始终指向源 columns，隐藏或冻结列后仍保持这个约定。行操作函数接收当前可见行索引；回调给出稳定 ID。`current-row` 是当前可见行索引，隐藏或删除当前行时为 -1。

| 接口 | 用途 |
|---|---|
| `enabled`、`editable`、`selection-mode` | 控制交互和编辑；禁用行跳过选择、编辑和键盘导航。 |
| `show-row-numbers`、`alternating-rows`、`row-height`、`header-height` | 控制序号、交替底色和尺寸；字号比例由 `ForgeTypography.scale` 管理。 |
| `visible-row-count`、`selected-row-count`、`scene`、`scroll-x/y` | 读取视图、全部源行中的选择数量和滚动位置。 |
| `select-row(index, toggle, range)`、`select-all()`、`clear-selection()` | 单行/切换/范围选择；全选仅作用于可见且未禁用的行。Ctrl+A 重复执行仍保持全选。 |
| `toggle-visible-selection()` | 根据当前可见行是否全部选中，选择或取消这些行；保留不可见行的选择。 |
| `focus-row(index)`、`scroll-to-row(index)` | 定位行；focus-row 同时设置当前行并接管键盘焦点。 |
| `sort-by(source-column)`、`resize-column(source-column, width)` | 同一列重复排序切换方向；宽度限制在列的有效范围。默认范围 40～4096，最小宽度至少 32。 |
| `toggle-row(index)`、`expand-all(bool)` | 树展开/折叠；平面表格忽略。 |
| `begin-edit(index, source-column)`、`commit-edit()`、`cancel-edit()` | 编辑器自动滚动到可见位置；`editing` 和 `edit-message` 表示编辑状态与错误。 |
| `delete-selected()` | 删除所选行；树形表格同时删除这些行的后代，保留有效树关系。 |
| `refresh()` | 外部修改嵌套 cells 模型后重新计算视图。 |

鼠标单击选择，Ctrl/Meta 单击切换，Shift 单击按可见顺序选择范围；双击可编辑单元格进入编辑。表头排序，列边界拖拽改宽度。键盘支持方向键、Home/End、PageUp/PageDown、Ctrl+A、Space、F2、Enter 和 Escape；树左右键控制展开、折叠及父子定位。编辑器 Enter 提交，Escape 放弃。数值编辑使用有限的 Slint float，拒绝非法数字、NaN、无穷和溢出/下溢；失败保留编辑文本与原数据。

回调：`row-clicked(id)`、`row-activated(id)`、`row-context-requested(id)`、`selection-changed()`、`sort-changed(column-id, descending)`、`column-resized(column-id, width)`、`cell-edited(row-id, column-id, text)`、`expansion-changed(row-id, expanded)`、`rows-removed(count)`。应用通过这些回调保存业务数据、显示菜单或打开详情；组件不访问数据库或文件系统。

交互会更新组件的 rows/columns。需要保存数据时将模型双向连接到应用属性，或在回调后读取新模型。外部更新顶层模型自动重新计算；在原有模型中调用 set-row-data 等操作或更新嵌套 cells 后调用 refresh。绑定与模型更新均在 UI 线程执行；后台任务通过 Slint 的事件循环回传数据。

## 服务绑定

消费窗口导出以下类型，宿主创建窗口后、运行窗口前绑定服务。未绑定时组件显示提示，不会静默显示为空。

```slint
import { ForgeDataGrid } from "@slintforge";
export { ForgeGridServices, ForgeGridScene, ForgeGridViewRow, ForgeGridViewColumn,
    ForgeGridRow, ForgeGridCell, ForgeGridColumn, ForgeGridEditResult } from "@slintforge";
```

```rust
slint::include_modules!();
use slint::ComponentHandle;
let ui = App::new()?;
slintforge::bind_grid_services!(ui, ForgeGridServices, ForgeGridScene, ForgeGridViewRow,
    ForgeGridViewColumn, ForgeGridRow, ForgeGridCell, ForgeGridColumn, ForgeGridEditResult);
ui.run()?;
```

宏的生成模型参数使用当前模块内的类型名。底层纯函数位于 `slintforge::grids`：`prepare_grid`、`update_grid`、`commit_grid`、`step_grid`；适用于在 UI 外准备和校验数据。

```cpp
#include "app.h"
#include "ForgeGrids.hpp"
auto ui = App::create();
forge_bind_grid_services<ForgeGridServices, ForgeGridScene, ForgeGridViewRow, ForgeGridViewColumn,
    ForgeGridRow, ForgeGridCell, ForgeGridColumn, ForgeGridEditResult>(ui);
ui->run();
```

C++ 底层接口为 `forge_prepare_grid`、`forge_update_grid`、`forge_commit_grid`、`forge_step_grid`；所有第一方符号使用全局唯一前缀。安装包包含头文件、静态实现和共享 Slint 文件，外部工程使用 `find_package(SlintForge CONFIG REQUIRED)` 和 `SlintForge::Sdk`。

Gallery 两页提供新增/删除/更新、万行加载、禁用、多选、冻结/隐藏列及树展开操作。独立消费者 `examples/ui/grids.slint` 使用公共组件，两个宿主的 `--grids-smoke` 检查平面、树形、空、禁用、排序选择、编辑和筛选七种状态。
