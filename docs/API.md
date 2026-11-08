# SlintForge v0.1.2 公共 API

宿主启动接口：Rust `configure_rendering(RenderOptions)` / C++ `forge_configure_rendering(ForgeRenderOptions)`，默认 `GpuPreferred`，支持 `Software`；详见 [UI 渲染偏好](RENDERING.md)。

> **兼容提示：**本页继续保留历史入口。按组件拆分、可中英文切换的新版文档见 [中文 API 站点源码](site/index.md) 与 [English API source](site/en/index.md)；发布包通过 `SlintForge-Docs.exe` 打开构建后的离线站点。

新增组件、公共模型与双端接入示例见 [新增组件 API](COMPONENTS.md)。DataGrid、TreeDataGrid 见 [表格 API](GRIDS.md) 和 [接手审查](GRID_REVIEW.md)；专业二维图表 ForgePlot 见 [图表 API](PLOT.md) 和 [验证记录](PLOT_VERIFICATION.md)。服务绑定、模型刷新和线程要求统一见 [接入约定](INTEGRATION.md)。专题验证报告保留原实施阶段记录，当前版本以发布目录的验收报告为准。

节点编辑、流程图、拓扑图和思维导图使用 [ForgeFlow API](FLOW.md)。公共组件为 `ForgeFlow`、`ForgeFlowCanvas`、`ForgeFlowNode`、`ForgeFlowPort`、`ForgeFlowControls`、`ForgeFlowMiniMap`；共享模型与服务从 `@slintforge` 导入，C++ 使用 `ForgeFlowDocument`，Rust 使用 `slintforge::flow::FlowDocument`。该组件要求 Skia 渲染器。

## `.slint` 契约

从 `@slintforge` 导入下列类型、全局对象和组件。构建侧将 `slintforge` 映射到 `ui/slintforge.slint`；该文件只导出公共 API，实现按类别位于 `ui/types`、`ui/basic`、`ui/navigation`、`ui/layouts`、`ui/surfaces`、`ui/window`、`ui/typography` 和 `ui/icons`。

| 类型 | 字段或取值 |
|---|---|
| `ForgeRouteParam` | `key: string`、`value: string` |
| `ForgeNavigationItem` | `id`、`parent-id`、`title`、`icon`、`kind`、`depth`、`expanded`、`has-children`、`child-count`、`enabled` |
| `ForgeNavPlacement` | `left`、`right`、`top` |
| `ForgePaneState` | `expanded`、`compact`、`hidden` |
| `ForgeNavItemKind` | `page`、`group`、`header`、`separator` |
| `ForgeInfoLevel` | `info`、`success`、`warning`、`error` |
| `ForgeBreadcrumbItem` | `id`、`title` |
| `ForgeTabItem` | `id`、`title` |
| `ForgeIconEntry` | `name`、`codepoint`、`glyph`；供 Slint 页面显示图标目录 |
| `ForgeSplitOrientation` | `horizontal`、`vertical` |
| `ForgeStaggeredItem` | 稳定 `id`、`title`、`description`、`tile-height`、`accent`、宿主计算的 `column` 与 `top` |
| `ForgeDockRect` | `x`、`y`、`width`、`height` |
| `ForgeDockEdge` | `top`、`bottom`、`left`、`right` |

| 全局对象 | 属性 | 用途 |
|---|---|---|
| `ForgeTypography` | `family: string`，默认 `""`；`scale: float`，默认 `1.0` | `ForgeWindow` 将 `family` 用作默认文字字体；库组件和 Gallery 的字号绑定 `scale`。应用自己的文字须显式绑定 `ForgeTypography.scale`。`""` 表示使用平台默认字体。 |
| `ForgeIcons` | `add`、`apps`、`arrow-left`、`arrow-clockwise`、`brightness`、`sun`、`moon`、`checkmark`、`chevron-down`、`chevron-right`、`dismiss`、`globe`、`home`、`info`、`panel-left`、`panel-left-expand`、`panel-right`、`panel-right-expand`、`panel-top-contract`、`panel-top-expand`、`pin`、`pin-off`、`search`、`settings`、`text-font`、`window` | 常用图标字形字符串；完整目录由 Rust/C++ 的 `IconCatalog` / `ForgeIconCatalog` 查询。 |

| 组件 | 输入 | 回调 / 插槽 |
|---|---|---|
| `ForgeWindow` | 继承 `Window` 的 `title`、`icon`、`maximized`、`minimized` 等属性；`brand-symbol`、`brand-icon`、`show-brand-icon`、`version-text`、`show-menu-button`、`navigation-glyph`、`show-theme-button`、`show-pin-button`、`show-settings-button`、`show-maximize-button`、`dark-mode`、`pinned`、`rounded-corners`；只读 `title-bar-height` | `menu-requested()`、`theme-toggle-requested()`、`pin-state-changed(bool)`、`settings-requested()`、`@children` |
| `ForgeIcon` | `glyph`、继承自 `Text` 的 `color`、`font-size`、`width`、`height` | 使用内嵌图标字体绘制字形；默认字号随 `ForgeTypography.scale` 变化 |
| `ForgeNavigationView` | `items`、`tree-items`、`footer-items`、`active-route-id`、`active-top-id`、`placement`、`pane-state`、`brand`、`search-query`、`can-go-back`、`dark-mode`、`show-app-bar` | `toggle-pane()`（完整菜单／图标栏）、`navigate-requested(id, params)`、`group-toggle-requested(id)`、`pane-state-requested(state)`、`search-requested(query)`、`back-requested()`、`theme-toggle-requested()`、`@children` |
| `ForgeBreadcrumb` | `items` | `selected(id)` |
| `ForgeTabBar` | `items`、`active-id` | `selected(id)` |
| `ForgePageHeader` | `title`、`subtitle`、`can-go-back` | `back-requested()` |
| `ForgeIconButton` | `icon`、`label`、`selected`、`enabled` | `clicked()` |
| `ForgeCard` | `title`、`content-padding`、`spacing` | `@children`；自然高度、空标题和标题换行 |
| `ForgeInfoBar` | `level`、`message`、`dismissible` | `dismissed()` |

### 布局组件

| 组件 | 输入与读取值 | 回调 / 内容 |
|---|---|---|
| `ForgeSplitLayout` | 双向 `ratio`（拖动时限制为 0.15–0.85）、`orientation`、`divider-size`；只读 `first-width`、`first-height`、`second-x`、`second-y`、`second-width`、`second-height` | `ratio-changed(float)`、`@children`；子项绑定公开几何属性。支持水平与垂直分割。 |
| `ForgeStaggeredGridView` | `items: [ForgeStaggeredItem]`、`min-column-width`（220px）、`max-columns`（4）、`gap`（12px）、`content-padding`（12px）、`content-height`；只读 `column-count`、`column-width`、`vertical-scrollbar-visible` | `layout-requested(columns)`、`selected(id)`。列数变化时宿主重算每项 `column`、`top` 与总高度，再回写模型。列宽不足时退为一列；按需竖向滚动。 |
| `ForgeFlowLayout` | `spacing`、`content-padding`；只读 `content-height`、`vertical-scrollbar-visible` | `@children` 放置任意尺寸的子组件，按可用宽度自动换行；内容高度超出视口时自动显示竖向滚动条，不显示横向滚动条。 |
| `ForgeAdaptiveGrid` | `min-item-width`（180px）、`column-gap`、`row-gap`、`content-padding`（三者默认 12px）、`max-columns`（0 表示不限）；只读 `item-width`、`column-count`、`content-height`、`vertical-scrollbar-visible` | `@children`；调用方把每个卡片的 `width` 绑定到 `item-width`。自动换列、同列宽、卡片可变高且逐行对齐，末行靠左，按需竖向滚动。 |
| `ForgeDockLayout` | 双向 `top-height`、`bottom-height`、`left-width`、`right-width`；`show-top/bottom/left/right`、`divider-size`（8px）、`min-center-width/height`（160×120px）；只读 `top/bottom/left/right/center-rect: ForgeDockRect` | `pane-size-changed(edge, size)`、`@children`；调用方绑定五区几何。四条分隔线可拖，隐藏边区收回空间并保留尺寸，窗口缩小时先压缩中央区。 |
| `ForgePageStack`、`ForgeStackPage` | Stack 的 `active-id`，每页的 `page-id` 与 `active-id` | 两层均提供 `@children`。每个预编译页面用 `ForgeStackPage` 包裹并把其 `active-id` 绑定到 Stack；隐藏页保留实例与输入状态，未知 ID 隐藏全部页面。 |
| `ForgeFormRow` | `title`、`description`、`breakpoint`（560px）、`label-width`（220px）、`spacing`（12px）；只读 `stacked-layout`、`control-origin-x/y` | `@children` 放自然宽度的控件。宽屏右对齐，窄屏移到文字下方靠左；多行直接用官方 `VerticalLayout`。 |

`ForgeAdaptiveGrid`、`ForgeFlowLayout`、`ForgeStaggeredGridView` 都使用按需竖向滚动，预留滚动条空间以避免列数反复抖动。首版面向中小规模数据，不提供虚拟化；Slint 1.18.0 不能在可虚拟化重复项中复用调用方的任意 `@children` 模板。通用虚拟网格暂缓。

瀑布流布局由 Rust `StaggeredLayout::arrange(&[StaggeredInput { id, height }], columns, gap, padding)` 与 C++ `ForgeStaggeredLayout::arrange(vector<ForgeStaggeredInput>, columns, gap, padding)` 实现同一最短列算法，列高相同时取最左列。返回包含稳定 ID、列号、纵向位置的列表和内容高度；高度单位为 Slint 逻辑像素。宿主接到 `layout-requested` 后回写 `ForgeStaggeredItem` 的 `column`、`top` 与组件 `content-height`。

两端瀑布流算法都把负数及非有限高度、间距和内边距按 0 处理，列数不超过条目数（空列表仍按一列计算），避免异常输入造成过大分配。瀑布流的 `content-height` 直接控制滚动视图内容高度；调用方应在每次重排后同步回写。`ForgeFormRow` 会按说明文字的实际换行高度计算控件起点与最小行高；不要给长说明设置小于最小行高的固定高度。Dock 边区的调用方内容应同时绑定对应的 `show-*` 状态或开启裁剪，避免隐藏后的子项越出零尺寸区域。

### 容器组件

| 组件 | 输入与读取值 | 行为 |
|---|---|---|
| `ForgePage` | `active`（true）、`content-padding`（0px）；只读 `capture-rect` | `@children`、裁剪、常驻状态；`capture()` 返回 `ForgeCaptureResult`。 |
| `ForgeRoundImageView` | `source: image`、`corner-radius`、`image-fit`，以及继承的边框属性 | 默认 `cover`；支持 `contain`、`fill` 和空图片，随尺寸变化重新布局。 |
| `ForgeIconLabel` | `icon`、`label`、`description`、`vertical` | 网格布局组合图标、标题和说明；支持换行、水平／垂直排列与字号缩放。 |
| `ForgeExpander` | `title`、双向 `expanded`、`content-height`、`content-padding`、`enabled` | `toggled(bool)`；默认自然高度，`content-height > 0` 为显式高度。点击或 Enter/Space 切换；折叠保留实例。 |
| `ForgeWatermark` | `mark`、`mark-opacity`、`tile-width`（180px）、`tile-height`（120px）；只读 `columns`、`rows` | 按尺寸平铺水印，透明度限制为 0–1；内容保持可交互。空文字隐藏水印。 |
| `ForgeMarquee` | `text`、`cycle-duration`、双向 `running`；只读 `overflowing`、`text-width`、`text-position` | 仅溢出时循环滚动，暂停后原位继续；文本、宽度或周期改变后重算。非正周期静止显示。 |

圆角裁剪依赖所选 Slint 渲染器；[Slint 官方渲染器说明](https://docs.slint.dev/latest/docs/slint/guide/backends-and-renderers/backends_and_renderers/)指出软件渲染器对 `clip: true` 和圆角同时使用存在限制，验收时以 Windows x64 默认渲染器为准。

`ForgeWindow` 使用 Slint 的无边框窗口和原生窗口拖动能力，内容插槽位于 ForgeMetrics.title-bar-height（默认 36px）标题栏下方。标题栏左侧的图标与文字垂直居中；非空 `version-text` 紧跟标题并以括号显示，例如 `SlintForge Gallery (v0.1.2)`。主题、图钉、设置及窗口控制位于右侧；可通过 show-back-button、leading-menu-button、show-search-box 启用左侧返回、菜单与中部搜索，搜索框默认高 28px。`navigation-glyph` 决定菜单按钮的字体图标；Gallery 根据左、右、顶部导航位置和隐藏状态选择对应的 `ForgeIcons.panel-*` 字形，点击后在完整菜单（208px）与图标栏（48px）间切换；隐藏状态仍可由应用显式设置。主题按钮使用 `ForgeIcons.sun` / `ForgeIcons.moon`：深色时显示太阳供切换到浅色，浅色时显示月亮供切换到深色。`show-brand-icon` 为 `true` 时绘制 `brand-icon`，否则使用 `brand-symbol` 方块。`Window.icon` 是任务栏／系统窗口图标，需由应用另行设置。双击可拖动区域切换最大化。`pinned` 双向绑定 Slint `always-on-top`；标题栏图钉切换此属性并发出 `pin-state-changed`。设置按钮不会自行注册页面，`theme-toggle-requested` 不会自行改变 `dark-mode` 或 `Palette.color-scheme`。边缘缩放由 `resize-border-width: 6px` 提供，Slint 1.18.0 的这一属性只在 winit 后端生效。

`rounded-corners` 是窗口边角意图。Windows 宿主在窗口句柄创建后调用 `slintforge::apply_window_corners(window, rounded)` 或 `forge_apply_window_corners(window, rounded)`，设置变化时再次调用。两者分别位于 Rust SDK 和 C++ `ForgeWindowCorners.hpp`，返回值表示原生请求是否成功。实现使用 Windows 11 的 DWM 边角偏好；Windows 11 构建 22000 以下和其他平台不支持此原生效果，且 DWM 对无边框窗口保留最终决定权。最大化、贴靠、虚拟机等场景可能由系统保持直角。

文字字体使用系统已安装的字体族名称；`ForgeTypography` 不提供字体文件加载接口。`ForgeIcon` 单独使用随库打包的 Microsoft Fluent System Icons Regular 字体，不受文字字体族设置影响。该资源的 MIT 许可文件位于 `ui/assets/fonts/LICENSE-FluentSystemIcons.txt`。可直接给 `glyph` 传 `"\u{f601}"` 等字形；`ForgeIcons` 提供常用码位。

完整图标目录有 9,767 个字体变体，名称采用官方码位键去掉 `ic_fluent_` 前缀的形式，如 `home_20_regular`。`ui/icons/catalog.tsv` 从固定版本的官方 JSON 生成；运行 `py ui/icons/sync_catalog.py --check` 可校验同步。Rust `IconCatalog::new().find(name)` 返回 `IconGlyph { name, codepoint }`，`glyph()` 可直接传给 `ForgeIcon.glyph`；`search(query, offset, limit)` 返回结果页和总数。C++ `ForgeIconCatalog::find(name)`、`count(query)`、`search(query, offset, limit)` 与 `ForgeIconGlyph::glyph_utf8()` 对应。搜索按名称进行 ASCII 大小写不敏感的包含匹配。

`ForgeNavigationView` 的 `navigate-requested` 默认传空参数列表；业务可自行通过 Router 调用带参导航。导航相关回调代表意图，页面和面板状态由宿主确认后回写。

侧栏使用 `items` 中按展开状态扁平化的可见行。顶部模式使用 `tree-items` 中的完整导航树，按 `parent-id` 构建单行分类入口和多级下拉菜单；`child-count` 用于确定菜单高度，`active-top-id` 标记当前页所属的一级分类。未提供 `tree-items` 时顶部模式回退到 `items`。顶栏宽度不足时可从右侧“更多”菜单访问全部一级分类。Gallery 的导航条目使用同一份图标目录中的语义图标名称，Rust/C++ 分别解析为字形后交给 `ForgeIcon` 绘制。

Gallery 的 全部注册页面均提供实际演示，包括导航、布局／容器、输入、基础图表、专业图表和表格。组件预览区域占页面主要空间，页面不显示源码面板；目录中的全部独立用法示例位于 `examples/component-usage`，由统一验证入口编译。

## 宿主路由

| 操作 | Rust | C++ | 行为 |
|---|---|---|---|
| 创建 | `Router::new(ids, initial)` | `ForgeRouter::create(ids, initial, out)` | 注册构建时已存在的页面 ID；拒绝空 ID、重复 ID、未知初始页 |
| 入栈 | `go(id, params)` | `go(id, params)` | 成功时当前页入历史；与当前 ID 和参数完全相同则不入栈 |
| 替换 | `replace(id, params)` | `replace(id, params)` | 更换当前页，不增加历史 |
| 返回 | `back()` | `back()` | 无历史时返回错误且状态不变 |
| 读取 | `current()` | `current()` | 返回当前 ID 与参数 |

Rust 参数类型为 `slintforge::RouteParam`。C++ 宿主参数类型为 `ForgeRouteArgument`，用于避免与 Slint 生成的全局 `ForgeRouteParam` 重名；两个类型都含 `key` 和 `value`，Gallery 在回调边界转换。Rust 用 `Result<T, Error>`（运行库缺失、ABI 不符等加载或调用失败返回 `Error::Runtime { code, message }`，不会 panic），C++ 用 `ForgeStatus { code, message, changed }` 报告结果。失败操作保持当前页及历史不变。

## 宿主导航树

Rust 使用 `slintforge::NavTree` 与 `NavItem`；C++ 使用 `ForgeNavTree` 与 `ForgeNavItem`。条目记录稳定 ID、可选父 ID、标题、图标、种类、启用状态。`visible_items()` 按输入中的兄弟顺序深度优先返回可见行。`toggle(id)` 和 `set_expanded(id, bool)` 只对分组有效。创建时检查空 ID、重复 ID、未知父 ID、非分组父节点和环。失败时保留原树状态。

## 接入示例

Rust 的 `build.rs`：

```rust
let config = slint_build::CompilerConfiguration::new()
    .with_library_paths(std::collections::HashMap::from([
        ("slintforge".into(), slintforge::ui_library_path()),
    ]));
slint_build::compile_with_config("ui/app.slint", config).unwrap();
```

C++ 的 `CMakeLists.txt`：

```cmake
find_package(SlintForge 0.1 CONFIG REQUIRED)
add_executable(app main.cpp)
target_link_libraries(app PRIVATE Slint::Slint SlintForge::Sdk)
slintforge_target_sources(app ui/app.slint)
```

Slint 文件：

```slint
import { ForgeWindow, ForgeCard, ForgeIconButton } from "@slintforge";
```

可运行的完整最小示例位于 `examples/rust`、`examples/cpp` 与 `examples/ui/minimal.slint`。

### 原生圆角与裁剪

圆角矩形和圆角裁剪直接使用 Slint `Rectangle`，不另行导出包装组件。主题色由调用方绑定 `Palette`。

```slint
Rectangle {
    border-radius: 12px;
    border-width: 1px;
    border-color: Palette.border;
    background: Palette.alternate-background;
    clip: true;
    // 在此放入子组件；不需要裁剪时省略 clip。
}
```

### 截图接口

英文接入说明见 [Window and page capture](CAPTURE.en.md)。

`ForgeCaptureRect { x, y, width, height }` 的字段为窗口逻辑像素长度。`ForgeCaptureResult` 包含 `ok`、`image`、`pixel-width`、`pixel-height` 和 `error`；尺寸为物理像素。`ForgeCaptureService` 是每个顶层组件所属的服务，提供 `connected`、`capture-window()`、`capture-region(rect)`。

`ForgeWindow.capture()` 截取整个客户区，包含自定义标题栏。`ForgePage.capture()` 截取窗口画面中页面矩形与窗口的交集；保留滚动位置和当前画面覆盖层。它不进行隐藏页面独立渲染或长截图，也不重建透明背景下的独立子树。页面容器应作为可视区域使用，将滚动内容放在其内部，并把 `active` 绑定到路由状态；不可见祖先不自动改变子项的 `active`。

截图须在所属 UI 线程、窗口完成显示和布局后调用。未显示窗口、未激活页面、未绑定服务、无效或没有交集的区域、渲染器失败均返回错误。坐标转换采用 `floor(起点 × scale)`、`ceil(终点 × scale)`，再与物理截图边界求交。渲染器支持以运行环境实际结果为准；软件渲染器的圆角裁剪限制仍适用。

消费工程的 Slint 编译入口应重导出 `ForgeCaptureService`、`ForgeCaptureRect`、`ForgeCaptureResult`。接入代码和完整示例见 `examples`：

```rust
slintforge::bind_capture_service!(ui, ForgeCaptureService, ForgeCaptureResult);
let pixels = slintforge::capture_window(ui.window())?;
let image = slint::Image::from_rgba8(pixels.clone());
slintforge::save_capture_png(&pixels, output_path)?;
```

```cpp
forge_bind_capture_service<ForgeCaptureService, ForgeCaptureResult>(ui);
const auto captured = forge_capture_window(ui->window());
if (captured.ok()) {
    slint::Image image(*captured.pixels);
    const auto error = forge_save_capture_png(*captured.pixels, output_path);
    // error 为空表示成功，非空时报告错误。
}
```

Rust 的 `capture_region(window, CaptureRect)` 与 C++ 的 `forge_capture_region(window, ForgeLogicalCaptureRect)` 返回 RGBA8 数据；`crop_capture` / `forge_crop_capture` 对已有像素缓冲裁剪。Rust 返回 `Result<SharedPixelBuffer<Rgba8Pixel>, CaptureError>`；C++ 返回 `ForgeCapturedImage { pixels, error }`。宿主绑定仅持有窗口弱引用，不形成引用环。

PNG 编码保留 RGBA、物理尺寸及透明度。保存接口由调用方传入路径并创建父目录；不会吞掉文件或编码错误。Rust 使用 `png 0.18.1`，Windows C++ 使用 WIC，安装包传递 `windowscodecs`、`ole32` 链接依赖。Gallery 及最小消费者导出到运行目录的 `build/captures`，文件采用时间戳命名并显示完整路径。


## 延时按钮与 Fluent 选择器

分栏时间、日期、日历、颜色、快捷键与三种延时确认模式详见 [控件 API](PICKERS.md)。
