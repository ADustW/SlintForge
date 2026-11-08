---
title: ForgeFlowEdge
description: ForgeFlowEdge 的公共 API、行为、示例与宿主接入说明。
---

# ForgeFlowEdge

`ForgeFlowEdge` 是 **ForgeFlow** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-flow-edge" name="ForgeFlowEdge" />

## 用途与适用场景

- **适用场景：** 在 ForgeFlow 页面中组合 `ForgeFlowEdge` 所代表的交互或展示能力。
- **继承关系：**`ForgeFlowEdge` → `Rectangle`。
- **源码位置：**`ui/flow/edge.slint:5`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeFlowEdge } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `edge` | `in` | `ForgeFlowEdgeData` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `default-color` | `in` | `color` | `ForgeFlowTheme.edge` | 由类型与默认表达式约束 | 宿主输入 |
| `show-label` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `hovered` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `animations-enabled` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `activated` | `()` | 由组件触发，宿主负责处理。 |
| 默认插槽 | `default` | `@children` | 承载调用方提供的 Slint 子元素。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 该组件源码未声明直接按键处理器。
- **鼠标/触控：** 该组件源码未声明直接指针处理器。
- **可访问性：** 已声明可访问性元数据；宿主仍需提供有意义的标签。
- **本地化：** 未检测到内部可翻译文本；调用方传入的字符串由宿主管理。
- **主题：** 使用组件声明颜色及继承的 Slint 样式。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-flow-edge.slint{qml}

## 宿主接入

无需额外宿主服务。

## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/flow/forge-flow-edge)
