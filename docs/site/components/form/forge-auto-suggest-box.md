---
title: ForgeAutoSuggestBox
description: ForgeAutoSuggestBox 的公共 API、行为、示例与宿主接入说明。
---

# ForgeAutoSuggestBox

`ForgeAutoSuggestBox` 是 **表单组件** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-auto-suggest-box" name="ForgeAutoSuggestBox" />

## 用途与适用场景

- **适用场景：** 在 表单组件 页面中组合 `ForgeAutoSuggestBox` 所代表的交互或展示能力。
- **继承关系：**`ForgeAutoSuggestBox` → `Rectangle`。
- **源码位置：**`ui/form/auto-suggest.slint:4`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeAutoSuggestBox } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `text` | `in-out` | `string` | `类型默认值` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `suggestions` | `in` | `[string]` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `enabled` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `placeholder-text` | `in` | `string` | `@tr("输入以搜索")` | 由类型与默认表达式约束 | 宿主输入 |
| `open` | `in-out` | `bool` | `false` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `current-suggestion` | `in-out` | `int` | `-1` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `keyboard-navigation-ready` | `out` | `bool` | `ForgeWidgetServices.connected` | 由类型与默认表达式约束 | 组件只读输出 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `edited` | `(string)` | 由组件触发，宿主负责处理。 |
| 回调 | `selected` | `(string)` | 由组件触发，宿主负责处理。 |
| 公开函数 | `move-candidate` | `(direction: int)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `accept-current` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `choose` | `(value: string)` | 由宿主调用，组件同步处理。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 该组件源码未声明直接按键处理器。
- **鼠标/触控：** 该组件源码未声明直接指针处理器。
- **可访问性：** 已声明可访问性元数据；宿主仍需提供有意义的标签。
- **本地化：** 组件包含翻译文本或语言输入。
- **主题：** 使用组件声明颜色及继承的 Slint 样式。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-auto-suggest-box.slint{qml}

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
- [English page](/en/components/form/forge-auto-suggest-box)
