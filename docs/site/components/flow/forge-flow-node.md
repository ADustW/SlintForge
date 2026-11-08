---
title: ForgeFlowNode
description: ForgeFlowNode 的公共 API、行为、示例与宿主接入说明。
---

# ForgeFlowNode

`ForgeFlowNode` 是 **ForgeFlow** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-flow-node" name="ForgeFlowNode" />

## 用途与适用场景

- **适用场景：** 在 ForgeFlow 页面中组合 `ForgeFlowNode` 所代表的交互或展示能力。
- **继承关系：**`ForgeFlowNode` → `Rectangle`。
- **源码位置：**`ui/flow/node.slint:45`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeFlowNode } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `node` | `in` | `ForgeFlowNodeData` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `read-only` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `tree` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `chinese` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `snap` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `pan-mode` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `resizable` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `reparent-on-drop` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `minimum-resize-width` | `in` | `float` | `160` | 由类型与默认表达式约束 | 宿主输入 |
| `minimum-resize-height` | `in` | `float` | `112` | 由类型与默认表达式约束 | 宿主输入 |
| `drag-area` | `in` | `ForgeFlowRect` | `{x: 0, y: 0, width: root.node.width, height: root.node.header-visible ? root.node.header-height : 0}` | 由类型与默认表达式约束 | 宿主输入 |
| `auto-size` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `content-width` | `in` | `float` | `0` | 由类型与默认表达式约束 | 宿主输入 |
| `content-height` | `in` | `float` | `0` | 由类型与默认表达式约束 | 宿主输入 |
| `zoom` | `in` | `float` | `1` | 由类型与默认表达式约束 | 宿主输入 |
| `candidate-port` | `in` | `string` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `accent` | `in` | `color` | `root.node.kind == "form" ? #7c5ce7 : root.node.kind == "device" ? #11998a : root.node.kind == "image" ? #d48727 : ForgeFlowTheme.accent` | 由类型与默认表达式约束 | 宿主输入 |
| `card-background` | `in` | `color` | `root.node.is-group ? ForgeFlowTheme.raised.with-alpha(0.5) : ForgeFlowTheme.surface` | 由类型与默认表达式约束 | 宿主输入 |
| `corner-radius` | `in` | `length` | `ForgeFlowTheme.radius` | 由类型与默认表达式约束 | 宿主输入 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `action` | `(ForgeFlowAction)` | 由组件触发，宿主负责处理。 |
| 公开函数 | `resize-to-content` | `()` | 由宿主调用，组件同步处理。 |
| 默认插槽 | `default` | `@children` | 承载调用方提供的 Slint 子元素。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 源码包含焦点或按键事件处理。
- **鼠标/触控：** 源码包含指针或触控处理。
- **可访问性：** 已声明可访问性元数据；宿主仍需提供有意义的标签。
- **本地化：** 未检测到内部可翻译文本；调用方传入的字符串由宿主管理。
- **主题：** 使用组件声明颜色及继承的 Slint 样式。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-flow-node.slint{qml}

## 宿主接入

无需额外宿主服务。

## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/flow/forge-flow-node)
