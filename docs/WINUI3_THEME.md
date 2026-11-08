# WinUI 3 主题与紧凑专业浅色方案

`ui/theme/theme.slint` 提供公开的 `ForgeTheme` 和 `ForgeMetrics`；`ForgeTypography` 提供字体与缩放。所有扩展组件、Gallery 和标准控件通过项目自有的 `slintforge-winui3` 样式读取同一主题。

| 语义 | 浅色 | 深色 |
|---|---|---|
| 窗口基础层 | `#F1F3F6` | `#202020` |
| 内容层 | `#F7F8FA` | `#282828` |
| 浮层底 | `#FFFFFF` | `#2C2C2C` |
| 强调色 | `#005FB8` | `#60CDFF` |
| 强调色文字 | 白色 | 黑色 |
| 正文 | `#202832` | 白色 |
| 说明文字 | `#596575` | 原有半透明白色 |
| 普通描边 | `#D8DEE7` | 原有半透明白色 |
| 区域分隔线 | `#CDD4DD` | 原有半透明白色 |

文字、控件填充、输入框、描边、卡片、分隔线、焦点及状态色独立定义。XAML 的 `#AARRGGBB` 转成 Slint `#RRGGBBAA`，例如 `#5DFFFFFF` 转为 `#FFFFFF5D`。禁用控件分别应用文字、填充和描边，不使用整块透明度淡化。用户指定的数据颜色、图片和颜色选择器色谱保持原意。

基础控件高 32、导航行高 36、标题栏高 36，标题栏搜索框高 28；展开侧栏宽 208、紧凑侧栏宽 48；普通控件圆角 4、卡片／浮层圆角 8，描边 1。页面常规边距 24，较窄内容区减至 16。文字为说明 12、正文 14、小标题 20、标题 28。字体默认请求 Segoe UI Variable，缺失字体和中文由 Windows/Slint 系统字体集合回退；应用设置 `ForgeTypography.family` 与 `scale` 的优先级不变。

```slint
import { ForgeTheme, ForgeTypography } from "@slintforge";
// 在应用的初始化或设置回调中更新，组件实例和用户模型保持常驻。
// ForgeTheme.color-scheme = ColorScheme.dark;
// ForgeTypography.scale = 1.25;
```

`ForgeFlowTheme` 的默认值映射公共主题，也保留逐项覆盖和独立 `dark` 属性。首页使用连续内容层、实色卡片与分区细线，移除圆环装饰；入口卡片随可用宽度和字体缩放换行。Acrylic／LiquidGlass 继续作为独立组件示例。

## 浅色表面与交互状态

浅色控件、卡片、表格和绘图区采用不透明颜色，避免背景叠加使同一个控件在不同页面发灰。普通控件的正常／悬停／按下填充依次为 `#FFFFFF`、`#F3F6FA`、`#E8EDF3`；禁用文字、填充、描边分别设置。强调按钮的悬停与按下颜色由应用当前强调色加深得到，保留自定义强调色能力。

`navigation-selected` 和 `accent-surface` 将强调色分别按 11% 和 6% 混入白色，作为导航选择与图标容器底色。`table-background`、`table-header`、`table-alternate`、`table-selected`、`table-grid` 提供表格分层；`plot-background`、`plot-grid` 提供白色绘图区与浅灰网格，数据系列颜色不变。Flow 的浅色画布跟随内容层，工具面板跟随实色表面，应用侧覆盖仍保留。

浅深主题共用紧凑尺寸，切换时壳层尺寸不跳变；深色基础颜色沿用 WinUI 3 配色。字体与缩放仍由用户设置决定。标准控件通过样式适配层消费公共颜色，不修改上游控件文件。

## 标题栏与导航内容区

Gallery 的返回和导航切换按钮位于标题栏左侧，搜索框位于中部；主题、置顶、设置和系统窗口按钮位于右侧。返回按钮的禁用状态跟随路由历史。搜索复用现有中英文组件索引，回车打开首个结果，Escape 清空查询；搜索框独立于窗口拖动区域。

`ForgeWindow` 增加可选的 `show-back-button`、`back-enabled`、`leading-menu-button`、`show-search-box` 和 `search-text`，通过 `back-clicked`、`search-edited`、`search-accepted` 回调接入应用。`focus-search()` 可将键盘焦点移入搜索框。默认不启用新增控件，已有窗口仍可使用原来的右侧菜单按钮。

`ForgeNavigationView` 的 `content-width`、`content-height`、`content-x`、`content-y` 提供实际内容区域。页面直接绑定这些尺寸，不重复扣除侧栏宽度。左／右侧栏采用基础层，页面采用内容层，二者相接，不保留装饰性空条。标题栏底部、侧栏内缘与页脚用 1px `divider` 分隔，线条绘在侧栏内部，不额外扣除内容宽度。Gallery 隐藏侧栏内重复的品牌区和搜索框；其他消费者可通过 `show-pane-header`、`show-pane-search` 分别控制，默认仍显示。

`ForgeNavigationView.toggle-pane()` 和内置菜单按钮只在 `expanded`（完整菜单）与 `compact`（图标栏）间切换；从 `hidden` 调用时恢复为完整菜单。应用仍可显式设置隐藏状态。Gallery 标题栏调用同一个公共方法，搜索结果临时展开时点击菜单会清空搜索并收为图标栏。紧凑侧栏内隐藏的分组标题、分隔线和子项不占据行高或行间距。

标题栏按钮的鼠标操作不获取输入焦点，也不残留键盘焦点框；Tab 导航与键盘激活保留可见焦点提示。两端 Gallery 的侧栏回调仅更新 `pane-state`，不重新查询图标或生成页面模型；设置由后台队列串行保存，连续请求合并，正常退出时等待最终状态写入。

图表、节点和表格页面使用 `GalleryRetainedViewport`：隐藏时保留上次内容尺寸，避免菜单宽度变化触发离屏计算；显示时立即采用当前可用尺寸，文档和编辑状态保持原样。初始尺寸仅用于未显示页面的预热，不约束实际窗口或可见页面。

搜索期间临时展开导航结果，清空或进入结果后恢复原有导航模式；这一临时状态不写入设置。页面组件保持常驻，以保留草稿、滚动和展开状态。

## Gallery 启动与交互性能

Windows 下 Rust 和 C++ Gallery 均使用图形程序入口，直接打开可执行文件不会附带命令窗口；自动化检查仍可重定向标准输出和错误。Rust 的开发配置单独优化 Gallery 生成的界面代码，避免日常预览使用未优化的布局与绑定路径。

常驻页面隐藏时暂停进度动画、跑马灯和 Flow 连线动画，重新显示后继续工作；不会销毁页面、文档或输入草稿。搜索、导航和主题变化只替换内容确实改变的列表模型，保留未变化的图标、完整导航和页脚实例。

Rust Gallery 的 `--interaction-smoke` 是定向交互检查入口，执行搜索、分组展开、侧栏切换、页面跳转、主题和窗口尺寸操作，检查草稿与模型保留，并测量操作到完成绘制的时间及末尾空闲重绘数量。结果默认写入 `build/gallery-performance/interaction/result.json`，可通过 `SLINTFORGE_INTERACTION_QA_DIR` 指定隔离目录；此模式不读取或保存用户设置。渲染耗时不包含显示器扫描输出，也不能代替真实鼠标和键盘检查。

该计时入口依赖 Slint 的渲染通知。当前 Gallery 未启用 WGPU 图形 API 暴露功能，运行此入口时须在隔离测试进程设置 `SLINT_BACKEND=winit-skia-opengl`；可通过 `Start-Process -WindowStyle Hidden` 后台执行。WGPU 和 Skia 软件渲染的验证使用 SDK 渲染测试及独立空闲采样，不能用缺少回调的超时判定界面卡死。普通启动仍使用公共接口的 GPU 优先默认值。

C++ Gallery 的 `--titlebar-smoke` 支持后台验证：向 Slint 场景直接发送指针事件，覆盖主题按钮、完整菜单／图标栏、窗口尺寸和重型页面恢复，并用渲染器快照保存 PNG。它不发送系统鼠标或键盘输入，也不请求激活窗口；启动器应使用 `-WindowStyle Hidden`。默认输出目录为 `build/titlebar/cpp-background`，可通过 `SLINTFORGE_TITLEBAR_QA_DIR` 覆盖，测试不读取或保存用户设置。

## 编译与 SDK

组件截图入口支持 `SLINTFORGE_COMPONENT_QA_BACKGROUND=1`：配合 `--components-smoke`、路由过滤和隐藏启动，仅使用渲染器快照，不执行最小化恢复或窗口激活。后台窗口尺寸无效时直接报错。

- Rust：`slintforge_build::configure_style()`，`CompilerConfiguration::with_style("slintforge-winui3".into()).with_include_paths(slintforge_build::style_include_paths())`。
- Rust 内联 Slint 测试：构建辅助工具生成 `OUT_DIR/slintforge_style.rs`；引入后使用 `slintforge_ui!`，避免相对 include 路径在嵌套组件中失效。
- C++：使用 `slintforge_target_sources()`。辅助函数负责自有样式、资源嵌入、依赖文件、翻译参数和 Windows 运行库复制。
- SDK 安装包含 `ui/styles/slintforge-winui3` 的所有 Slint/SVG 文件、来源、SHA-256 清单和许可证；不得修改 Slint 安装目录。

标准控件使用固定版本 Slint 1.18.0 的未改动文件。项目只提供独立的主题适配文件；`upstream-sha256.json` 可核对原文件完整性。编译入口统一启用该版本内部属性标记所需的实验性编译选项，升级 Slint 时应同步审查适配层。

## 参考

- [微软 WinUI 主题资源](https://github.com/microsoft/microsoft-ui-xaml/blob/main/controls/dev/CommonStyles/Common_themeresources_any.xaml)
- [Windows 字体规范](https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/typography)
- [Windows 几何规范](https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/geometry)

实际构建、图形、性能和窗口验证记录见本次迁移验证文档；源码检查不能代替实际窗口验收。
