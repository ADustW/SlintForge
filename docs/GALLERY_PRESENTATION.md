# Gallery 展示页调整清单

## 2026-10-25 组件页模板改版

组件页改为统一模板（`Gallery/ui/component/component-page.slint`）：页头显示分类路径、组件名、来源标签（共享组件 / 原生控件、Rust · C++）和一句说明；正文由若干示例区组成，每个示例区包括标题、说明、内容居中的预览框、可选的右侧属性面板和底部输出栏；页尾列出主要属性、回调和函数的 API 摘要。原来所有组件通用的“多种效果 / 业务组合”标签页，改为按组件编写的“状态对照”和实际场景示例（如对话框操作栏、订单评价、打印设置、运营看板、编辑器布局）。

先以 PillButton、RatingControl、NumberBox、Statistic、SplitLayout、CalendarPicker、DataGrid、TreeDataGrid 和七类基础图表试点，确认后迁移了全部 81 个组件页；首页同步改版（Fluent 图标替换字母占位、按宽度 4/2 列排布、移除调试用布局按钮）。仅首页、关于、设置、生命周期、占位页及 ForgePlot、ForgeFlow 演示保留各自布局。截图验收沿用 `showcase-routes.inc` 的状态数：`tab = N` 时滚动到第 N 个附加示例区，Rust 端每轮 498 帧。旧的 `GallerySamplePage`、`GalleryShowcase`、`GalleryDemoCard`、`GalleryPickerExample` 已删除，下文的逐页高度表仅作历史记录。

迁移中顺带修复的库问题：导航分组箭头与 NavigationView 应用栏改用图标字体（原字符缺字形显示为方框，`ForgeIcons` 新增 `navigation`）；Timeline 行高固定，避免空间不足时重叠；SkeletonView 行宽改用伸缩比例，消除可拉伸布局中的绑定环，并修正行数超过 30 时末行缩短的判断；FileTree 去掉缺字形的“▸”前缀；Statistic 无单位时不再追加空格。

参考本地 FluentUI-Pro Gallery 源码。统一 24px 页边距、24px 分区间距、8px 圆角与 1px 边框，卡片内留白 20px。示例按内容指定高度，并保留导航、输入、弹层和窗口状态。用法代码位于独立示例目录，页面不显示代码面板。

日历选择器按参考页采用标题与两张紧凑示例卡片，省略副标题，分区间距为 20px；未选择日期时两种模式按钮同宽，选中后宽度随文本自适应。月历弹层为 300×350px，日期时间弹层为 500×350px，均随字号缩放。

逐项记录覆盖全部 96 个路由；DataGrid、TreeDataGrid 和专业图表 ForgePlot 均提供交互演示。没有对应 FluentUI 页面的扩展布局使用同一 ScrollablePage / CardHighlight 展示约定。

## 2026-10-05 示例扩充

55 个页面增加可直接浏览的组合示例。44 个输入、表单、反馈、数据和导航页面保留基础示例，并增加“多种效果”和“业务组合”标签；状态卡片各自保存输入和选择，标签与树使用宿主模型。当前页面的扩展示例按需创建，减少初始栈占用；原有基础页继续常驻。宽窗口并排展示，窄窗口自动换行，通过页面滚动查看全部示例；切换标签保留已输入内容。动画只在相关页面可见时运行。

ForgePlot 默认显示“曲线与统计”总览，另有“科学绘图”和“交互与导出”：总览展示曲线误差填充、参数曲线、正负柱形、统计箱、蜡烛、OHLC、二维色图、双轴、极坐标和多绘图区。点击卡片的“在交互示例中打开”即可进入相应模式，继续缩放、测量和导出。

七类基础图表提供样式、单系列和空数据对照；两个表格页面提供紧凑只读、禁用、单行与空列表。ContentDialog 页面可直接打开确认框或包含设备名、禁用端口和数据目录的业务对话框，并切换提交条件，验证焦点遍历和恢复。

双宿主自动验收保留原有 308 个状态，新增 111 个标签状态，并为 12 个复杂页面增加 50 个英文放大字号及窄窗口状态，共每端 469 帧。实际执行结果记录在本次发布目录的验收报告中。以下表格描述原有基础示例布局；新增标签与状态清单由 `Gallery/data/showcase-routes.inc` 统一定义。

| 路由 | 标题 | 参考页面 | 参考示例区数量 | 本轮展示内容 |
|---|---|---|---:|---|
| `home` | Home | `page/T_Home.qml` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `inputs-validation` | InputValidation | `page/T_TextBox.qml` | 1 | 用法与状态；卡片高度 210px，随字体缩放 |
| `inputs-standard-button` | Button | `component/CardHighlight.qml + ScrollablePage` | 0 | 用法与状态；卡片高度 150px，随字体缩放 |
| `inputs-filled-button` | FilledButton | `page/T_FilledButton.qml` | 2 | 用法与状态；卡片高度 150px，随字体缩放 |
| `inputs-pill-button` | PillButton | `page/T_PillButton.qml` | 2 | 用法与状态；卡片高度 250px，随字体缩放 |
| `inputs-delay-button` | DelayButton | `page/T_DelayButton.qml` | 1 | 用法与状态；卡片高度 180px，随字体缩放 |
| `iconbutton` | IconButton | `page/T_IconButton.qml` | 2 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `inputs-dropdown-button` | DropDownButton | `page/T_DropDownButton.qml` | 1 | 用法与状态；卡片高度 160px，随字体缩放 |
| `inputs-hyperlink-button` | HyperlinkButton | `page/T_HyperlinkButton.qml` | 1 | 用法与状态；卡片高度 100px，随字体缩放 |
| `inputs-toggle-button` | ToggleButton | `page/T_ToggleButton.qml` | 1 | 用法与状态；卡片高度 150px，随字体缩放 |
| `inputs-radio-button` | RadioButton | `page/T_RadioButton.qml` | 2 | 用法与状态；卡片高度 190px，随字体缩放 |
| `inputs-progress-button` | ProgressButton | `page/T_ProgressButton.qml` | 1 | 用法与状态；卡片高度 170px，随字体缩放 |
| `inputs-chip` | Chip | `page/T_Chip.qml` | 1 | 用法与状态；卡片高度 190px，随字体缩放 |
| `inputs-tag-input` | TagInputBox | `page/T_TagInputBox.qml` | 5 | 用法与状态；卡片高度 170px，随字体缩放 |
| `inputs-copyable-text` | CopyableText | `page/T_CopyableText.qml` | 1 | 用法与状态；卡片高度 340px，随字体缩放 |
| `inputs-checkbox` | Checkbox | `page/T_Checkbox.qml` | 3 | 用法与状态；卡片高度 190px，随字体缩放 |
| `inputs-slider` | Slider | `page/T_Slider.qml` | 2 | 用法与状态；卡片高度 310px，随字体缩放 |
| `inputs-toggle-switch` | ToggleSwitch | `page/T_ToggleSwitch.qml` | 1 | 用法与状态；卡片高度 190px，随字体缩放 |
| `form-time-picker` | TimePicker | `page/T_TimePicker.qml` | 3 | 用法与状态；卡片高度 160px，随字体缩放 |
| `form-date-picker` | DatePicker | `page/T_DatePicker.qml` | 2 | 用法与状态；卡片高度 160px，随字体缩放 |
| `form-calendar-picker` | CalendarPicker | `page/T_CalendarPicker.qml` | 2 | 日期、日期时间两种紧凑按钮；卡片高度 118px，随字体缩放 |
| `form-color-picker` | ColorPicker | `page/T_ColorPicker.qml` | 1 | 用法与状态；卡片高度 350px，随字体缩放 |
| `form-shortcut-picker` | ShortcutPicker | `page/T_ShortcutPicker.qml` | 0 | 用法与状态；卡片高度 160px，随字体缩放 |
| `form-textbox` | TextBox | `page/T_TextBox.qml` | 1 | 用法与状态；卡片高度 240px，随字体缩放 |
| `form-multiline-textbox` | MultiLineTextBox | `page/T_MultiLineTextBox.qml` | 1 | 用法与状态；卡片高度 250px，随字体缩放 |
| `form-scrollable-textbox` | ScrollableTextBox | `page/T_ScrollableTextBox.qml` | 1 | 用法与状态；卡片高度 250px，随字体缩放 |
| `form-password-box` | PasswordBox | `page/T_PasswordBox.qml` | 1 | 用法与状态；卡片高度 170px，随字体缩放 |
| `form-auto-suggest` | AutoSuggestBox | `page/T_AutoSuggestBox.qml` | 1 | 用法与状态；收起时 190px，展开候选时 360px，随字体缩放 |
| `form-combo-box` | ComboBox | `page/T_ComboBox.qml` | 2 | 用法与状态；卡片高度 180px，随字体缩放 |
| `form-multi-select-combo` | MultiSelectComboBox | `page/T_MultiSelectComboBox.qml` | 7 | 用法与状态；卡片高度 120px，随字体缩放 |
| `form-multi-tree-combo` | MultiTreeSelectComboBox | `component/CardHighlight.qml + ScrollablePage` | 0 | 用法与状态；卡片高度 160px，随字体缩放 |
| `form-number-box` | NumberBox | `page/T_NumberBox.qml` | 2 | 用法与状态；卡片高度 180px，随字体缩放 |
| `form-spin-box` | SpinBox | `page/T_SpinBox.qml` | 1 | 用法与状态；卡片高度 180px，随字体缩放 |
| `navigation` | NavigationView | `page/T_NavigationView.qml` | 1 | 用法与状态；卡片高度 370px，随字体缩放 |
| `breadcrumb` | Breadcrumb | `page/T_Breadcrumb.qml` | 1 | 用法与状态；卡片高度 150px，随字体缩放 |
| `navigation-stack-page` | StackPage | `page/T_StackPage.qml` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `tabbar` | TabBar | `page/T_TabBar.qml` | 0 | 用法与状态；卡片高度 150px，随字体缩放 |
| `navigation-tab-view` | TabView | `page/T_TabView.qml` | 0 | 用法与状态；卡片高度 310px，随字体缩放 |
| `navigation-segmented` | SegmentedControl | `page/T_SegmentedControl.qml` | 0 | 用法与状态；卡片高度 150px，随字体缩放 |
| `navigation-pagination` | Pagination | `page/T_Pagination.qml` | 1 | 用法与状态；卡片高度 170px，随字体缩放 |
| `navigation-multi-window` | MultiWindow | `page/T_MultiWindow.qml` | 0 | 用法与状态；卡片高度 150px，随字体缩放 |
| `pageheader` | PageHeader | `component/CardHighlight.qml + ScrollablePage` | 0 | 用法与状态；卡片高度 180px，随字体缩放 |
| `data-grid` | DataGrid | `page/T_DataGrid.qml` | 1 | 表格卡片 600px，随字体缩放；排序、编辑、多选、冻结/隐藏列、数据更新与万行加载；树页增加展开/折叠 |
| `data-tree-grid` | TreeDataGrid | `page/T_TreeDataGrid.qml` | 1 | 表格卡片 600px，随字体缩放；排序、编辑、多选、冻结/隐藏列、数据更新与万行加载；树页增加展开/折叠 |
| `data-file-tree` | FileTreeData | `page/T_FileTreeData.qml` | 0 | 用法与状态；卡片高度 370px，随字体缩放 |
| `data-timeline` | Timeline | `page/T_Timeline.qml` | 1 | 用法与状态；卡片高度 330px，随字体缩放 |
| `data-statistic` | Statistic | `page/T_Statistic.qml` | 7 | 用法与状态；卡片高度 230px，随字体缩放 |
| `data-selection` | SelectionArea | `page/T_SelectionArea.qml` | 0 | 用法与状态；卡片高度 340px，随字体缩放 |
| `data-qrcode` | QRCode | `page/T_QRCode.qml` | 1 | 用法与状态；卡片高度 450px，随字体缩放 |
| `infobar` | InfoBar | `page/T_InfoBar.qml` | 1 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `feedback-info-badge` | InfoBadge | `page/T_InfoBadge.qml` | 1 | 用法与状态；卡片高度 150px，随字体缩放 |
| `feedback-progress-bar` | ProgressBar | `page/T_ProgressBar.qml` | 2 | 用法与状态；卡片高度 190px，随字体缩放 |
| `feedback-progress-ring` | ProgressRing | `page/T_ProgressRing.qml` | 2 | 用法与状态；卡片高度 220px，随字体缩放 |
| `feedback-result` | Result | `page/T_Result.qml` | 4 | 用法与状态；卡片高度 230px，随字体缩放 |
| `feedback-rating` | RatingControl | `page/T_RatingControl.qml` | 2 | 用法与状态；卡片高度 150px，随字体缩放 |
| `feedback-shimmer` | Shimmer | `page/T_Shimmer.qml` | 0 | 用法与状态；卡片高度 210px，随字体缩放 |
| `feedback-skeleton` | SkeletonView | `page/T_SkeletonView.qml` | 0 | 用法与状态；卡片高度 330px，随字体缩放 |
| `layouts-split` | SplitLayout | `component/CardHighlight.qml + ScrollablePage` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `layouts-staggered` | StaggeredGridView | `component/CardHighlight.qml + ScrollablePage` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `layouts-flow` | FlowLayout | `component/CardHighlight.qml + ScrollablePage` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `layouts-adaptive-grid` | AdaptiveGrid | `component/CardHighlight.qml + ScrollablePage` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `layouts-dock` | DockLayout | `component/CardHighlight.qml + ScrollablePage` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `layouts-page-stack` | PageStack | `page/T_StackPage.qml` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `layouts-form-row` | FormRow | `component/CardHighlight.qml + ScrollablePage` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `surfaces-page` | ForgePage | `component/CardHighlight.qml + ScrollablePage` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `surfaces-round-image` | RoundImageView | `component/CardHighlight.qml + ScrollablePage` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `surfaces-icon-label` | IconLabel | `page/T_IconLabel.qml` | 2 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `surfaces-expander` | Expander | `page/T_Expander.qml` | 1 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `surfaces-watermark` | Watermark | `page/T_Watermark.qml` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `surfaces-marquee` | Marquee | `component/CardHighlight.qml + ScrollablePage` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `card` | Card | `component/CardHighlight.qml + ScrollablePage` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `forge-window` | ForgeWindow | `component/CardHighlight.qml + ScrollablePage` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `popups-menu` | Menu | `page/T_Menu.qml` | 1 | 用法与状态；卡片高度 280px，随字体缩放 |
| `popups-content-dialog` | ContentDialog | `page/T_ContentDialog.qml` | 1 | 用法与状态；卡片高度 200px，随字体缩放 |
| `popups-dialog-window` | DialogWindow | `page/T_DialogWindow.qml` | 1 | 用法与状态；卡片高度 150px，随字体缩放 |
| `popups-tooltip` | ToolTip | `page/T_ToolTip.qml` | 1 | 用法与状态；卡片高度 100px，随字体缩放 |
| `popups-drawer` | Drawer | `page/T_Drawer.qml` | 1 | 用法与状态；卡片高度 150px，随字体缩放 |
| `chart-bar` | Bar Chart | `chart/T_BarChart.qml` | 1 | 370px 图表、图例、悬停、选择、更新/清空；类型对应选项 |
| `chart-line` | Line Chart | `chart/T_LineChart.qml` | 1 | 370px 图表、图例、悬停、选择、更新/清空；类型对应选项 |
| `chart-pie` | Pie Chart | `chart/T_PieChart.qml` | 1 | 370px 图表、图例、悬停、选择、更新/清空；类型对应选项 |
| `chart-polar` | Polar Area Chart | `chart/T_PolarAreaChart.qml` | 1 | 370px 图表、图例、悬停、选择、更新/清空；类型对应选项 |
| `chart-bubble` | Bubble Chart | `chart/T_BubbleChart.qml` | 1 | 370px 图表、图例、悬停、选择、更新/清空；类型对应选项 |
| `chart-scatter` | Scatter Chart | `chart/T_ScatterChart.qml` | 1 | 370px 图表、图例、悬停、选择、更新/清空；类型对应选项 |
| `chart-radar` | Radar Chart | `chart/T_RadarChart.qml` | 1 | 370px 图表、图例、悬停、选择、更新/清空；类型对应选项 |
| `chart-professional` | ForgePlot | 专业二维图表扩展 | 1 | 图表菜单第一项；首页和搜索均可进入；560px 示例随字号缩放；十种绘图模式、实时数据、多轴/多绘图区、极坐标与五种格式导出 |
| `theme-acrylic` | Acrylic | `page/T_Acrylic.qml` | 1 | 用法与状态；卡片高度 420px，随字体缩放 |
| `theme-liquid-glass` | Liquid Glass | `page/T_LiquidGlass.qml` | 0 | 用法与状态；卡片高度 420px，随字体缩放 |
| `theme-colors` | Colors | `page/T_Colors.qml` | 0 | 用法与状态；卡片高度 190px，随字体缩放 |
| `theme-typography` | Typography | `page/T_Typography.qml` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `theme-icons` | Icons | `page/T_Icons.qml` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `other-tour` | Tour | `page/T_Tour.qml` | 2 | 用法与状态；卡片高度 460px，随字体缩放 |
| `other-syntax` | SyntaxView | `page/T_SyntaxView.qml` | 0 | 用法与状态；卡片高度 570px，随字体缩放 |
| `other-dial` | Dial | `page/T_Dial.qml` | 1 | 用法与状态；卡片高度 250px，随字体缩放 |
| `about` | About | `page/T_About.qml` | 0 | 用法与状态；卡片高度 160px，随字体缩放 |
| `settings` | Settings | `page/T_Settings.qml` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
| `lifecycle` | 页面生命周期 | `component/CardHighlight.qml + ScrollablePage` | 0 | 分区演示；按内容调整卡片高度，移除 API 展示文字 |
