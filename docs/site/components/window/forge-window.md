---
title: ForgeWindow
description: ForgeWindow 的公共 API、行为、示例与宿主接入说明。
---

# ForgeWindow

`ForgeWindow` 是 **窗口** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-window" name="ForgeWindow" />

## 用途与适用场景

- **适用场景：** 在 窗口 页面中组合 `ForgeWindow` 所代表的交互或展示能力。
- **继承关系：**`ForgeWindow` → `Window`。
- **源码位置：**`ui/window/forge-window.slint:9`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgeWindow } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `brand-symbol` | `in` | `string` | `"S"` | 由类型与默认表达式约束 | 宿主输入 |
| `brand-icon` | `in` | `image` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `show-brand-icon` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `version-text` | `in` | `string` | `""` | 由类型与默认表达式约束 | 宿主输入 |
| `show-menu-button` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `leading-menu-button` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `show-back-button` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `back-enabled` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `show-search-box` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `search-text` | `in` | `string` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `search-placeholder` | `in` | `string` | `@tr("搜索组件")` | 由类型与默认表达式约束 | 宿主输入 |
| `search-has-focus` | `out` | `bool` | `root.show-search-box && search-input.has-focus` | 由类型与默认表达式约束 | 组件只读输出 |
| `navigation-glyph` | `in` | `string` | `ForgeIcons.panel-left` | 由类型与默认表达式约束 | 宿主输入 |
| `show-theme-button` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `show-pin-button` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `show-settings-button` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `show-maximize-button` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `dark-mode` | `in-out` | `bool` | `false` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `pinned` | `in-out` | `bool` | `false` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `rounded-corners` | `in-out` | `bool` | `true` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `title-bar-height` | `out` | `length` | `ForgeMetrics.title-bar-height` | 由类型与默认表达式约束 | 组件只读输出 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `menu-requested` | `()` | 由组件触发，宿主负责处理。 |
| 回调 | `back-clicked` | `()` | 由组件触发，宿主负责处理。 |
| 回调 | `search-edited` | `(string)` | 由组件触发，宿主负责处理。 |
| 回调 | `search-accepted` | `(string)` | 由组件触发，宿主负责处理。 |
| 回调 | `theme-toggle-requested` | `()` | 由组件触发，宿主负责处理。 |
| 回调 | `pin-state-changed` | `(bool)` | 由组件触发，宿主负责处理。 |
| 回调 | `settings-requested` | `()` | 由组件触发，宿主负责处理。 |
| 公开函数 | `focus-search` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `capture` | `() -> ForgeCaptureResult` | 由宿主调用，组件同步处理。 |
| 默认插槽 | `default` | `@children` | 承载调用方提供的 Slint 子元素。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 源码包含焦点或按键事件处理。
- **鼠标/触控：** 源码包含指针或触控处理。
- **可访问性：** 未检测到显式可访问性元数据；应在组合页面提供标签并做整页验证。
- **本地化：** 组件包含翻译文本或语言输入。
- **主题：** 使用 SlintForge 主题或排版令牌。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-window.slint{qml}

## 宿主接入

本组件引用 `ForgeCaptureService`。先初始化运行库，再绑定由 Slint 生成的全局服务类型：

```rust
slintforge::runtime::initialize()?;
slintforge::bind_capture_service!(ui, /* generated Slint service/result types */);
```

宏的类型参数应替换为消费工程 `slint!`/`include_modules!` 生成的类型。

```cpp
#include <ForgeCapture.hpp>

forge_bind_capture_service(ui);
```


## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/window/forge-window)
