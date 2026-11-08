---
title: ForgeSyntaxView
description: ForgeSyntaxView 的公共 API、行为、示例与宿主接入说明。
---

# ForgeSyntaxView

`ForgeSyntaxView` 是 **其他组件** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-syntax-view" name="ForgeSyntaxView" />

## 用途与适用场景

- **适用场景：** 在 其他组件 页面中组合 `ForgeSyntaxView` 所代表的交互或展示能力。
- **继承关系：**`ForgeSyntaxView` → `Rectangle`。
- **源码位置：**`ui/other/syntax-view.slint:4`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeSyntaxView } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `code` | `in` | `string` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `language` | `in` | `string` | `"slint"` | 由类型与默认表达式约束 | 宿主输入 |
| `line-numbers` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `font-size` | `in` | `length` | `14px` | 由类型与默认表达式约束 | 宿主输入 |
| `lines` | `out` | `[ForgeSyntaxLine]` | `类型默认值` | 由类型与默认表达式约束 | 组件只读输出 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 公开函数 | `refresh` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `copy` | `()` | 由宿主调用，组件同步处理。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 该组件源码未声明直接按键处理器。
- **鼠标/触控：** 该组件源码未声明直接指针处理器。
- **可访问性：** 未检测到显式可访问性元数据；应在组合页面提供标签并做整页验证。
- **本地化：** 组件包含翻译文本或语言输入。
- **主题：** 使用组件声明颜色及继承的 Slint 样式。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-syntax-view.slint{qml}

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
- [English page](/en/components/other/forge-syntax-view)
