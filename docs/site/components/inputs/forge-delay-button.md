---
title: ForgeDelayButton
description: ForgeDelayButton 的公共 API、行为、示例与宿主接入说明。
---

# ForgeDelayButton

`ForgeDelayButton` 是 **输入组件** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-delay-button" name="ForgeDelayButton" />

## 用途与适用场景

- **适用场景：** 在 输入组件 页面中组合 `ForgeDelayButton` 所代表的交互或展示能力。
- **继承关系：**`ForgeDelayButton` → `Rectangle`。
- **源码位置：**`ui/inputs/action-buttons.slint:55`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeDelayButton } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `text` | `in` | `string` | `@tr("按住确认")` | 由类型与默认表达式约束 | 宿主输入 |
| `delay` | `in` | `duration` | `1200ms` | 由类型与默认表达式约束 | 宿主输入 |
| `enabled` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `mode` | `in` | `ForgeDelayButtonMode` | `hold-release` | 由类型与默认表达式约束 | 宿主输入 |
| `primary` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `pill` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `show-progress` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `running` | `out` | `bool` | `root.active` | 由类型与默认表达式约束 | 组件只读输出 |
| `pressed` | `out` | `bool` | `touch.pressed \|\| root.key-held` | 由类型与默认表达式约束 | 组件只读输出 |
| `has-focus` | `out` | `bool` | `focus.has-focus` | 由类型与默认表达式约束 | 组件只读输出 |
| `progress` | `out` | `float` | `!root.active ? 0 : root.delay <= 0ms ? 1 : min(1, root.elapsed / root.delay)` | 由类型与默认表达式约束 | 组件只读输出 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `clicked` | `()` | 由组件触发，宿主负责处理。 |
| 回调 | `canceled` | `()` | 由组件触发，宿主负责处理。 |
| 公开函数 | `focus-button` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `cancel` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `begin` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `finish` | `()` | 由宿主调用，组件同步处理。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 源码包含焦点或按键事件处理。
- **鼠标/触控：** 源码包含指针或触控处理。
- **可访问性：** 已声明可访问性元数据；宿主仍需提供有意义的标签。
- **本地化：** 组件包含翻译文本或语言输入。
- **主题：** 使用 SlintForge 主题或排版令牌。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-delay-button.slint{qml}

## 宿主接入

无需额外宿主服务。

## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/inputs/forge-delay-button)
