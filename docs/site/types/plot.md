# Plot 公共类型

本子系统的公共结构体、枚举和全局对象。

| 符号 | 类型 | 成员 / 枚举值 | 源码 |
| --- | --- | --- | --- |
| `ForgePlotKind` | 枚举 | `graph`, `curve`, `bars`, `statistical-box`, `candlestick`, `ohlc`, `polar`, `bubble`, `pie`, `polar-area`, `radar` | `ui/types/plot.slint:2` |
| `ForgePlotLineStyle` | 枚举 | `auto`, `none`, `line`, `step-left`, `step-right`, `step-center`, `impulse`, `smooth` | `ui/types/plot.slint:3` |
| `ForgePlotScatterStyle` | 枚举 | `none`, `circle`, `disc`, `square`, `diamond`, `cross`, `cross-plus`, `triangle`, `plus`, `star` | `ui/types/plot.slint:4` |
| `ForgePlotAxisSide` | 枚举 | `bottom`, `top`, `left`, `right` | `ui/types/plot.slint:5` |
| `ForgePlotScale` | 枚举 | `linear`, `logarithmic` | `ui/types/plot.slint:6` |
| `ForgePlotTicker` | 枚举 | `numeric`, `text`, `date-time`, `duration`, `pi` | `ui/types/plot.slint:7` |
| `ForgePlotLegendPosition` | 枚举 | `bottom`, `top`, `left`, `right`, `top-right`, `bottom-right`, `top-left`, `bottom-left` | `ui/types/plot.slint:8` |
| `ForgePlotLegendStyle` | 枚举 | `plain`, `framed`, `compact` | `ui/types/plot.slint:10` |
| `ForgePlotPlacement` | 枚举 | `inside`, `outside` | `ui/types/plot.slint:9` |
| `ForgePlotXWindowMode` | 枚举 | `fit`, `unlimited`, `fixed` | `ui/types/plot.slint:11` |
| `ForgePlotItemKind` | 枚举 | `text`, `line`, `arrow`, `rectangle`, `ellipse`, `curve`, `bracket`, `tracer`, `image` | `ui/types/plot.slint:12` |
| `ForgePlotCoordinates` | 枚举 | `data`, `ratio`, `pixel` | `ui/types/plot.slint:13` |
| `ForgePlotPoint` | 结构体 | `x: float`, `y: float`, `low: float`, `high: float`, `open: float`, `close: float`, `q1: float`, `q3: float`, `error-x-minus: float`, `error-x-plus: float`, `error-y-minus: float`, `error-y-plus: float`, `label: string`, `outliers: [float]` | `ui/types/plot.slint:14` |
| `ForgePlotPointStyle` | 结构体 | `index: int`, `hidden: bool`, `size: float`, `color: color` | `ui/types/plot.slint:28` |
| `ForgePlotTick` | 结构体 | `value: float`, `label: string` | `ui/types/plot.slint:19` |
| `ForgePlotAxis` | 结构体 | `id: int`, `panel: int`, `side: ForgePlotAxisSide`, `scale: ForgePlotScale`, `ticker: ForgePlotTicker`, `manual-range: bool`, `reversed: bool`, `hidden: bool`, `hide-grid: bool`, `precision: int`, `minimum: float`, `maximum: float`, `tick-step: float`, `offset: float`, `label: string`, `format: string`, `time-origin: string`, `ticks: [ForgePlotTick]` | `ui/types/plot.slint:20` |
| `ForgePlotPanel` | 结构体 | `id: int`, `x: float`, `y: float`, `width: float`, `height: float`, `title: string`, `categories: [string]` | `ui/types/plot.slint:26` |
| `ForgePlotSeries` | 结构体 | `id: int`, `kind: ForgePlotKind`, `x-axis: int`, `y-axis: int`, `line-style: ForgePlotLineStyle`, `scatter-style: ForgePlotScatterStyle`, `hidden: bool`, `unselectable: bool`, `stacked: bool`, `stack-group: int`, `channel-fill: bool`, `fill-target: int`, `layer: int`, `sorted: bool`, `color: color`, `fill-color: color`, `line-width: float`, `point-size: float`, `bar-width: float`, `baseline: float`, `name: string`, `points: [ForgePlotPoint]`, `inner-radius: float`, `point-styles: [ForgePlotPointStyle]` | `ui/types/plot.slint:29` |
| `ForgePlotColorMap` | 结构体 | `id: int`, `panel: int`, `x-axis: int`, `y-axis: int`, `columns: int`, `rows: int`, `palette: int`, `manual-range: bool`, `logarithmic: bool`, `hidden: bool`, `interpolate: bool`, `hide-color-scale: bool`, `layer: int`, `x-min: float`, `x-max: float`, `y-min: float`, `y-max: float`, `z-min: float`, `z-max: float`, `name: string`, `values: [float]` | `ui/types/plot.slint:37` |
| `ForgePlotItem` | 结构体 | `id: int`, `kind: ForgePlotItemKind`, `panel: int`, `x-axis: int`, `y-axis: int`, `coordinates: ForgePlotCoordinates`, `anchored: bool`, `anchor: int`, `anchor-part: int`, `unclipped: bool`, `layer: int`, `selectable: bool`, `x: float`, `y: float`, `x2: float`, `y2: float`, `x3: float`, `y3: float`, `x4: float`, `y4: float`, `width: float`, `size: float`, `color: color`, `fill-color: color`, `text: string`, `image-data: string` | `ui/types/plot.slint:43` |
| `ForgePlotOptions` | 结构体 | `width: float`, `height: float`, `font-size: float`, `background: color`, `foreground: color`, `grid-color: color`, `selection-color: color`, `hide-legend: bool`, `disable-adaptive: bool`, `disable-antialias: bool`, `max-points-per-pixel: int`, `legend-position: ForgePlotLegendPosition`, `legend-style: ForgePlotLegendStyle`, `legend-latest-value: bool`, `legend-precision: int`, `legend-placement: ForgePlotPlacement`, `measurement-enabled: bool`, `measurement-position: ForgePlotLegendPosition`, `measurement-placement: ForgePlotPlacement`, `measurement-series: int`, `measurement-width: float`, `measurement-height: float`, `x-window-mode: ForgePlotXWindowMode`, `x-window-axis: int`, `x-window-size: float`, `title: string`, `font-family: string`, `revision: int`, `stream-revision: int`, `render-scale: float` | `ui/types/plot.slint:49` |
| `ForgePlotRange` | 结构体 | `id: int`, `scale: int`, `reversed: bool`, `minimum: float`, `maximum: float` | `ui/types/plot.slint:59` |
| `ForgePlotRect` | 结构体 | `id: int`, `x: float`, `y: float`, `width: float`, `height: float` | `ui/types/plot.slint:60` |
| `ForgePlotHit` | 结构体 | `kind: int`, `id: int`, `index: int`, `panel: int`, `x: float`, `y: float`, `value: float`, `pixel-x: float`, `pixel-y: float`, `distance: float`, `label: string` | `ui/types/plot.slint:61` |
| `ForgePlotSelection` | 结构体 | `id: int`, `first: int`, `last: int` | `ui/types/plot.slint:65` |
| `ForgePlotScene` | 结构体 | `token: int`, `status: int`, `image: image`, `svg: string`, `error: string`, `input-points: int`, `rendered-points: int`, `path-count: int`, `svg-bytes: int`, `ranges: [ForgePlotRange]`, `domains: [ForgePlotRange]`, `rects: [ForgePlotRect]`, `window-range: ForgePlotRange`, `history-range: ForgePlotRange`, `legend-rect: ForgePlotRect`, `measurement-rect: ForgePlotRect`, `measurement-axis: int`, `measurement-range: ForgePlotRange` | `ui/types/plot.slint:66` |
| `ForgePlotExportResult` | 结构体 | `ok: bool`, `error: string` | `ui/types/plot.slint:74` |
| `ForgePlotServices` | 全局对象 | `connected`, `allocate`, `release`, `append`, `select-rect`, `merge-selection`, `change-view`, `scroll-view`, `toggle-series`, `toggle-point`, `export` | `ui/types/plot.slint:75` |
