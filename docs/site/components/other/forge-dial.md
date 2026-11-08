---
title: ForgeDial
description: ForgeDial 的公共 API、行为、示例与宿主接入说明。
---

# ForgeDial

`ForgeDial` 是 **其他组件** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-dial" name="ForgeDial" />

## 用途与适用场景

- **适用场景：** 在 其他组件 页面中组合 `ForgeDial` 所代表的交互或展示能力。
- **继承关系：**`ForgeDial` → `Rectangle`。
- **源码位置：**`ui/other/dial.slint:4`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeDial } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `value` | `in-out` | `float` | `0` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `minimum` | `in` | `float` | `0` | 由类型与默认表达式约束 | 宿主输入 |
| `maximum` | `in` | `float` | `100` | 由类型与默认表达式约束 | 宿主输入 |
| `step` | `in` | `float` | `1` | 由类型与默认表达式约束 | 宿主输入 |
| `enabled` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `suffix` | `in` | `string` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `needle-angle` | `out` | `angle` | `(-135 + (root.maximum <= root.minimum ? 0 : (root.value - root.minimum) / (root.maximum - root.minimum)) * 270) * 1deg` | 由类型与默认表达式约束 | 组件只读输出 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `value-changed` | `(float)` | 由组件触发，宿主负责处理。 |
| 公开函数 | `set-value` | `(candidate: float)` | 由宿主调用，组件同步处理。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 源码包含焦点或按键事件处理。
- **鼠标/触控：** 源码包含指针或触控处理。
- **可访问性：** 已声明可访问性元数据；宿主仍需提供有意义的标签。
- **本地化：** 未检测到内部可翻译文本；调用方传入的字符串由宿主管理。
- **主题：** 使用 SlintForge 主题或排版令牌。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-dial.slint{qml}

## 宿主接入

无需额外宿主服务。

## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/other/forge-dial)
