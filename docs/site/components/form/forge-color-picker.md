---
title: ForgeColorPicker
description: ForgeColorPicker 的公共 API、行为、示例与宿主接入说明。
---

# ForgeColorPicker

`ForgeColorPicker` 是 **表单组件** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-color-picker" name="ForgeColorPicker" />

## 用途与适用场景

- **适用场景：** 在 表单组件 页面中组合 `ForgeColorPicker` 所代表的交互或展示能力。
- **继承关系：**`ForgeColorPicker` → `Rectangle`。
- **源码位置：**`ui/form/color-picker.slint:7`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeColorPicker } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `language` | `in` | `string` | `"zh"` | 由类型与默认表达式约束 | 宿主输入 |
| `dark` | `in` | `bool` | `Palette.color-scheme == ColorScheme.dark` | 由类型与默认表达式约束 | 宿主输入 |
| `enabled` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `red` | `in-out` | `int` | `64` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `green` | `in-out` | `int` | `128` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `blue` | `in-out` | `int` | `224` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `alpha` | `in-out` | `int` | `255` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `value` | `out` | `color` | `rgba(max(0, min(255, root.red)), max(0, min(255, root.green)), max(0, min(255, root.blue)), max(0, min(255, root.alpha)) / 255)` | 由类型与默认表达式约束 | 组件只读输出 |
| `draft-value` | `out` | `color` | `hsv(root.hue, root.saturation, root.brightness, root.opacity-value)` | 由类型与默认表达式约束 | 组件只读输出 |
| `is-open` | `out` | `bool` | `popup.is-open` | 由类型与默认表达式约束 | 组件只读输出 |
| `picking-screen` | `out` | `bool` | `root.picking` | 由类型与默认表达式约束 | 组件只读输出 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `color-changed` | `(color)` | 由组件触发，宿主负责处理。 |
| 回调 | `accepted` | `(color)` | 由组件触发，宿主负责处理。 |
| 回调 | `canceled` | `()` | 由组件触发，宿主负责处理。 |
| 公开函数 | `set-draft` | `(value: color)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `open` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `close` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `confirm` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `cancel` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `edit-hex` | `(text: string)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `start-screen-pick` | `()` | 由宿主调用，组件同步处理。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 源码包含焦点或按键事件处理。
- **鼠标/触控：** 源码包含指针或触控处理。
- **可访问性：** 已声明可访问性元数据；宿主仍需提供有意义的标签。
- **本地化：** 组件包含翻译文本或语言输入。
- **主题：** 使用 SlintForge 主题或排版令牌。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-color-picker.slint{qml}

## 宿主接入

本组件引用 `ForgeWidgetServices`。先初始化运行库，再绑定由 Slint 生成的全局服务类型：

```rust
slintforge::runtime::initialize()?;
slintforge::bind_widget_services!(ui, /* generated Slint service/result types */);
```

宏的类型参数应替换为消费工程 `slint!`/`include_modules!` 生成的类型。

```cpp
#include <ForgeWidgets.hpp>

forge_bind_widget_services(ui);
```


## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/form/forge-color-picker)
