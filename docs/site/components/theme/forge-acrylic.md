---
title: ForgeAcrylic
description: ForgeAcrylic 的公共 API、行为、示例与宿主接入说明。
---

# ForgeAcrylic

`ForgeAcrylic` 是 **主题与材质** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-acrylic" name="ForgeAcrylic" />

## 用途与适用场景

- **适用场景：** 在 主题与材质 页面中组合 `ForgeAcrylic` 所代表的交互或展示能力。
- **继承关系：**`ForgeAcrylic` → `Rectangle`。
- **源码位置：**`ui/theme/backdrop.slint:5`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeAcrylic } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `source` | `in` | `image` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `blur-radius` | `in` | `int` | `10` | 由类型与默认表达式约束 | 宿主输入 |
| `refraction` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `tint` | `in` | `color` | `#7080a04c` | 由类型与默认表达式约束 | 宿主输入 |
| `content-padding` | `in` | `length` | `18px` | 由类型与默认表达式约束 | 宿主输入 |
| `processed` | `out` | `image` | `类型默认值` | 由类型与默认表达式约束 | 组件只读输出 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 公开函数 | `refresh` | `()` | 由宿主调用，组件同步处理。 |
| 默认插槽 | `default` | `@children` | 承载调用方提供的 Slint 子元素。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 该组件源码未声明直接按键处理器。
- **鼠标/触控：** 该组件源码未声明直接指针处理器。
- **可访问性：** 未检测到显式可访问性元数据；应在组合页面提供标签并做整页验证。
- **本地化：** 未检测到内部可翻译文本；调用方传入的字符串由宿主管理。
- **主题：** 使用 SlintForge 主题或排版令牌。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-acrylic.slint{qml}

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
- [English page](/en/components/theme/forge-acrylic)
