---
title: ForgeExpander
description: ForgeExpander 的公共 API、行为、示例与宿主接入说明。
---

# ForgeExpander

`ForgeExpander` 是 **容器组件** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-expander" name="ForgeExpander" />

## 用途与适用场景

- **适用场景：** 在 容器组件 页面中组合 `ForgeExpander` 所代表的交互或展示能力。
- **继承关系：**`ForgeExpander` → `Rectangle`。
- **源码位置：**`ui/surfaces/expander.slint:5`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeExpander } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `title` | `in` | `string` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `expanded` | `in-out` | `bool` | `false` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `content-height` | `in` | `length` | `0px` | 由类型与默认表达式约束 | 宿主输入 |
| `content-padding` | `in` | `length` | `12px` | 由类型与默认表达式约束 | 宿主输入 |
| `enabled` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `natural-content-height` | `out` | `length` | `slot.preferred-height + 2 * Math.max(0px, root.content-padding)` | 由类型与默认表达式约束 | 组件只读输出 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `toggled` | `(bool)` | 由组件触发，宿主负责处理。 |
| 公开函数 | `toggle` | `()` | 由宿主调用，组件同步处理。 |
| 默认插槽 | `default` | `@children` | 承载调用方提供的 Slint 子元素。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 源码包含焦点或按键事件处理。
- **鼠标/触控：** 源码包含指针或触控处理。
- **可访问性：** 已声明可访问性元数据；宿主仍需提供有意义的标签。
- **本地化：** 未检测到内部可翻译文本；调用方传入的字符串由宿主管理。
- **主题：** 使用 SlintForge 主题或排版令牌。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-expander.slint{qml}

## 宿主接入

无需额外宿主服务。

## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/surfaces/forge-expander)
