---
title: ForgeDatePickerPopup
description: ForgeDatePickerPopup 的公共 API、行为、示例与宿主接入说明。
---

# ForgeDatePickerPopup

`ForgeDatePickerPopup` 是 **表单组件** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-date-picker-popup" name="ForgeDatePickerPopup" />

## 用途与适用场景

- **适用场景：** 在 表单组件 页面中组合 `ForgeDatePickerPopup` 所代表的交互或展示能力。
- **继承关系：**`ForgeDatePickerPopup` → `Rectangle`。
- **源码位置：**`ui/form/date-picker.slint:8`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeDatePickerPopup } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `language` | `in` | `string` | `"zh"` | 由类型与默认表达式约束 | 宿主输入 |
| `title` | `in` | `string` | `root.language == "en" ? "Select date" : "选择日期"` | 由类型与默认表达式约束 | 宿主输入 |
| `date` | `in` | `Date` | `{year: 2026, month: 10, day: 12}` | 由类型与默认表达式约束 | 宿主输入 |
| `minimum-date` | `in` | `Date` | `{year: 1, month: 1, day: 1}` | 由类型与默认表达式约束 | 宿主输入 |
| `maximum-date` | `in` | `Date` | `{year: 9999, month: 12, day: 31}` | 由类型与默认表达式约束 | 宿主输入 |
| `dark` | `in` | `bool` | `Palette.color-scheme == ColorScheme.dark` | 由类型与默认表达式约束 | 宿主输入 |
| `show-time` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `confirm-on-selection` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `time` | `in` | `Time` | `{hour: 9, minute: 0, second: 0}` | 由类型与默认表达式约束 | 宿主输入 |
| `minute-increment` | `in` | `int` | `1` | 由类型与默认表达式约束 | 宿主输入 |
| `is-open` | `out` | `bool` | `popup.is-open` | 由类型与默认表达式约束 | 组件只读输出 |
| `current-date` | `out` | `Date` | `root.draft` | 由类型与默认表达式约束 | 组件只读输出 |
| `current-time` | `out` | `Time` | `root.draft-time` | 由类型与默认表达式约束 | 组件只读输出 |
| `date-text` | `out` | `string` | `ForgeDateTime.format-date(root.draft, root.language)` | 由类型与默认表达式约束 | 组件只读输出 |
| `month-text` | `out` | `string` | `ForgeDateTime.format-month(root.display-year, root.display-month, root.language)` | 由类型与默认表达式约束 | 组件只读输出 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `accepted` | `(Date)` | 由组件触发，宿主负责处理。 |
| 回调 | `time-accepted` | `(Time)` | 由组件触发，宿主负责处理。 |
| 回调 | `canceled` | `()` | 由组件触发，宿主负责处理。 |
| 公开函数 | `open` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `close` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `set-time` | `(value: Time)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `choose-day` | `(day: int)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `browse-month` | `(delta: int)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `confirm` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `cancel` | `()` | 由宿主调用，组件同步处理。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 源码包含焦点或按键事件处理。
- **鼠标/触控：** 源码包含指针或触控处理。
- **可访问性：** 已声明可访问性元数据；宿主仍需提供有意义的标签。
- **本地化：** 组件包含翻译文本或语言输入。
- **主题：** 使用 SlintForge 主题或排版令牌。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-date-picker-popup.slint{qml}

## 宿主接入

无需额外宿主服务。

## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/form/forge-date-picker-popup)
