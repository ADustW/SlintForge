---
title: ForgePagination
description: ForgePagination 的公共 API、行为、示例与宿主接入说明。
---

# ForgePagination

`ForgePagination` 是 **导航组件** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-pagination" name="ForgePagination" />

## 用途与适用场景

- **适用场景：** 在 导航组件 页面中组合 `ForgePagination` 所代表的交互或展示能力。
- **继承关系：**`ForgePagination` → `Rectangle`。
- **源码位置：**`ui/navigation/pagination.slint:3`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgePagination } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `total-items` | `in` | `int` | `0` | 由类型与默认表达式约束 | 宿主输入 |
| `page-size` | `in` | `int` | `10` | 由类型与默认表达式约束 | 宿主输入 |
| `enabled` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `current-page` | `in-out` | `int` | `1` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `page-count` | `out` | `int` | `root.total-items <= 0 \|\| root.page-size <= 0 ? 0 : ceil(root.total-items / root.page-size)` | 由类型与默认表达式约束 | 组件只读输出 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `page-changed` | `(int)` | 由组件触发，宿主负责处理。 |
| 公开函数 | `go-to` | `(page: int)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `normalize` | `()` | 由宿主调用，组件同步处理。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 该组件源码未声明直接按键处理器。
- **鼠标/触控：** 该组件源码未声明直接指针处理器。
- **可访问性：** 已声明可访问性元数据；宿主仍需提供有意义的标签。
- **本地化：** 组件包含翻译文本或语言输入。
- **主题：** 使用 SlintForge 主题或排版令牌。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-pagination.slint{qml}

## 宿主接入

无需额外宿主服务。

## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/navigation/forge-pagination)
