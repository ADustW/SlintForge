---
title: ForgePlot
description: ForgePlot public API, behavior, example, and host integration.
---

# ForgePlot

`ForgePlot` is a public component in the **ForgePlot** subsystem. This page is generated from the current `ui/slintforge.slint` contract and records the directly declared public surface without changing runtime behavior.

<ApiScreenshot slug="forge-plot" name="ForgePlot" />

## Purpose and usage

- **Use when:** composing a forgeplot screen that needs the behavior represented by `ForgePlot`.
- **Inheritance:** `ForgePlot` → `Rectangle`.
- **Source:** `ui/charts/plot.slint:9`.
- **State ownership:** `in` is host-owned, `in-out` is shared, and `out` is component-owned read-only state.

## Import

```qml
import { ForgePlot } from "slintforge.slint";
```

## Properties

| Property | Direction | Type | Default | Constraint | State owner |
| --- | --- | --- | --- | --- | --- |
| `axes` | `in-out` | `[ForgePlotAxis]` | `[{id: 0, side: bottom, precision: 2}, {id: 1, side: left, precision: 2}]` | Constrained by its type and default expression | Shared by host and component |
| `panels` | `in` | `[ForgePlotPanel]` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `series` | `in-out` | `[ForgePlotSeries]` | `Type default` | Constrained by its type and default expression | Shared by host and component |
| `color-maps` | `in-out` | `[ForgePlotColorMap]` | `Type default` | Constrained by its type and default expression | Shared by host and component |
| `items` | `in-out` | `[ForgePlotItem]` | `Type default` | Constrained by its type and default expression | Shared by host and component |
| `title` | `in` | `string` | `Type default` | Constrained by its type and default expression | Host-owned input |
| `enabled` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-legend` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `legend-position` | `in` | `ForgePlotLegendPosition` | `bottom` | Constrained by its type and default expression | Host-owned input |
| `legend-placement` | `in` | `ForgePlotPlacement` | `inside` | Constrained by its type and default expression | Host-owned input |
| `legend-style` | `in` | `ForgePlotLegendStyle` | `plain` | Constrained by its type and default expression | Host-owned input |
| `legend-latest-value` | `in` | `bool` | `false` | Constrained by its type and default expression | Host-owned input |
| `legend-precision` | `in` | `int` | `2` | Constrained by its type and default expression | Host-owned input |
| `x-window-mode` | `in` | `ForgePlotXWindowMode` | `fit` | Constrained by its type and default expression | Host-owned input |
| `x-window-axis` | `in` | `int` | `0` | Constrained by its type and default expression | Host-owned input |
| `x-window-size` | `in` | `float` | `10` | Constrained by its type and default expression | Host-owned input |
| `measure-enabled` | `in-out` | `bool` | `false` | Constrained by its type and default expression | Shared by host and component |
| `cursor-a` | `in-out` | `float` | `0` | Constrained by its type and default expression | Shared by host and component |
| `cursor-b` | `in-out` | `float` | `1` | Constrained by its type and default expression | Shared by host and component |
| `measurement-series` | `in` | `int` | `0` | Constrained by its type and default expression | Host-owned input |
| `measurement-position` | `in` | `ForgePlotLegendPosition` | `bottom` | Constrained by its type and default expression | Host-owned input |
| `measurement-placement` | `in` | `ForgePlotPlacement` | `outside` | Constrained by its type and default expression | Host-owned input |
| `measurement-width` | `in` | `length` | `0px` | Constrained by its type and default expression | Host-owned input |
| `measurement-height` | `in` | `length` | `0px` | Constrained by its type and default expression | Host-owned input |
| `measurement-follow-view` | `in` | `bool` | `root.x-window-mode != fit` | Constrained by its type and default expression | Host-owned input |
| `cursor-a-pixel` | `out` | `float` | `ForgePlotServices.coordinate(root.scene.token, root.scene.measurement-axis, root.cursor-a, false)` | Constrained by its type and default expression | Read-only component output |
| `cursor-b-pixel` | `out` | `float` | `ForgePlotServices.coordinate(root.scene.token, root.scene.measurement-axis, root.cursor-b, false)` | Constrained by its type and default expression | Read-only component output |
| `measurement-a` | `out` | `ForgePlotHit` | `root.scene.status < 2 ? ForgePlotServices.measure(root.scene.token, root.measurement-series, root.cursor-a) : {kind: -1}` | Constrained by its type and default expression | Read-only component output |
| `measurement-b` | `out` | `ForgePlotHit` | `root.scene.status < 2 ? ForgePlotServices.measure(root.scene.token, root.measurement-series, root.cursor-b) : {kind: -1}` | Constrained by its type and default expression | Read-only component output |
| `show-crosshair` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `show-tooltip` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `adaptive-sampling` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `antialias` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `max-points-per-pixel` | `in` | `int` | `4` | Constrained by its type and default expression | Host-owned input |
| `allow-pan` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `allow-zoom` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `allow-selection` | `in` | `bool` | `true` | Constrained by its type and default expression | Host-owned input |
| `directions` | `in` | `int` | `3` | Constrained by its type and default expression | Host-owned input |
| `frame-interval` | `in` | `int` | `16` | Constrained by its type and default expression | Host-owned input |
| `plot-background` | `in` | `color` | `Palette.background` | Constrained by its type and default expression | Host-owned input |
| `selection` | `in-out` | `[ForgePlotSelection]` | `Type default` | Constrained by its type and default expression | Shared by host and component |
| `hovered` | `out` | `ForgePlotHit` | `root.hover` | Constrained by its type and default expression | Read-only component output |
| `scene` | `out` | `ForgePlotScene` | `root.connected && root.token > 0 ? ForgePlotServices.prepare(root.token, root.axes, root.panels, root.series, root.color-maps, root.items, root.selection, root.options) : {token: 0, status: 3, error: @tr("尚未绑定图表服务")}` | Constrained by its type and default expression | Read-only component output |

## Callbacks, public functions, and slots

| Kind | Name | Signature | Contract |
| --- | --- | --- | --- |
| Callback | `point-selected` | `(ForgePlotHit)` | Emitted by the component and handled by the host. |
| Callback | `item-selected` | `(ForgePlotHit)` | Emitted by the component and handled by the host. |
| Callback | `axis-clicked` | `(int)` | Emitted by the component and handled by the host. |
| Callback | `legend-clicked` | `(int)` | Emitted by the component and handled by the host. |
| Callback | `selection-changed` | `([ForgePlotSelection])` | Emitted by the component and handled by the host. |
| Callback | `range-changed` | `([ForgePlotRange])` | Emitted by the component and handled by the host. |
| Callback | `coordinate-moved` | `(int, float, float)` | Emitted by the component and handled by the host. |
| Callback | `export-finished` | `(ForgePlotExportResult)` | Emitted by the component and handled by the host. |
| Public function | `connect` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `reset-measurement-cursors` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `refresh` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `refresh-immediate` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `append-points` | `(id: int, points: [ForgePlotPoint]) -> bool` | Called by the host and handled synchronously by the component. |
| Public function | `append-point` | `(id: int, point: ForgePlotPoint) -> bool` | Called by the host and handled synchronously by the component. |
| Public function | `flush` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `measure-at` | `(id: int, x: float) -> ForgePlotHit` | Called by the host and handled synchronously by the component. |
| Public function | `scroll-history` | `(position: float)` | Called by the host and handled synchronously by the component. |
| Public function | `follow-latest` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `release` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `reset-view` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `zoom-at` | `(x: float, y: float, factor: float, panel: int)` | Called by the host and handled synchronously by the component. |
| Public function | `pan-by` | `(dx: float, dy: float, panel: int)` | Called by the host and handled synchronously by the component. |
| Public function | `clear-selection` | `()` | Called by the host and handled synchronously by the component. |
| Public function | `select-point` | `(id: int, index: int, additive: bool)` | Called by the host and handled synchronously by the component. |
| Public function | `export-file` | `(path: string, format: string) -> ForgePlotExportResult` | Called by the host and handled synchronously by the component. |

## Interaction, accessibility, localization, and platform notes

- **Keyboard:** The implementation contains focus or key-event handling.
- **Pointer:** The implementation contains pointer/touch handling.
- **Accessibility:** No explicit accessible metadata was detected; provide surrounding labels and validate the composed screen.
- **Localization:** The component contains translated text or a language input.
- **Theme:** Uses SlintForge theme/typography tokens.
- **Platform:** No platform-specific conditional was detected in the Slint declaration. Native window, clipboard, file, and capture behavior still depends on the selected Slint backend and host integration.

## Minimal compilable example

<<< ../../../../../examples/api-components/forge-plot.slint{qml}

## Host integration

This component references `ForgePlotServices`. Initialize the runtime and bind the generated Slint global service type:

```rust
slintforge::runtime::initialize()?;
slintforge::bind_plot_services!(ui, /* generated Slint service/result types */);
```

Replace macro type arguments with the types generated by the consumer's `slint!`/`include_modules!` call.

```cpp
#include <ForgePlotBindings.hpp>

forge_bind_plot_services(ui);
```


## Related API

- [Shared type index](/en/types/)
- [Rust SDK](/en/sdk/rust)
- [C++ SDK](/en/sdk/cpp)
- [Chinese page](/components/plot/forge-plot)
