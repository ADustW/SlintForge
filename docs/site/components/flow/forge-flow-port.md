---
title: ForgeFlowPort
description: ForgeFlowPort 的公共 API、行为、示例与宿主接入说明。
---

# ForgeFlowPort

`ForgeFlowPort` 是 **ForgeFlow** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-flow-port" name="ForgeFlowPort" />

## 用途与适用场景

- **适用场景：** 在 ForgeFlow 页面中组合 `ForgeFlowPort` 所代表的交互或展示能力。
- **继承关系：**`ForgeFlowPort` → `Rectangle`。
- **源码位置：**`ui/flow/node.slint:5`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeFlowPort } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `port` | `in` | `ForgeFlowPortData` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `connected` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `read-only` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `accent` | `in` | `color` | `ForgeFlowTheme.accent` | 由类型与默认表达式约束 | 宿主输入 |
| `candidate` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `keyboard-enabled` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `name` | `in` | `string` | `root.port.id` | 由类型与默认表达式约束 | 宿主输入 |
| `node-id` | `in` | `string` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `gesture` | `(string, float, float)` | 由组件触发，宿主负责处理。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 源码包含焦点或按键事件处理。
- **鼠标/触控：** 源码包含指针或触控处理。
- **可访问性：** 已声明可访问性元数据；宿主仍需提供有意义的标签。
- **本地化：** 未检测到内部可翻译文本；调用方传入的字符串由宿主管理。
- **主题：** 使用组件声明颜色及继承的 Slint 样式。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-flow-port.slint{qml}

## 宿主接入

无需额外宿主服务。

## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/flow/forge-flow-port)
