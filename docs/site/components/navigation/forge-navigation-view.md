---
title: ForgeNavigationView
description: ForgeNavigationView 的公共 API、行为、示例与宿主接入说明。
---

# ForgeNavigationView

`ForgeNavigationView` 是 **导航组件** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-navigation-view" name="ForgeNavigationView" />

## 用途与适用场景

- **适用场景：** 在 导航组件 页面中组合 `ForgeNavigationView` 所代表的交互或展示能力。
- **继承关系：**`ForgeNavigationView` → `Rectangle`。
- **源码位置：**`ui/navigation/navigation-view.slint:9`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeNavigationView } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `items` | `in` | `[ForgeNavigationItem]` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `tree-items` | `in` | `[ForgeNavigationItem]` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `footer-items` | `in` | `[ForgeNavigationItem]` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `active-route-id` | `in` | `string` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `active-top-id` | `in` | `string` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `placement` | `in` | `ForgeNavPlacement` | `ForgeNavPlacement.left` | 由类型与默认表达式约束 | 宿主输入 |
| `pane-state` | `in` | `ForgePaneState` | `ForgePaneState.expanded` | 由类型与默认表达式约束 | 宿主输入 |
| `brand` | `in` | `string` | `"SlintForge Gallery"` | 由类型与默认表达式约束 | 宿主输入 |
| `search-query` | `in` | `string` | `""` | 由类型与默认表达式约束 | 宿主输入 |
| `can-go-back` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `dark-mode` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `show-app-bar` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `show-pane-header` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `show-pane-search` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `content-width` | `out` | `length` | `content.width` | 由类型与默认表达式约束 | 组件只读输出 |
| `content-height` | `out` | `length` | `content.height` | 由类型与默认表达式约束 | 组件只读输出 |
| `content-x` | `out` | `length` | `content.x` | 由类型与默认表达式约束 | 组件只读输出 |
| `content-y` | `out` | `length` | `body.y` | 由类型与默认表达式约束 | 组件只读输出 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `navigate-requested` | `(string, [ForgeRouteParam])` | 由组件触发，宿主负责处理。 |
| 回调 | `group-toggle-requested` | `(string)` | 由组件触发，宿主负责处理。 |
| 回调 | `pane-state-requested` | `(ForgePaneState)` | 由组件触发，宿主负责处理。 |
| 回调 | `search-requested` | `(string)` | 由组件触发，宿主负责处理。 |
| 回调 | `back-requested` | `()` | 由组件触发，宿主负责处理。 |
| 回调 | `theme-toggle-requested` | `()` | 由组件触发，宿主负责处理。 |
| 默认插槽 | `default` | `@children` | 承载调用方提供的 Slint 子元素。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 该组件源码未声明直接按键处理器。
- **鼠标/触控：** 源码包含指针或触控处理。
- **可访问性：** 未检测到显式可访问性元数据；应在组合页面提供标签并做整页验证。
- **本地化：** 未检测到内部可翻译文本；调用方传入的字符串由宿主管理。
- **主题：** 使用 SlintForge 主题或排版令牌。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-navigation-view.slint{qml}

## 宿主接入

无需额外宿主服务。

## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/navigation/forge-navigation-view)
