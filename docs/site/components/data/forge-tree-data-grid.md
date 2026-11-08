---
title: ForgeTreeDataGrid
description: ForgeTreeDataGrid 的公共 API、行为、示例与宿主接入说明。
---

# ForgeTreeDataGrid

`ForgeTreeDataGrid` 是 **数据展示** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-tree-data-grid" name="ForgeTreeDataGrid" />

## 用途与适用场景

- **适用场景：** 在 数据展示 页面中组合 `ForgeTreeDataGrid` 所代表的交互或展示能力。
- **继承关系：**`ForgeTreeDataGrid` → `ForgeDataGrid`。
- **源码位置：**`ui/data/data-grid.slint:426`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeTreeDataGrid } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| — | — | — | — | 无直接公开属性；请参阅继承类型。 | — |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| — | — | — | 无直接公开回调、函数或插槽。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 源码包含焦点或按键事件处理。
- **鼠标/触控：** 源码包含指针或触控处理。
- **可访问性：** 已声明可访问性元数据；宿主仍需提供有意义的标签。
- **本地化：** 组件包含翻译文本或语言输入。
- **主题：** 使用 SlintForge 主题或排版令牌。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-tree-data-grid.slint{qml}

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
- [English page](/en/components/data/forge-tree-data-grid)
