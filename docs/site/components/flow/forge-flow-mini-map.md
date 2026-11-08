---
title: ForgeFlowMiniMap
description: ForgeFlowMiniMap 的公共 API、行为、示例与宿主接入说明。
---

# ForgeFlowMiniMap

`ForgeFlowMiniMap` 是 **ForgeFlow** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-flow-mini-map" name="ForgeFlowMiniMap" />

## 用途与适用场景

- **适用场景：** 在 ForgeFlow 页面中组合 `ForgeFlowMiniMap` 所代表的交互或展示能力。
- **继承关系：**`ForgeFlowMiniMap` → `Rectangle`。
- **源码位置：**`ui/flow/controls.slint:45`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeFlowMiniMap } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `nodes` | `in` | `[ForgeFlowNodeData]` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `bounds` | `in` | `ForgeFlowRect` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `viewport` | `in` | `ForgeFlowViewport` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `canvas-width` | `in` | `float` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `canvas-height` | `in` | `float` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `viewport-color` | `in` | `color` | `ForgeFlowTheme.accent` | 由类型与默认表达式约束 | 宿主输入 |
| `viewport-fill` | `in` | `color` | `root.viewport-color.with-alpha(0.04)` | 由类型与默认表达式约束 | 宿主输入 |
| `node-radius` | `in` | `length` | `2px` | 由类型与默认表达式约束 | 宿主输入 |
| `zoom-step` | `in` | `float` | `1.12` | 由类型与默认表达式约束 | 宿主输入 |
| `wheel-enabled` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `wheel-requires-control` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `navigate` | `(float, float)` | 由组件触发，宿主负责处理。 |
| 回调 | `zoom-by` | `(float)` | 由组件触发，宿主负责处理。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 该组件源码未声明直接按键处理器。
- **鼠标/触控：** 源码包含指针或触控处理。
- **可访问性：** 未检测到显式可访问性元数据；应在组合页面提供标签并做整页验证。
- **本地化：** 未检测到内部可翻译文本；调用方传入的字符串由宿主管理。
- **主题：** 使用组件声明颜色及继承的 Slint 样式。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-flow-mini-map.slint{qml}

## 宿主接入

无需额外宿主服务。

## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/flow/forge-flow-mini-map)
