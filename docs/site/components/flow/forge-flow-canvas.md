---
title: ForgeFlowCanvas
description: ForgeFlowCanvas 的公共 API、行为、示例与宿主接入说明。
---

# ForgeFlowCanvas

`ForgeFlowCanvas` 是 **ForgeFlow** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-flow-canvas" name="ForgeFlowCanvas" />

## 用途与适用场景

- **适用场景：** 在 ForgeFlow 页面中组合 `ForgeFlowCanvas` 所代表的交互或展示能力。
- **继承关系：**`ForgeFlowCanvas` → `Rectangle`。
- **源码位置：**`ui/flow/canvas.slint:12`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeFlowCanvas } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `mode` | `in` | `string` | `"graph"` | 由类型与默认表达式约束 | 宿主输入 |
| `demo` | `in` | `int` | `-1` | 由类型与默认表达式约束 | 宿主输入 |
| `read-only` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `grid` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `background-variant` | `in-out` | `int` | `0` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `pan-on-drag` | `in-out` | `bool` | `false` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `show-node-toolbar` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `connection-target` | `out` | `string` | `root.candidate.op == "" ? "" : (root.pending.target == "reconnect" && root.pending.number == 0 ? root.candidate.id : root.candidate.target)` | 由类型与默认表达式约束 | 组件只读输出 |
| `connection-port` | `out` | `string` | `root.candidate.op == "" ? "" : (root.pending.target == "reconnect" && root.pending.number == 0 ? root.candidate.port : root.candidate.target-port)` | 由类型与默认表达式约束 | 组件只读输出 |
| `show-search` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `show-context-menu` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `properties-open` | `in-out` | `bool` | `false` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `search-open` | `in-out` | `bool` | `false` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `snap` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `chinese` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `show-controls` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `show-minimap` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `show-edges` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `animations-enabled` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `show-edit-controls` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `show-edge-editor` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `show-alignment-controls` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `edge-style` | `in-out` | `int` | `1` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `edge-arrow` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `initial-options-json` | `in` | `string` | `"{}"` | 由类型与默认表达式约束 | 宿主输入 |
| `state` | `in-out` | `ForgeFlowState` | `类型默认值` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `viewport` | `in-out` | `ForgeFlowViewport` | `{x: 40, y: 40, zoom: 1}` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `error` | `out` | `string` | `类型默认值` | 由类型与默认表达式约束 | 组件只读输出 |
| `dirty` | `out` | `bool` | `root.state.dirty` | 由类型与默认表达式约束 | 组件只读输出 |
| `disposed` | `out` | `bool` | `false` | 由类型与默认表达式约束 | 组件只读输出 |
| `connection-path` | `out` | `string` | `root.linking ? root.preview-edge.path : ""` | 由类型与默认表达式约束 | 组件只读输出 |
| `pan-mode` | `out` | `bool` | `root.space-down \|\| root.pan-on-drag` | 由类型与默认表达式约束 | 组件只读输出 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `dirty-changed` | `(bool)` | 由组件触发，宿主负责处理。 |
| 回调 | `save-finished` | `(string, string)` | 由组件触发，宿主负责处理。 |
| 回调 | `image-exported` | `(string, string)` | 由组件触发，宿主负责处理。 |
| 回调 | `changed-document` | `(ForgeFlowState)` | 由组件触发，宿主负责处理。 |
| 回调 | `connection-allowed` | `(ForgeFlowAction) -> bool` | 由组件触发，宿主负责处理。 |
| 回调 | `edit-allowed` | `(ForgeFlowAction) -> bool` | 由组件触发，宿主负责处理。 |
| 回调 | `edited` | `(ForgeFlowAction, ForgeFlowState)` | 由组件触发，宿主负责处理。 |
| 回调 | `edit-rejected` | `(ForgeFlowAction, string)` | 由组件触发，宿主负责处理。 |
| 回调 | `interaction` | `(string, ForgeFlowAction)` | 由组件触发，宿主负责处理。 |
| 公开函数 | `initialize` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `dispose` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `reopen` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `select-next` | `(reverse: bool, edges: bool)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `commit-view` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `zoom-at` | `(value: float, px: float, py: float)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `fit` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `fit-selected` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `focus-node` | `(id: string)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `fit-bounds` | `(b: ForgeFlowRect)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `wheel` | `(dx: float, dy: float, px: float, py: float, control: bool) -> bool` | 由宿主调用，组件同步处理。 |
| 公开函数 | `change-edge-style` | `(style: int)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `act` | `(a: ForgeFlowAction)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `finish-connection` | `(x: float, y: float)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `edge-at` | `(x: float, y: float) -> string` | 由宿主调用，组件同步处理。 |
| 公开函数 | `track-pointer` | `(x: float, y: float)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `update-connection` | `(x: float, y: float)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `save` | `(path: string)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `export-image` | `(path: string, selected: bool, options: string)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `load` | `(path: string)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `document-json` | `() -> string` | 由宿主调用，组件同步处理。 |
| 公开函数 | `focus-canvas` | `()` | 由宿主调用，组件同步处理。 |
| 默认插槽 | `default` | `@children` | 承载调用方提供的 Slint 子元素。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 源码包含焦点或按键事件处理。
- **鼠标/触控：** 源码包含指针或触控处理。
- **可访问性：** 已声明可访问性元数据；宿主仍需提供有意义的标签。
- **本地化：** 未检测到内部可翻译文本；调用方传入的字符串由宿主管理。
- **主题：** 使用 SlintForge 主题或排版令牌。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-flow-canvas.slint{qml}

## 宿主接入

本组件引用 `ForgeFlowServices`。先初始化运行库，再绑定由 Slint 生成的全局服务类型：

```rust
slintforge::runtime::initialize()?;
slintforge::bind_flow_services!(ui, /* generated Slint service/result types */);
```

宏的类型参数应替换为消费工程 `slint!`/`include_modules!` 生成的类型。

```cpp
#include <ForgeFlowBindings.hpp>

forge_bind_flow_services(ui);
```


## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/flow/forge-flow-canvas)
