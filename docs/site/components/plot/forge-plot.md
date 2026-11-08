---
title: ForgePlot
description: ForgePlot 的公共 API、行为、示例与宿主接入说明。
---

# ForgePlot

`ForgePlot` 是 **ForgePlot** 子系统的公共组件。本页由当前 `ui/slintforge.slint` 契约生成，只记录直接声明的公共接口，不改变运行时行为。

<ApiScreenshot slug="forge-plot" name="ForgePlot" />

## 用途与适用场景

- **适用场景：** 在 ForgePlot 页面中组合 `ForgePlot` 所代表的交互或展示能力。
- **继承关系：**`ForgePlot` → `Rectangle`。
- **源码位置：**`ui/charts/plot.slint:9`。
- **状态所有权：**`in` 由宿主持有，`in-out` 由宿主与组件共享，`out` 是组件只读输出。

## 导入方式

```qml
import { ForgePlot } from "slintforge.slint";
```

## 属性

| 属性 | 方向 | 类型 | 默认值 | 约束 | 状态所有权 |
| --- | --- | --- | --- | --- | --- |
| `axes` | `in-out` | `[ForgePlotAxis]` | `[{id: 0, side: bottom, precision: 2}, {id: 1, side: left, precision: 2}]` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `panels` | `in` | `[ForgePlotPanel]` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `series` | `in-out` | `[ForgePlotSeries]` | `类型默认值` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `color-maps` | `in-out` | `[ForgePlotColorMap]` | `类型默认值` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `items` | `in-out` | `[ForgePlotItem]` | `类型默认值` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `title` | `in` | `string` | `类型默认值` | 由类型与默认表达式约束 | 宿主输入 |
| `enabled` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `show-legend` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `legend-position` | `in` | `ForgePlotLegendPosition` | `bottom` | 由类型与默认表达式约束 | 宿主输入 |
| `legend-placement` | `in` | `ForgePlotPlacement` | `inside` | 由类型与默认表达式约束 | 宿主输入 |
| `legend-style` | `in` | `ForgePlotLegendStyle` | `plain` | 由类型与默认表达式约束 | 宿主输入 |
| `legend-latest-value` | `in` | `bool` | `false` | 由类型与默认表达式约束 | 宿主输入 |
| `legend-precision` | `in` | `int` | `2` | 由类型与默认表达式约束 | 宿主输入 |
| `x-window-mode` | `in` | `ForgePlotXWindowMode` | `fit` | 由类型与默认表达式约束 | 宿主输入 |
| `x-window-axis` | `in` | `int` | `0` | 由类型与默认表达式约束 | 宿主输入 |
| `x-window-size` | `in` | `float` | `10` | 由类型与默认表达式约束 | 宿主输入 |
| `measure-enabled` | `in-out` | `bool` | `false` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `cursor-a` | `in-out` | `float` | `0` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `cursor-b` | `in-out` | `float` | `1` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `measurement-series` | `in` | `int` | `0` | 由类型与默认表达式约束 | 宿主输入 |
| `measurement-position` | `in` | `ForgePlotLegendPosition` | `bottom` | 由类型与默认表达式约束 | 宿主输入 |
| `measurement-placement` | `in` | `ForgePlotPlacement` | `outside` | 由类型与默认表达式约束 | 宿主输入 |
| `measurement-width` | `in` | `length` | `0px` | 由类型与默认表达式约束 | 宿主输入 |
| `measurement-height` | `in` | `length` | `0px` | 由类型与默认表达式约束 | 宿主输入 |
| `measurement-follow-view` | `in` | `bool` | `root.x-window-mode != fit` | 由类型与默认表达式约束 | 宿主输入 |
| `cursor-a-pixel` | `out` | `float` | `ForgePlotServices.coordinate(root.scene.token, root.scene.measurement-axis, root.cursor-a, false)` | 由类型与默认表达式约束 | 组件只读输出 |
| `cursor-b-pixel` | `out` | `float` | `ForgePlotServices.coordinate(root.scene.token, root.scene.measurement-axis, root.cursor-b, false)` | 由类型与默认表达式约束 | 组件只读输出 |
| `measurement-a` | `out` | `ForgePlotHit` | `root.scene.status < 2 ? ForgePlotServices.measure(root.scene.token, root.measurement-series, root.cursor-a) : {kind: -1}` | 由类型与默认表达式约束 | 组件只读输出 |
| `measurement-b` | `out` | `ForgePlotHit` | `root.scene.status < 2 ? ForgePlotServices.measure(root.scene.token, root.measurement-series, root.cursor-b) : {kind: -1}` | 由类型与默认表达式约束 | 组件只读输出 |
| `show-crosshair` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `show-tooltip` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `adaptive-sampling` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `antialias` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `max-points-per-pixel` | `in` | `int` | `4` | 由类型与默认表达式约束 | 宿主输入 |
| `allow-pan` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `allow-zoom` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `allow-selection` | `in` | `bool` | `true` | 由类型与默认表达式约束 | 宿主输入 |
| `directions` | `in` | `int` | `3` | 由类型与默认表达式约束 | 宿主输入 |
| `frame-interval` | `in` | `int` | `16` | 由类型与默认表达式约束 | 宿主输入 |
| `plot-background` | `in` | `color` | `Palette.background` | 由类型与默认表达式约束 | 宿主输入 |
| `selection` | `in-out` | `[ForgePlotSelection]` | `类型默认值` | 由类型与默认表达式约束 | 宿主与组件共享 |
| `hovered` | `out` | `ForgePlotHit` | `root.hover` | 由类型与默认表达式约束 | 组件只读输出 |
| `scene` | `out` | `ForgePlotScene` | `root.connected && root.token > 0 ? ForgePlotServices.prepare(root.token, root.axes, root.panels, root.series, root.color-maps, root.items, root.selection, root.options) : {token: 0, status: 3, error: @tr("尚未绑定图表服务")}` | 由类型与默认表达式约束 | 组件只读输出 |

## 回调、公开函数与插槽

| 类型 | 名称 | 签名 | 契约 |
| --- | --- | --- | --- |
| 回调 | `point-selected` | `(ForgePlotHit)` | 由组件触发，宿主负责处理。 |
| 回调 | `item-selected` | `(ForgePlotHit)` | 由组件触发，宿主负责处理。 |
| 回调 | `axis-clicked` | `(int)` | 由组件触发，宿主负责处理。 |
| 回调 | `legend-clicked` | `(int)` | 由组件触发，宿主负责处理。 |
| 回调 | `selection-changed` | `([ForgePlotSelection])` | 由组件触发，宿主负责处理。 |
| 回调 | `range-changed` | `([ForgePlotRange])` | 由组件触发，宿主负责处理。 |
| 回调 | `coordinate-moved` | `(int, float, float)` | 由组件触发，宿主负责处理。 |
| 回调 | `export-finished` | `(ForgePlotExportResult)` | 由组件触发，宿主负责处理。 |
| 公开函数 | `connect` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `reset-measurement-cursors` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `refresh` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `refresh-immediate` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `append-points` | `(id: int, points: [ForgePlotPoint]) -> bool` | 由宿主调用，组件同步处理。 |
| 公开函数 | `append-point` | `(id: int, point: ForgePlotPoint) -> bool` | 由宿主调用，组件同步处理。 |
| 公开函数 | `flush` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `measure-at` | `(id: int, x: float) -> ForgePlotHit` | 由宿主调用，组件同步处理。 |
| 公开函数 | `scroll-history` | `(position: float)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `follow-latest` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `release` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `reset-view` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `zoom-at` | `(x: float, y: float, factor: float, panel: int)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `pan-by` | `(dx: float, dy: float, panel: int)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `clear-selection` | `()` | 由宿主调用，组件同步处理。 |
| 公开函数 | `select-point` | `(id: int, index: int, additive: bool)` | 由宿主调用，组件同步处理。 |
| 公开函数 | `export-file` | `(path: string, format: string) -> ForgePlotExportResult` | 由宿主调用，组件同步处理。 |

## 键盘、鼠标、可访问性、本地化、主题与平台限制

- **键盘：** 源码包含焦点或按键事件处理。
- **鼠标/触控：** 源码包含指针或触控处理。
- **可访问性：** 未检测到显式可访问性元数据；应在组合页面提供标签并做整页验证。
- **本地化：** 组件包含翻译文本或语言输入。
- **主题：** 使用 SlintForge 主题或排版令牌。
- **平台限制：** Slint 声明中未检测到平台条件；原生窗口、剪贴板、文件和截图行为仍取决于 Slint 后端与宿主接入。

## 可编译最小示例

<<< ../../../../examples/api-components/forge-plot.slint{qml}

## 宿主接入

本组件引用 `ForgePlotServices`。先初始化运行库，再绑定由 Slint 生成的全局服务类型：

```rust
slintforge::runtime::initialize()?;
slintforge::bind_plot_services!(ui, /* generated Slint service/result types */);
```

宏的类型参数应替换为消费工程 `slint!`/`include_modules!` 生成的类型。

```cpp
#include <ForgePlotBindings.hpp>

forge_bind_plot_services(ui);
```


## 相关 API

- [公共类型索引](/types/)
- [Rust SDK](/sdk/rust)
- [C++ SDK](/sdk/cpp)
- [English page](/en/components/plot/forge-plot)
