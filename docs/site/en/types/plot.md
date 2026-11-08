# Plot Shared Types

Public structs, enums, and globals for this subsystem.

| Symbol | Kind | Members / values | Source |
| --- | --- | --- | --- |
| `ForgePlotKind` | Enum | `graph`, `curve`, `bars`, `statistical-box`, `candlestick`, `ohlc`, `polar`, `bubble`, `pie`, `polar-area`, `radar` | `ui/types/plot.slint:2` |
| `ForgePlotLineStyle` | Enum | `auto`, `none`, `line`, `step-left`, `step-right`, `step-center`, `impulse`, `smooth` | `ui/types/plot.slint:3` |
| `ForgePlotScatterStyle` | Enum | `none`, `circle`, `disc`, `square`, `diamond`, `cross`, `cross-plus`, `triangle`, `plus`, `star` | `ui/types/plot.slint:4` |
| `ForgePlotAxisSide` | Enum | `bottom`, `top`, `left`, `right` | `ui/types/plot.slint:5` |
| `ForgePlotScale` | Enum | `linear`, `logarithmic` | `ui/types/plot.slint:6` |
| `ForgePlotTicker` | Enum | `numeric`, `text`, `date-time`, `duration`, `pi` | `ui/types/plot.slint:7` |
| `ForgePlotLegendPosition` | Enum | `bottom`, `top`, `left`, `right`, `top-right`, `bottom-right`, `top-left`, `bottom-left` | `ui/types/plot.slint:8` |
| `ForgePlotLegendStyle` | Enum | `plain`, `framed`, `compact` | `ui/types/plot.slint:10` |
| `ForgePlotPlacement` | Enum | `inside`, `outside` | `ui/types/plot.slint:9` |
| `ForgePlotXWindowMode` | Enum | `fit`, `unlimited`, `fixed` | `ui/types/plot.slint:11` |
| `ForgePlotItemKind` | Enum | `text`, `line`, `arrow`, `rectangle`, `ellipse`, `curve`, `bracket`, `tracer`, `image` | `ui/types/plot.slint:12` |
| `ForgePlotCoordinates` | Enum | `data`, `ratio`, `pixel` | `ui/types/plot.slint:13` |
| `ForgePlotPoint` | Struct | `x: float`, `y: float`, `low: float`, `high: float`, `open: float`, `close: float`, `q1: float`, `q3: float`, `error-x-minus: float`, `error-x-plus: float`, `error-y-minus: float`, `error-y-plus: float`, `label: string`, `outliers: [float]` | `ui/types/plot.slint:14` |
| `ForgePlotPointStyle` | Struct | `index: int`, `hidden: bool`, `size: float`, `color: color` | `ui/types/plot.slint:28` |
| `ForgePlotTick` | Struct | `value: float`, `label: string` | `ui/types/plot.slint:19` |
| `ForgePlotAxis` | Struct | `id: int`, `panel: int`, `side: ForgePlotAxisSide`, `scale: ForgePlotScale`, `ticker: ForgePlotTicker`, `manual-range: bool`, `reversed: bool`, `hidden: bool`, `hide-grid: bool`, `precision: int`, `minimum: float`, `maximum: float`, `tick-step: float`, `offset: float`, `label: string`, `format: string`, `time-origin: string`, `ticks: [ForgePlotTick]` | `ui/types/plot.slint:20` |
| `ForgePlotPanel` | Struct | `id: int`, `x: float`, `y: float`, `width: float`, `height: float`, `title: string`, `categories: [string]` | `ui/types/plot.slint:26` |
| `ForgePlotSeries` | Struct | `id: int`, `kind: ForgePlotKind`, `x-axis: int`, `y-axis: int`, `line-style: ForgePlotLineStyle`, `scatter-style: ForgePlotScatterStyle`, `hidden: bool`, `unselectable: bool`, `stacked: bool`, `stack-group: int`, `channel-fill: bool`, `fill-target: int`, `layer: int`, `sorted: bool`, `color: color`, `fill-color: color`, `line-width: float`, `point-size: float`, `bar-width: float`, `baseline: float`, `name: string`, `points: [ForgePlotPoint]`, `inner-radius: float`, `point-styles: [ForgePlotPointStyle]` | `ui/types/plot.slint:29` |
| `ForgePlotColorMap` | Struct | `id: int`, `panel: int`, `x-axis: int`, `y-axis: int`, `columns: int`, `rows: int`, `palette: int`, `manual-range: bool`, `logarithmic: bool`, `hidden: bool`, `interpolate: bool`, `hide-color-scale: bool`, `layer: int`, `x-min: float`, `x-max: float`, `y-min: float`, `y-max: float`, `z-min: float`, `z-max: float`, `name: string`, `values: [float]` | `ui/types/plot.slint:37` |
| `ForgePlotItem` | Struct | `id: int`, `kind: ForgePlotItemKind`, `panel: int`, `x-axis: int`, `y-axis: int`, `coordinates: ForgePlotCoordinates`, `anchored: bool`, `anchor: int`, `anchor-part: int`, `unclipped: bool`, `layer: int`, `selectable: bool`, `x: float`, `y: float`, `x2: float`, `y2: float`, `x3: float`, `y3: float`, `x4: float`, `y4: float`, `width: float`, `size: float`, `color: color`, `fill-color: color`, `text: string`, `image-data: string` | `ui/types/plot.slint:43` |
| `ForgePlotOptions` | Struct | `width: float`, `height: float`, `font-size: float`, `background: color`, `foreground: color`, `grid-color: color`, `selection-color: color`, `hide-legend: bool`, `disable-adaptive: bool`, `disable-antialias: bool`, `max-points-per-pixel: int`, `legend-position: ForgePlotLegendPosition`, `legend-style: ForgePlotLegendStyle`, `legend-latest-value: bool`, `legend-precision: int`, `legend-placement: ForgePlotPlacement`, `measurement-enabled: bool`, `measurement-position: ForgePlotLegendPosition`, `measurement-placement: ForgePlotPlacement`, `measurement-series: int`, `measurement-width: float`, `measurement-height: float`, `x-window-mode: ForgePlotXWindowMode`, `x-window-axis: int`, `x-window-size: float`, `title: string`, `font-family: string`, `revision: int`, `stream-revision: int`, `render-scale: float` | `ui/types/plot.slint:49` |
| `ForgePlotRange` | Struct | `id: int`, `scale: int`, `reversed: bool`, `minimum: float`, `maximum: float` | `ui/types/plot.slint:59` |
| `ForgePlotRect` | Struct | `id: int`, `x: float`, `y: float`, `width: float`, `height: float` | `ui/types/plot.slint:60` |
| `ForgePlotHit` | Struct | `kind: int`, `id: int`, `index: int`, `panel: int`, `x: float`, `y: float`, `value: float`, `pixel-x: float`, `pixel-y: float`, `distance: float`, `label: string` | `ui/types/plot.slint:61` |
| `ForgePlotSelection` | Struct | `id: int`, `first: int`, `last: int` | `ui/types/plot.slint:65` |
| `ForgePlotScene` | Struct | `token: int`, `status: int`, `image: image`, `svg: string`, `error: string`, `input-points: int`, `rendered-points: int`, `path-count: int`, `svg-bytes: int`, `ranges: [ForgePlotRange]`, `domains: [ForgePlotRange]`, `rects: [ForgePlotRect]`, `window-range: ForgePlotRange`, `history-range: ForgePlotRange`, `legend-rect: ForgePlotRect`, `measurement-rect: ForgePlotRect`, `measurement-axis: int`, `measurement-range: ForgePlotRange` | `ui/types/plot.slint:66` |
| `ForgePlotExportResult` | Struct | `ok: bool`, `error: string` | `ui/types/plot.slint:74` |
| `ForgePlotServices` | Global | `connected`, `allocate`, `release`, `append`, `select-rect`, `merge-selection`, `change-view`, `scroll-view`, `toggle-series`, `toggle-point`, `export` | `ui/types/plot.slint:75` |
