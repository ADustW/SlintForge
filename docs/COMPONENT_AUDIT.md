# Gallery 组件审查与补齐

> 阶段记录：本文保留该组件实施时的范围、测试数量和验证结果；v0.1.1 当前能力见接入与交付说明，本次验收以时间戳发布目录中的 ACCEPTANCE.md 和 verification 为准。

审查于 2026-10-03，前置对话 `01a0fd07-7f1f-70e3-b894-7d5ee1d401d4` 已结束。
基线：Windows x64、Slint 1.18.0、fluent 样式；首次审查时导航有 95 个页面，22 个实际演示、73 个占位；图表阶段完成后为 93 个实际演示；按后续表格指令补齐 DataGrid 与 TreeDataGrid，目前 95 页均有演示。

逐项状态保存在 `Gallery/data/component-status.txt`，Rust/C++ 共同读取；`tools/component_catalog.py --check` 校验导航、演示与公共导出。

## 原生能力

通过已安装的 1.18.0 编译器 `widgets/fluent/std-widgets.slint`、控件源码和官方 API 双重核对。
27 个页面直接使用原生能力，不新增对应 Forge 封装：普通/主色/切换按钮、单选、复制文字、复选、滑块、开关；日期/日历/时间选择器、单/多行/滚动文本、密码、组合框、整数步进；标签页、多窗口、文本选择、进度条/转圈；菜单、对话窗口、提示、主题色，以及原生富文本链接。

官方依据：
- [标准控件目录](https://docs.slint.dev/latest/docs/slint/reference/std-widgets/overview/)
- [Button](https://docs.slint.dev/latest/docs/slint/reference/std-widgets/basic-widgets/button/)
- [TextEdit](https://docs.slint.dev/latest/docs/slint/reference/std-widgets/views/textedit/)
- [日期选择器](https://docs.slint.dev/latest/docs/slint/reference/std-widgets/misc/datepicker/)
- [时间选择器](https://docs.slint.dev/latest/docs/slint/reference/std-widgets/misc/timepicker/)
- [Tooltip](https://docs.slint.dev/latest/docs/slint/reference/window/tooltip/)

`CopyableText`、`SelectionArea` 使用只读 `TextEdit` 的原生鼠标/键盘选择和复制；`RadioButton` 使用 `RadioGroup`，保留原生互斥行为；`TabView` 使用 `TabWidget`。
`CalendarPicker` 与 `DatePicker` 同用 `DatePickerPopup` 的日历界面。

2026-10-08 更新：日期、日历与时间选择页改用共享 `ForgeDatePickerPopup` / `ForgeTimePickerPopup`。Slint 1.18.0 的原生日期格式固定英文，AM/PM 文本也没有公开的语言配置，无法满足中英文即时切换；新组件使用现有 Palette、Button、ComboBox、SpinBox 和 PopupWindow，月份、星期、时段及操作按钮均支持 `language: "zh" / "en"`。以机器可读目录和 [当前组件 API](COMPONENTS.md) 为准。

## 自定义与已有组件

31 个尚无等价完整原生控件的页面需要新增共享 Slint 契约：输入验证、胶囊/延迟/下拉/进度按钮、标签与标签输入；颜色/快捷键选择、建议输入、多选与树多选、浮点数字；分段、分页；文件树、时间线、统计、二维码；徽标、结果、评分、闪烁、骨架；内容对话框、抽屉；亚克力与玻璃效果；引导、语法视图、旋钮。

5 个已有导航组件缺少 Gallery 演示，补 NavigationView、Breadcrumb、StackPage、TabBar、PageHeader 页面；补充 About 页面。
`StackPage` 复用既有 `ForgePageStack`，不创建重复实现。

主题效果接收应用提供的背景图，由宿主执行模糊/折射处理后在共享 Slint 组件中合成。动态场景需要应用更新背景图；不会自动抓取桌面或其他窗口。
文件树接收宿主提供的模型；不会在 UI 控件内部遍历本机文件系统。
二维码使用 Project Nayuki 的 MIT 实现，两种语言使用同版本源码；语法视图提供语言词法高亮和行号，保留原始文本。

## 后续图表与表格指令

后续指令先纳入 7 个 Chart 页面：柱图、折线、饼图、极区、气泡、散点与雷达；再纳入 DataGrid、TreeDataGrid。新增公共组件总数由 31 增至 40，当前没有暂缓页面。

Slint 原生 [StandardTableView](https://docs.slint.dev/latest/docs/slint/reference/std-widgets/views/standardtableview/) 已有文本单元格、当前行、排序回调和列宽调整。本次表格使用原生 ListView 的行虚拟化与滚动能力，在共享 Slint 契约中增加多选、单元格编辑、冻结/隐藏列和树形表格；普通文本表格仍直接使用原生 StandardTableView。模型与接入见 [表格文档](GRIDS.md)。
