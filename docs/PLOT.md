# 专业二维图表 ForgePlot

> **兼容提示：**本页继续保留 ForgePlot 专题说明。逐组件中英文 API 见 [中文 API 站点源码](site/index.md) 与 [English API source](site/en/index.md)。

`ForgePlot` 是组件库的专业绘图入口，参考 QCustomPlot 的二维绘图能力。共享 `.slint` 契约、Rust/C++ 薄适配层和共用绘图引擎支持相同的数据、坐标、命中和导出。七类基础图表也使用此入口，新增图形和 ABI v2 迁移见 [统一图表](CHARTS.md)。

## 最小 Slint 窗口

```slint
import { ForgePlot } from "@slintforge";
export { ForgePlotServices, ForgePlotScene, ForgePlotAxis, ForgePlotRange,
    ForgePlotRect, ForgePlotHit, ForgePlotSelection, ForgePlotExportResult } from "@slintforge";
export component App inherits Window {
    width: 800px; height: 500px;
    ForgePlot {
        title: "测量数据";
        series: [{id: 0, name: "电压", points: [{x: 0, y: 1}, {x: 1, y: 3}, {x: 2, y: 2}]}];
    }
}
```

默认提供 X 轴 0、Y 轴 1、自动范围、线条、图例、拖动、缩放、选择和提示。系列缺省的 `x-axis: 0, y-axis: 0` 自动使用轴 0/1。自定义多轴时明确指定两个轴的 ID。每个系列 ID 须唯一。

Gallery 的“图表 → ForgePlot（该菜单第一项）”直接演示十种场景、实时采样和导出；首页“最近添加的示例”也提供 ForgePlot 入口，导航搜索可直接输入 `ForgePlot`。参考曲线的空值会切断系列间填充；隐藏参考曲线时取消对应填充。

Rust 在窗口创建后、运行前绑定一次：

```rust
slintforge::bind_plot_services!(ui, ForgePlotServices, ForgePlotScene, ForgePlotAxis,
    ForgePlotRange, ForgePlotRect, ForgePlotHit, ForgePlotSelection, ForgePlotExportResult);
```

C++ 引入生成的窗口头文件和 `ForgePlot.hpp`，再绑定：

```cpp
forge_bind_plot_services<ForgePlotServices, ForgePlotScene, ForgePlotAxis,
    ForgePlotRange, ForgePlotRect, ForgePlotHit, ForgePlotSelection, ForgePlotExportResult>(ui);
```

服务未绑定时组件显示明确状态。每个组件获得独立 token，多个图表不会共用数据。动态删除组件前调用 `release()`；再次使用可以调用 `connect()`。窗口关闭时宿主服务自动释放持有的模型和场景。

## 图形与数据

`ForgePlotSeries.kind` 支持 `graph`、`curve`、`bars`、`statistical-box`、`candlestick`、`ohlc`、`polar`。

| 图形 | 点字段与选项 |
|---|---|
| 折线、散点 | `x/y`；`line-style` 为 `auto/none/line/step-left/step-right/step-center/impulse` |
| 点符号 | `scatter-style` 为 `none/circle/disc/square/diamond/cross/cross-plus/triangle/plus/star`；`point-size` 单位为像素 |
| 参数曲线 | 按输入顺序连接，可表示闭环；折线则在每个有限数据段内按 X 稳定排序 |
| 面积 | `fill-color` 与 `baseline`；系列间填充使用 `channel-fill: true, fill-target: 目标ID` |
| 柱状图 | `y` 是相对基线的增量；同轴系列默认分组；`stacked: true, stack-group: ID` 按正负分别堆叠 |
| 统计箱 | `low ≤ q1 ≤ y(中位数) ≤ q3 ≤ high`；`outliers` 为离群值数组 |
| 蜡烛/OHLC | `low/high/open/close`；`y` 不参与金融图形自动范围；`bar-width` 为 X 坐标单位 |
| 极坐标 | `x` 是角度，缺省自动范围为 0–360；`y` 是半径；使用圆形网格与裁剪 |

点还可设置 `error-x-minus/plus`、`error-y-minus/plus` 和 `label`。非有限 X/Y 形成断点，非法统计/金融点不绘制；原始索引始终对应输入模型。极坐标与笛卡尔图形放在不同绘图区。

`hidden` 控制系列显隐，`unselectable` 禁止数据选择，`layer` 控制叠放顺序。缺省颜色使用调色板，缺省线宽 2px、符号大小 6px、柱宽 0.8 个 X 单位。

## 坐标轴、布局与刻度

`axes` 使用稳定 ID，通过 `panel` 归属绘图区；`side` 支持 `bottom/top/left/right`。同侧多轴自动错开，`offset` 可增加轴间距。

- 缺省自动范围覆盖误差棒、离群值、金融高低值、正负堆叠总和及色图坐标。
- `manual-range: true` 配合 `minimum/maximum` 固定范围；`reversed` 反向。
- `scale: logarithmic` 支持全正或全负范围，同一轴不能跨零。
- `ticker` 支持 `numeric/text/date-time/duration/pi`。文本刻度使用 `ticks: [{value:…,label:…}]`；`tick-step` 和 `precision` 控制刻度。
- 日期时间以 UTC 格式化，`format` 使用 `strftime`，缺省 `%H:%M:%S`。Slint 数值为 f32，秒级时间序列应使用相对秒数，并以十进制字符串 `time-origin` 保存完整 Unix 秒基准，例如 `time-origin: "1790985600"`。原生引擎和宿主数据 API 使用 f64。

`panels` 使用整个图表的 0–1 比例矩形，字段为 `id/x/y/width/height/title`。每个绘图区单独布局、命中和裁剪；系列使用该区域的轴 ID。未提供绘图区时使用区域 0。

## 二维色图与图元

`color-maps` 指定 `columns/rows/values/x-min/x-max/y-min/y-max` 以及轴和区域 ID。数据逐行存储，第一行对应较小 Y。`palette` 的 0–3 分别是紫绿黄、热度、灰度、蓝白红；支持自动或手动 Z 范围、正值对数色阶、插值及色标。NaN/无穷或对数色阶中的非正值透明。

`items` 支持 `text/line/arrow/rectangle/ellipse/curve/bracket/tracer/image`。`coordinates` 为 `data/ratio/pixel`，比例坐标相对绘图区、像素坐标相对组件；数据坐标使用指定的轴。默认裁剪，`unclipped` 允许延伸到绘图区外。图片使用 PNG/JPEG/BMP 的 base64 `data:image/...` URI，两端宿主均可解码；图片进入 SVG 和 PDF 导出。

图元有独立 ID 和 `layer`；`anchored: true, anchor: 父图元ID` 引用父锚点。`anchor-part` 0–8 对应包围矩形的左上、上中、右上、左中、中心、右中、左下、下中、右下。缺失锚点和循环引用返回错误。曲线额外使用 `x3/y3, x4/y4` 控制点。

## 交互和刷新

| 操作 | 行为 |
|---|---|
| 鼠标拖动 | 平移，`allow-pan` 和 `directions` 限制轴方向 |
| 滚轮 | 光标中心缩放；`allow-zoom` 和 `zoom-directions` 控制 |
| Shift 拖动 | 按 `zoom-directions` 框选缩放；单轴模式的选框占满另一方向 |
| Ctrl/Meta 拖动 | 合并矩形选择；选择保持原始数据索引 |
| 点击数据、Ctrl/Meta 点击 | 替换选择、切换单点选择；选中数据有高亮 |
| 点击图例 | 切换系列显隐并触发回调 |
| 双击、Home、0 | 恢复自动范围 |
| 方向键、+/- | 平移、缩放，遵守交互开关 |
| Escape | 清除选择并取消拖动 |

`directions` 为 1(水平)、2(垂直)、3(两者)。`zoom-directions` 可独立指定缩放方向：1 仅 X 轴、2 仅 Y 轴、3 双轴，默认跟随 `directions`，兼容既有用法。它统一约束滚轮、键盘 +/-、Shift 框选和 `zoom-at()`，不会改变平移方向；切换方向不会复位已有范围或中断实时跟随。Gallery 与专业绘图示例提供“缩放：X + Y / 仅缩放 X 轴 / 仅缩放 Y 轴”选择器，默认双轴，极坐标模式禁用该选择器。

```slint
ForgePlot {
    directions: 3;       // 平移仍允许两个方向。
    zoom-directions: 1;  // 仅缩放 X 轴，Y 轴范围保持不变。
}
```

`enabled: false` 禁止指针和键盘交互。`show-crosshair/show-tooltip/show-legend` 可控制辅助显示。

公共函数包括 `append-point(id, point)`、`append-points(id, points)`、`flush()`、`follow-latest()`、`scroll-history(position)`、`measure-at(id, x)`、`refresh()`、`refresh-immediate()`、`reset-view()`、`zoom-at(x,y,factor,panel)`、`pan-by(dx,dy,panel)`、`select-point(id,index,additive)`、`clear-selection()`、`export-file(path,format)`、`release()`。追加数据自动以 `frame-interval` 合并视图刷新，缺省 16ms；`flush()` 在需要立即读取最新 scene 时更新视图，导出会自动执行。`refresh()` 用于原地修改静态模型；它重新导入静态数据，因此增量追加不需要调用它。替换点模型自动重新导入。

父组件共享可变的 `series/axes/selection` 时使用 `<=>` 双向绑定，接收图例、视图和选择的更新。若使用只读派生数据，应用在切换配置或更新数据时重新发布模型；随附示例保留实时更新中的图例显隐，切换模式时重新发布坐标并清除选择。

回调包含数据点、图元、轴、图例、选择变化、坐标变化和范围变化。`selection` 使用闭区间 `{id,first,last}`。`hovered` 是当前命中；`scene` 提供图像、SVG、范围、绘图区及统计，`status` 为 0(正常)、1(无有效数据)、2(错误)、3(未绑定)。

## 大数据与导出

显示图像按场景和窗口 DPI 缓存；悬停、十字线和提示框移动复用像素图像。数据、范围、选择、主题、尺寸或 DPI 变化后重新绘制。缓存仅包含当前图像，SVG/PDF 保留逻辑尺寸与矢量内容。密集折线上的散点符号按屏幕网格去重，折线顶点、误差和原始索引命中保持完整；关闭 `adaptive-sampling` 可显示全部符号。

仓库 Gallery 和独立 Rust 图表示例的 Debug 配置对外部依赖启用优化，并保留项目调试信息；Rust SDK 的共用 C++ 数值引擎也启用优化。外部 Rust 应用控制自身 Cargo profile，可采用 `[profile.dev.package."*"] opt-level = 3`，或使用 Release 构建。

折线用像素列的首点/最小值/最大值/末点保留尖峰和断点；散点按二维网格采样。`adaptive-sampling` 缺省开启，`max-points-per-pixel` 控制采样预算。关闭自适应可检查完整绘图。单个系列使用合并路径，界面不为百万点创建百万个控件。仅改变视图时复用原始输入，框选查询完整数据。

Rust 的 `PlotBuffer` 和 C++ 的 `ForgePlotBuffer` 仍供有界缓存使用。Gallery 与独立示例以 33ms 周期批量追加实际运行期间的 10ms 采样点，暂停后保留时间和历史。直接 f64 数值使用 Rust `plot::Plot::append_xy(id, &[(x,y)])` 或 C++ `ForgePlot::append_xy(id, points)`；随后 `render()`。日期时间轴使用相对秒数和 `time-origin`，避免 Slint float 的大时间戳精度损失。

## 实时窗口、图例与测量

```slint
plot := ForgePlot {
    axes: [{id:0,side:bottom,label:"时间 / s"},
           {id:1,side:left,label:"电压 / V"},
           {id:2,side:right,label:"温度 / ℃"}];
    series: [{id:0,name:"电压",y-axis:1}, {id:1,name:"温度",y-axis:2}];
    x-window-mode: unlimited;
    x-window-size: 10;
    legend-position: top-right;
    legend-placement: inside;
    legend-style: framed;
    legend-latest-value: true;
    measure-enabled: true;
    measurement-series: 0;
    measurement-position: bottom;
    measurement-placement: outside;
    cursor-a: 1; cursor-b: 3;
}
// 回调中调用 plot.append-point(0, {x:时间, y:电压})；批量输入用 append-points。
```

`x-window-mode` 为 `fit`（全量自动适配）、`unlimited`（保留历史、按跨度显示、超出跨度后显示滚动条）或 `fixed`（按跨度显示、不显示历史滚动条）。`x-window-axis` 指定线性 X 轴 ID，默认 0；`x-window-size` 使用数据坐标单位。三种模式均保留追加数据；固定模式限制的是视图跨度。实时跟随时，滚轮、键盘、`zoom-at()` 和框选缩放调整显示跨度，并继续跟随最新数据；Y 轴保留缩放后的范围。平移或历史滚动暂停跟随，此时继续缩放也保持历史定位。滚动条双击或 `follow-latest()` 恢复 X 轴跟随并保留当前缩放跨度；`reset-view()` 复位两方向，并恢复配置的 `x-window-size`。修改窗口模式、目标 X 轴或配置跨度时清除缩放跨度；`fit` 模式仍使用原有的自由缩放行为。

`legend-position` 支持 `bottom/top/left/right/top-right/bottom-right/top-left/bottom-left`；`legend-placement` 为 `inside`（默认）或 `outside`。区域内距第一个绘图区边界固定 6 个逻辑像素；区域外预留独立布局带，距坐标轴刻度及标题的占用边界为 6 像素，避免遮挡波形及坐标。上下位置居中、左右位置垂直居中、四角按对应边界对齐；外部四角使用上方或下方布局带。所有位置和样式背景均透明；`legend-style` 支持 `plain/framed/compact`，其中 `framed` 仅绘制边框。`legend-latest-value` 显示最新有效点的 Y 值，金融图使用 close；`legend-precision` 默认 2。点击区域内外图例均可隐藏系列，调整布局和曲线外观保留追加数据。

启用 `measure-enabled` 后拖动 A/B 光标，测量栏以四行显示系列名、两点的 X/Y 和 ΔX/ΔY。橙色 A、紫色 B 竖线与顶部字母标记使用所选系列的 X 轴。`measurement-series` 指定系列 ID；`measurement-a/b` 返回原始采样点和原始索引。测量读取最接近光标的原始点，不使用显示抽样点，也不在断点之间插值。开发者可通过 `measure-at(id,x)` 查询任意系列。

`measurement-position` 与图例共用上述八种位置，包含左上 `top-left` 和左下 `bottom-left`，默认 `bottom`；`measurement-placement` 默认 `outside`，也可设置为 `inside`。`measurement-width/measurement-height` 默认 0px，按系列名、数值及字体自动计算紧凑尺寸；设置正值可指定尺寸，并受可用空间限制。测量框跟随选定曲线所在的绘图区，只显示一次；图例和测量框在区域外选择同一侧时分别占用相邻布局带，间隔为 6px。区域内信息框发生重叠且空间足够时，测量框向内错开。测量框在区域内使用完全透明背景，区域外使用主题背景；文字及边框随主题切换，系列名超长时省略显示。布局元数据通过 `scene.legend-rect/measurement-rect` 提供；游标位置更新不重绘原生场景图像。

首次启用时，如果 A/B 已移出视图，将其放到当前 X 范围的 25% 和 65% 位置。`measurement-follow-view` 默认在实时窗口模式启用，光标随范围变化保持相对位置，拖动后使用新的相对位置；设为 `false` 可锚定数据坐标。`reset-measurement-cursors()` 将两个光标复位到当前可见范围。`scene.measurement-axis/measurement-range` 提供测量系列的坐标轴及范围。

鼠标靠近光标时显示左右拖动指针，可按住竖线或顶部 A/B 标记拖动。拖动由稳定的图表输入层保持捕获，实时追加数据时仍跟随鼠标，光标交叉也不会切换正在拖动的光标。拖动光标不触发坐标轴平移；Esc、取消输入、关闭测量或禁用组件可结束拖动。

有序折线通过二分定位可见点；像素桶保留首、尾、峰、谷。纯 XY 折线使用增量维护的分层摘要，数据块完全位于同一像素桶时直接读取摘要；断点、误差棒、独立选择和对数符号变化回退到逐点处理。自动范围也使用增量维护的线性及正负对数域范围。无限历史的内存随原始点数增长；乱序折线需重新排序，参数曲线保留原始顺序。

`export-file` 支持 `png/jpeg/jpg/bmp/svg/pdf`，导出图表本身及其当前视图、尺寸、选择。PNG/SVG 保留透明度；JPEG/BMP 以白色合成透明背景。SVG 使用合并矢量路径并嵌入位图。PDF 的曲线、轴、图元为矢量，色图/图片带透明掩码；Windows 导出把字体轮廓写入 PDF，中文不依赖阅读端字体，轮廓文字不作为可搜索文本。写入错误通过返回值报告。

当前功能、性能和安装消费者验收平台为 Windows x64、Slint 1.18.0。C++ JPEG/BMP 编码使用 Windows WIC；其他平台的对应编码器需另行提供，不能把 Windows 验收外推为跨平台运行验收。

## 可运行示例

- Rust：`cargo run --release --manifest-path examples/plot-rust/Cargo.toml`。
- C++：先安装 SDK，然后配置 `examples/plot-cpp`，将安装前缀与 Slint SDK 加入 `CMAKE_PREFIX_PATH`；运行时让 Slint SDK 的 `lib` 目录在 PATH 中。
- 两端 `--smoke` 遍历十种图形场景、捕获窗口、验证导出；`--benchmark` 测量十万/百万点模型准备、场景准备、窗口捕获、平移和缩放。
- 两端 `--interaction-benchmark` 使用真实悬停、拖动和滚轮事件，以及实时模型更新；记录事件、场景和完整窗口截图耗时。`SLINTFORGE_PLOT_QA_DIR` 指定本次输出目录，性能回归检查为 `tools/verify_plot_performance.py`。

完整实施范围见 [实施与验收](PLOT_IMPLEMENTATION.md)，回归与性能结果见 [图表验证记录](PLOT_VERIFICATION.md)。

日常交互卡顿及菜单入口的后续修复见 [交互性能复核](PLOT_PERFORMANCE_REVIEW.md)。

增量实时数据、历史窗口、图例和光标测量的最新验收见 [实时绘图与交互验证](PLOT_STREAMING_IMPLEMENTATION.md)。

图例透明背景、八方位贴边布局及深色测量框的后续修复见 [主题与图例布局复核](PLOT_APPEARANCE_REVIEW.md)。

图例与测量框区域内外设置以及默认红色图片移除见 [区域内外布局验证](PLOT_PLACEMENT_REVIEW.md)。两端 `--placement-smoke` 捕获本次位置组合，使用 `tools/verify_plot_placement.py` 复验布局和测量文字。

左上/左下位置和区域内完全透明背景的最新复核见 [透明叠加与左侧角落验证](PLOT_OVERLAY_REVIEW.md)。

展开菜单、实时光标可见性、紧凑测量框与 Gallery 分行设置见 [图表可用性验证](PLOT_USABILITY_REVIEW.md)。

实时更新期间的光标拖动修复见 [光标拖动验证](PLOT_CURSOR_DRAG_REVIEW.md)。

系列在折线、参数曲线或其他类型之间切换时重新建立绘制顺序，保留原始点索引及已追加历史。类型切换后继续追加的回归结果见 [组件代码审查与修复](COMPONENT_CODE_REVIEW_20261004.md)。
