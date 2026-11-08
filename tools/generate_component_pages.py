# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""生成逐组件演示页；运行后可直接编辑生成的 .slint 文件。"""
from pathlib import Path
import json
import re
from component_catalog import ROOT, CUSTOM, NATIVE, EXISTING_DEMOS
from gallery_presentation import prepare_presentations
from license_header import with_license_header

STD = 'Button, CheckBox, Slider, Switch, RadioGroup, LineEdit, TextEdit, ComboBox, SpinBox, TabWidget, Date, Time, Palette, ProgressIndicator, Spinner, StandardButton'
FORGE = ', '.join(sorted(set(CUSTOM.values()) | {'ForgeDateTime', 'ForgeChoiceItem', 'ForgeTreeItem', 'ForgeTourStep', 'ForgeTimelineItem', 'ForgeInfoLevel', 'ForgeDrawerEdge', 'ForgeTypography', 'ForgeNavigationView', 'ForgeNavigationItem', 'ForgeNavItemKind', 'ForgeBreadcrumb', 'ForgeTabBar', 'ForgePageHeader'}))
DEMOS = {}

def add(key, body, note, extra='', overlay='', snippet=''):
    DEMOS[key] = dict(body=body, note=note, extra=extra, overlay=overlay, snippet=snippet)

for key, primary in [('inputs-standard-button', False), ('inputs-filled-button', True)]:
    add(key, f'Button {{ text: @tr("点击按钮"); primary: {str(primary).lower()}; clicked => {{ root.count += 1; }} }}\nText {{ text: @tr("点击次数：{{}}", root.count); color: Palette.foreground; }}', '直接使用 Button；primary=true 使用主色。', snippet=f'Button {{ text: "Button"; primary: {str(primary).lower()}; }}')
add('inputs-toggle-button', 'Button { text: @tr("切换按钮"); checkable: true; checked <=> root.flag; }\nText { text: root.flag ? @tr("已选中") : @tr("未选中"); color: Palette.foreground; }', 'Button.checkable=true；checked 双向绑定，原生键盘与焦点行为。', snippet='Button { text: "Toggle"; checkable: true; }')
add('inputs-radio-button', 'RadioGroup { RadioButton { text: @tr("方案一"); } RadioButton { text: @tr("方案二"); } RadioButton { text: @tr("方案三"); enabled: false; } selected(value) => { root.status = value; } }\nText { text: root.status; color: Palette.foreground; }', 'RadioGroup 管理 RadioButton 的互斥、方向键与焦点。', snippet='RadioGroup { RadioButton { text: "A"; } RadioButton { text: "B"; } }')
add('inputs-hyperlink-button', 'StyledText { text: @markdown("[Slint 官方 API](https://docs.slint.dev/latest/docs/slint/)"); default-color: Palette.foreground; link-clicked(url) => { Platform.open-url(url); } }', 'StyledText 原生渲染链接，Platform.open-url 处理用户激活。', snippet='StyledText { text: @markdown("[Slint](https://slint.dev)"); link-clicked(url) => { Platform.open-url(url); } }')
for key in ['inputs-copyable-text', 'data-selection']:
    add(key, 'selectable := TextEdit { text: @tr("可选择、复制的文本。\n使用鼠标选择，或按 Ctrl+A、Ctrl+C。\n此示例使用 Slint 原生 TextEdit。"); read-only: true; height: 160px; }\nButton { text: @tr("复制全部"); clicked => { selectable.select-all(); selectable.copy(); } }\nLineEdit { placeholder-text: @tr("在此粘贴以验证复制"); }', '只读 TextEdit 保留鼠标/键盘选择与复制，无需再实现选择层。', snippet='TextEdit { text: "Selectable text"; read-only: true; }')
add('inputs-checkbox', 'CheckBox { text: @tr("启用通知"); checked <=> root.flag; }\nCheckBox { text: @tr("禁用选项"); enabled: false; }\nText { text: root.flag ? @tr("已启用") : @tr("已关闭"); color: Palette.foreground; }', '原生 CheckBox.checked 双向绑定。', snippet='CheckBox { text: "Enable"; checked: true; }')
add('inputs-slider', 'Slider { minimum: 0; maximum: 100; value <=> root.number; }\nText { text: @tr("当前值：{}", round(root.number)); color: Palette.foreground; }', '原生 Slider.minimum、maximum、value、changed。', snippet='Slider { minimum: 0; maximum: 100; value: 40; }')
add('inputs-toggle-switch', 'Switch { text: @tr("启用功能"); checked <=> root.flag; }\nText { text: root.flag ? @tr("开启") : @tr("关闭"); color: Palette.foreground; }', '直接使用 Switch 的原生开关行为。', snippet='Switch { text: "Enable"; checked: true; }')
add('form-time-picker', 'Button { text: @tr("选择时间"); clicked => { picker.open(); } }\nCheckBox { text: @tr("24 小时制"); checked <=> root.use-24-hour-format; }\nText { text: ForgeDateTime.format-time(root.selected-time, root.language, root.use-24-hour-format); color: Palette.foreground; }', 'ForgeTimePickerPopup 支持中英文、时钟与数字输入及 12/24 小时格式。', extra='in property<string> language: "zh";\nin-out property<bool> use-24-hour-format: true;\nin-out property<Time> selected-time: {hour: 9, minute: 30, second: 0};', overlay='picker := ForgeTimePickerPopup { x: 30px; y: 100px; language: root.language; title: @tr("选择时间"); use-24-hour-format <=> root.use-24-hour-format; time: root.selected-time; accepted(value) => { root.selected-time = value; } }', snippet='in-out property<string> language: "zh";\nButton { text: root.language == "en" ? "Time" : "时间"; clicked => { picker.open(); } }\npicker := ForgeTimePickerPopup { language: root.language; time: {hour: 9, minute: 30, second: 0}; accepted(value) => { debug(value); } }')
for key in ['form-date-picker', 'form-calendar-picker']:
    add(key, 'Button { text: @tr("选择日期"); clicked => { picker.open(); } }\nText { text: ForgeDateTime.format-date(root.selected-date, root.language); color: Palette.foreground; }', 'ForgeDatePickerPopup 支持中英文月份、星期、日期格式以及年月切换和确认取消。', extra='in property<string> language: "zh";\nin-out property<Date> selected-date: {year: 2026, month: 10, day: 3};', overlay='picker := ForgeDatePickerPopup { x: 30px; y: 100px; language: root.language; title: @tr("选择日期"); date: root.selected-date; accepted(value) => { root.selected-date = value; } }', snippet='in-out property<string> language: "zh";\nButton { text: root.language == "en" ? "Date" : "日期"; clicked => { picker.open(); } }\npicker := ForgeDatePickerPopup { language: root.language; date: {year: 2026, month: 10, day: 8}; accepted(value) => { debug(value); } }')
add('form-textbox', 'LineEdit { text <=> root.draft; placeholder-text: @tr("请输入文本"); }\nText { text: root.draft; color: Palette.foreground; wrap: word-wrap; }', '直接使用 LineEdit，沿用原生选区、输入法和撤销行为。', snippet='LineEdit { placeholder-text: "Input"; }')
for key in ['form-multiline-textbox', 'form-scrollable-textbox']:
    add(key, 'TextEdit { text: @tr("第一行\n第二行\n第三行\n第四行\n第五行\n第六行\n第七行\n第八行\n第九行\n第十行"); height: 170px; }', 'TextEdit 自带多行编辑及滚动视口，无需包装。', snippet='TextEdit { text: "Line 1\\nLine 2"; }')
add('form-password-box', 'LineEdit { input-type: root.flag ? InputType.password : InputType.text; text <=> root.draft; placeholder-text: @tr("输入密码"); }\nCheckBox { text: @tr("隐藏密码"); checked <=> root.flag; }', 'LineEdit.input-type=password；切换类型保留输入。', snippet='LineEdit { input-type: password; }')
add('form-combo-box', 'ComboBox { model: ["Slint", "Rust", "C++"]; selected(value) => { root.status = value; } }\nText { text: root.status; color: Palette.foreground; }', '原生 ComboBox 提供单选、弹出列表和键盘选择。', snippet='ComboBox { model: ["Slint", "Rust", "C++"]; }')
add('form-spin-box', 'SpinBox { minimum: -10; maximum: 100; value <=> root.count; }\nText { text: @tr("整数：{}", root.count); color: Palette.foreground; }', '整数步进使用原生 SpinBox；浮点数使用 ForgeNumberBox。', snippet='SpinBox { minimum: 0; maximum: 100; value: 10; }')
add('navigation-tab-view', 'TabWidget { height: 260px; Tab { title: @tr("概览"); VerticalLayout { padding: 20px; Text { text: @tr("原生标签内容"); color: Palette.foreground; } LineEdit { placeholder-text: @tr("切换标签保留输入"); } } } Tab { title: @tr("详情"); VerticalLayout { padding: 20px; Text { text: @tr("第二个标签页"); color: Palette.foreground; } } } }', 'TabWidget / Tab 提供原生标签栏、页面选择和焦点管理。', snippet='TabWidget { Tab { title: "A"; Text { text: "Page A"; } } Tab { title: "B"; Text { text: "Page B"; } } }')
add('navigation-multi-window', 'Button { text: @tr("打开原生窗口"); clicked => { GalleryDemoModels.show-window(false); } }\nButton { text: @tr("隐藏示例窗口"); clicked => { GalleryDemoModels.close-windows(); } }', 'Window 由宿主持有并执行 show()/hide()，隐藏后保留实例。', snippet='export component ToolWindow inherits Window { width: 420px; height: 240px; Text { text: "Tool"; } }')
add('feedback-progress-bar', 'Slider { minimum: 0; maximum: 100; value <=> root.number; }\nProgressIndicator { progress: root.number / 100; indeterminate: !root.flag; height: 12px; }\nSwitch { text: @tr("确定进度"); checked <=> root.flag; }', '原生 ProgressIndicator 支持确定和不确定进度。', snippet='ProgressIndicator { progress: 0.45; }')
add('feedback-progress-ring', 'Spinner { width: 64px; height: 64px; visible: root.flag; }\nSwitch { text: @tr("显示加载动画"); checked <=> root.flag; }', '不确定进度圆环直接使用原生 Spinner。', snippet='Spinner { width: 48px; height: 48px; }')
add('popups-menu', 'Rectangle { height: 170px; background: Palette.control-background; border-radius: 8px; Text { text: @tr("在此区域右击打开原生菜单"); color: Palette.foreground; horizontal-alignment: center; vertical-alignment: center; } ContextMenuArea { Menu { MenuItem { title: @tr("打开"); activated => { root.status = @tr("已打开"); } } MenuItem { title: @tr("保存"); activated => { root.status = @tr("已保存"); } } MenuItem { title: @tr("禁用操作"); enabled: false; } } } }\nText { text: root.status; color: Palette.foreground; }', 'ContextMenuArea + Menu/MenuItem 已提供菜单与键盘交互。', snippet='ContextMenuArea { Menu { MenuItem { title: "Open"; activated => { debug("Open"); } } } }')
add('popups-dialog-window', 'Button { text: @tr("打开原生对话窗口"); clicked => { GalleryDemoModels.show-window(true); } }\nButton { text: @tr("隐藏示例窗口"); clicked => { GalleryDemoModels.close-windows(); } }', '原生 Dialog 与 StandardButton，宿主处理确认/取消并持有实例。', snippet='export component NativeDialog inherits Dialog { Text { text: "Confirm?"; } StandardButton { kind: ok; } StandardButton { kind: cancel; } }')
add('popups-tooltip', 'Button { text: @tr("悬停查看提示"); Tooltip { text: @tr("Slint 原生 Tooltip，支持自定义内容。"); } }', '直接使用 Tooltip，延迟、定位和消失由 Slint 处理。', snippet='Button { text: "Hover"; Tooltip { text: "Native tooltip"; } }')
add('theme-colors', 'HorizontalLayout { spacing: 12px; Rectangle { height: 90px; background: Palette.accent-background; } Rectangle { height: 90px; background: Palette.background; border-width: 1px; border-color: Palette.border; } Rectangle { height: 90px; background: Palette.control-background; } Rectangle { height: 90px; background: Palette.foreground; } }\nText { text: @tr("主色 · 背景 · 控件背景 · 前景"); color: Palette.foreground; }', '复用全局 Palette，可在设置中切换亮/暗主题。', snippet='Rectangle { background: Palette.accent-background; }')

add('inputs-validation', 'validated := ForgeInputValidation { minimum-length: 3; maximum-length: 24; placeholder-text: @tr("请输入 3～24 个字符"); touched: true; accepted(value) => { root.status = value; } }\nButton { text: @tr("验证"); clicked => { validated.touched = true; root.status = validated.valid ? @tr("验证通过") : validated.validation-message; } }\nText { text: root.status; color: Palette.foreground; wrap: word-wrap; }', 'required / minimum-length / maximum-length / error-message；valid、validation-message、accepted。', snippet='ForgeInputValidation { minimum-length: 3; maximum-length: 24; }')
add('inputs-pill-button', 'ForgePillButton { text: @tr("胶囊按钮"); clicked => { root.count += 1; } }\nForgePillButton { text: @tr("主色胶囊"); primary: true; clicked => { root.count += 1; } }\nForgePillButton { text: @tr("禁用状态"); enabled: false; }\nText { text: @tr("点击次数：{}", root.count); color: Palette.foreground; }', 'text / primary / enabled / clicked；Enter 和 Space 与鼠标行为一致。', snippet='ForgePillButton { text: "Confirm"; primary: true; }')
add('inputs-delay-button', 'hold := ForgeDelayButton { text: @tr("按住 1.2 秒确认"); delay: 1200ms; clicked => { root.count += 1; } }\nProgressIndicator { progress: hold.progress; height: 8px; }\nText { text: @tr("确认次数：{}", root.count); color: Palette.foreground; }', '按住至 delay 后松开触发；提前松开或禁用不会确认，支持键盘持续按住。', snippet='ForgeDelayButton { text: "Hold to confirm"; delay: 1200ms; }')
add('inputs-dropdown-button', 'ForgeDropDownButton { text: @tr("操作菜单"); items: [{id: "open", label: @tr("打开"), enabled: true}, {id: "save", label: @tr("保存"), enabled: true}, {id: "disabled", label: @tr("禁用操作"), enabled: false}]; selected(id) => { root.status = id; } }\nText { text: root.status; color: Palette.foreground; }', 'items: [ForgeChoiceItem] / selected(id)；点击外部或 Escape 关闭。', snippet='ForgeDropDownButton { items: [{id: "open", label: "Open", enabled: true}]; }')
add('inputs-progress-button', 'ForgeProgressButton { busy: root.busy; progress: root.number / 100; text: @tr("开始任务"); clicked => { root.number = 0; root.busy = true; } }\nProgressIndicator { progress: root.number / 100; height: 8px; }', 'busy 禁止重复提交；progress<0 显示不确定状态，进度由宿主或应用回传。', extra='in-out property<bool> busy: false;', overlay='Timer { interval: 100ms; running: root.busy && root.visible; triggered => { root.number += 5; if root.number >= 100 { root.busy = false; } } }', snippet='ForgeProgressButton { text: "Start"; busy: false; progress: 0.4; }')
add('inputs-chip', 'HorizontalLayout { spacing: 8px; ForgeChip { text: "Slint"; selected <=> root.flag; toggled(value) => { root.status = value ? @tr("已选中") : @tr("未选中"); } removable: false; } ForgeChip { text: @tr("可移除标签"); visible: !root.removed; removed => { root.removed = true; } } ForgeChip { text: @tr("禁用标签"); enabled: false; } }\nButton { text: @tr("恢复标签"); clicked => { root.removed = false; } }\nText { text: root.status; color: Palette.foreground; }', 'selected / removable / enabled；toggled(bool)、removed()。', extra='in-out property<bool> removed: false;', snippet='ForgeChip { text: "Slint"; selected: true; }')
add('inputs-tag-input', 'ForgeTagInputBox { tags: GalleryDemoModels.tags; add-requested(value) => { GalleryDemoModels.add-tag(value); } remove-requested(index) => { GalleryDemoModels.remove-tag(index); } }\nText { text: @tr("标签数量：{}", GalleryDemoModels.tags.length); color: Palette.foreground; }', 'tags 模型由应用持有；add-requested(text)、remove-requested(index)。演示去空白、去重并限制数量。', snippet='ForgeTagInputBox { tags: ["Slint", "Rust", "C++"]; }')
add('form-color-picker', 'ForgeColorPicker { height: 300px; }', 'RGBA 四通道，value 输出颜色；color-changed(color)，所有通道限制在 0～255。', snippet='ForgeColorPicker { red: 64; green: 128; blue: 224; alpha: 200; }')
add('form-shortcut-picker', 'ForgeShortcutPicker { shortcut: "Ctrl+K"; shortcut-changed(value) => { root.status = value; } }\nText { text: root.status; color: Palette.foreground; }', '支持 Ctrl/Alt/Shift/Meta 组合；Escape 取消录制，单独修饰键不提交，clear 清除。', snippet='ForgeShortcutPicker { shortcut: "Ctrl+K"; }')
add('form-auto-suggest', 'suggest := ForgeAutoSuggestBox { suggestions: ["Slint", "Slider", "SpinBox", "Switch", "Settings", "Rust", "C++"]; selected(value) => { root.status = value; } }\nButton { text: @tr("显示建议"); clicked => { suggest.open = true; } }\nText { text: root.status; color: Palette.foreground; }', '不区分大小写的前缀匹配；Down 展开，Escape 收起，Enter 提交；应用也可回传 suggestions。', snippet='ForgeAutoSuggestBox { suggestions: ["Slint", "Slider", "Switch"]; }')
add('form-multi-select-combo', 'ForgeMultiSelectComboBox { items: [{id: "rust", label: "Rust", enabled: true, selected: true}, {id: "cpp", label: "C++", enabled: true}, {id: "slint", label: "Slint", enabled: true}, {id: "disabled", label: @tr("禁用选项"), enabled: false}]; selection-text: root.draft; selection-changed(id, selected) => { root.draft = id + (selected ? " +" : " −"); } }', 'items.selected 双向更新；selection-changed(id,bool)。关闭弹出层后选中状态保留。', snippet='ForgeMultiSelectComboBox { items: [{id: "rust", label: "Rust", enabled: true}]; }')
add('form-multi-tree-combo', 'ForgeMultiTreeSelectComboBox { items: GalleryDemoModels.tree-items; selection-text: GalleryDemoModels.tree-selection; expansion-changed(id, value) => { GalleryDemoModels.expand-tree(id, value); } selection-changed(id, value) => { GalleryDemoModels.select-tree(id, value, true); } }\nText { text: GalleryDemoModels.tree-selection; color: Palette.foreground; wrap: word-wrap; }', '树模型包含父 ID、深度与可见状态；宿主执行展开和父子选择传播，禁用节点不参与选择。', snippet='ForgeMultiTreeSelectComboBox { items: []; }')
add('form-number-box', 'ForgeNumberBox { value: 12.5; minimum: -100; maximum: 100; step: 0.25; value-changed(value) => { root.number = value; } }\nText { text: @tr("当前值：{}", root.number); color: Palette.foreground; }', '浮点 value / minimum / maximum / step；回车提交、方向键步进，非法输入恢复原值。', snippet='ForgeNumberBox { value: 12.5; minimum: -100; maximum: 100; step: 0.25; }')
add('navigation-segmented', 'ForgeSegmentedControl { selected-id: "day"; items: [{id: "day", label: @tr("日"), enabled: true}, {id: "week", label: @tr("周"), enabled: true}, {id: "month", label: @tr("月"), enabled: true}]; selected(id) => { root.status = id; } }\nText { text: root.status; color: Palette.foreground; }', 'items / selected-id / selected(id)；使用原生可选按钮提供键盘行为。', snippet='ForgeSegmentedControl { items: [{id: "a", label: "A", enabled: true}, {id: "b", label: "B", enabled: true}]; selected-id: "a"; }')
add('navigation-pagination', 'pages := ForgePagination { total-items: 237; page-size: 10; page-changed(value) => { root.count = value; } }\nText { text: @tr("当前页：{}，总页数：{}", pages.current-page, pages.page-count); color: Palette.foreground; }', 'total-items / page-size / current-page / go-to；空数据返回第 0 页，首末页、页数变化均限制范围。', snippet='ForgePagination { total-items: 237; page-size: 10; }')
add('data-file-tree', 'ForgeFileTree { height: 300px; items: GalleryDemoModels.tree-items; expansion-changed(id, value) => { GalleryDemoModels.expand-tree(id, value); } selection-changed(id, value) => { GalleryDemoModels.select-tree(id, value, false); } }\nText { text: GalleryDemoModels.tree-selection; color: Palette.foreground; wrap: word-wrap; }', '文件模型由应用提供；支持目录展开、单选、多选和禁用节点，组件不读取文件系统。', snippet='ForgeFileTree { items: [{id: "root", label: "Project", folder: true, expanded: true, visible: true, enabled: true}]; }')
add('data-timeline', 'ForgeTimeline { items: [{title: @tr("创建工程"), description: @tr("共享 Slint 契约"), time: "09:00", completed: true}, {title: @tr("完成组件"), description: @tr("Rust 与 C++ 使用同一套界面"), time: "10:00", completed: true}, {title: @tr("执行验证"), description: @tr("构建、行为测试与页面截图"), time: "11:00", completed: false}]; }', 'items: [ForgeTimelineItem]；时间、描述、完成状态，长文字按自然高度换行。', snippet='ForgeTimeline { items: [{title: "Created", time: "09:00", description: "Ready", completed: true}]; }')
add('data-statistic', 'ForgeStatistic { title: @tr("已完成任务"); value: "1,280"; suffix: @tr("项"); trend: 12.8; }\nForgeStatistic { title: @tr("错误数量"); value: "3"; trend: -25; }', 'title / value / suffix / trend / show-trend；数字格式由应用提供。', snippet='ForgeStatistic { title: "Completed"; value: "1280"; trend: 12.8; }')
add('data-qrcode', 'HorizontalLayout { spacing: 20px; VerticalLayout { spacing: 12px; LineEdit { text <=> root.payload; } qr := ForgeQRCode { text: root.payload; width: 240px; height: 240px; } Text { text: @tr("模块数：{}", qr.result.modules); color: Palette.foreground; } Button { text: @tr("导出 PNG"); enabled: qr.result.ok; clicked => { root.status = GalleryDemoModels.export-image(qr.result.image); } } } }\nText { text: root.status; color: Palette.foreground; wrap: word-wrap; }', 'text / result；UTF-8 ECI、纠错与静区由 SDK 编码，PNG 可扫码验证。需绑定 ForgeWidgetServices。', extra='in-out property<string> payload: "SlintForge|Rust|C++";', snippet='ForgeQRCode { text: "SlintForge"; width: 240px; height: 240px; }')
add('feedback-info-badge', 'SpinBox { minimum: 0; maximum: 200; value <=> root.badge-value; }\nHorizontalLayout { spacing: 20px; ForgeInfoBadge { value: root.badge-value; } ForgeInfoBadge { dot: true; } ForgeInfoBadge { value: 0; show-zero: true; } }', 'value / maximum / dot / show-zero；超过上限显示 99+。', extra='in-out property<int> badge-value: 120;', snippet='ForgeInfoBadge { value: 120; maximum: 99; }')
add('feedback-result', 'ForgeResult { title: @tr("操作成功"); description: @tr("所有任务已完成，可继续下一步。"); action-requested => { root.count += 1; } }\nText { text: @tr("点击次数：{}", root.count); color: Palette.foreground; }', 'level / title / description / action-text / action-requested；支持成功、错误、警告和信息。', snippet='ForgeResult { title: "Completed"; description: "Ready"; }')
add('feedback-rating', 'rating := ForgeRatingControl { value: 3; value-changed(value) => { root.count = value; } }\nText { text: @tr("评分：{}", rating.value); color: Palette.foreground; }', '整数星级 value / maximum / read-only；鼠标与方向键、Home、End；值限制到有效范围。', snippet='ForgeRatingControl { value: 3; maximum: 5; }')
add('feedback-shimmer', 'Switch { text: @tr("运行动画"); checked <=> root.flag; }\nForgeShimmer { height: 36px; running: root.flag && root.visible; }\nForgeShimmer { height: 24px; width: 260px; running: root.flag && root.visible; }', 'running / cycle；停下后保留相位，非正周期静止显示。', snippet='ForgeShimmer { height: 28px; cycle: 1600ms; }')
add('feedback-skeleton', 'Switch { text: @tr("加载中"); checked <=> root.flag; }\nForgeSkeletonView { height: 230px; loading: root.flag; animate: root.visible; rows: 3; LineEdit { text: @tr("加载完成，输入状态保留"); } }', 'loading=false 显示 @children；隐藏内容不销毁，animate 控制骨架动画。', snippet='ForgeSkeletonView { loading: true; Text { text: "Loaded"; } }')
add('popups-content-dialog', 'Button { text: @tr("打开内容对话框"); clicked => { root.dialog-open = true; } }\nLineEdit { placeholder-text: @tr("对话框关闭后此输入保持"); }\nText { text: root.status; color: Palette.foreground; }', 'open / title / message / @children / accepted / canceled；遮罩阻止背景鼠标输入，Escape 关闭，Tab 在操作按钮间切换。', extra='in-out property<bool> dialog-open: false;', overlay='ForgeContentDialog { open <=> root.dialog-open; title: @tr("确认操作"); message: @tr("这是保留页面状态的内容对话框。"); accepted => { root.status = @tr("已确认"); } canceled => { root.status = @tr("已取消"); } }', snippet='ForgeContentDialog { open: true; title: "Confirm"; message: "Continue?"; }')
add('popups-drawer', 'Button { text: @tr("打开抽屉"); clicked => { root.drawer-open = true; } }\nText { text: root.status; color: Palette.foreground; }', 'open / edge / panel-width / @children / dismissed；点击遮罩或 Escape 关闭，隐藏时保留内容。', extra='in-out property<bool> drawer-open: false;', overlay='ForgeDrawer { open <=> root.drawer-open; title: @tr("抽屉设置"); dismissed => { root.status = @tr("抽屉已关闭"); } VerticalLayout { spacing: 14px; LineEdit { placeholder-text: @tr("输入会保留"); } Switch { text: @tr("启用功能"); } } }', snippet='ForgeDrawer { open: true; title: "Settings"; LineEdit { placeholder-text: "Value"; } }')
for key, component in [('theme-acrylic', 'ForgeAcrylic'), ('theme-liquid-glass', 'ForgeLiquidGlass')]:
    add(key, f'Image {{ source: GalleryDemoModels.backdrop-image; height: 90px; image-fit: cover; }}\n{component} {{ height: 260px; source: GalleryDemoModels.backdrop-image; VerticalLayout {{ spacing: 18px; Text {{ text: @tr("背景图上的材质效果"); color: white; font-size: 24px; font-weight: 600; }} LineEdit {{ placeholder-text: @tr("内容仍可交互"); }} Button {{ text: @tr("点击按钮"); clicked => {{ root.count += 1; }} }} Text {{ text: @tr("点击次数：{{}}", root.count); color: white; }} }} }}', 'source / blur-radius / tint / content-padding；SDK 处理背景图，动态场景需应用更新 source。玻璃模式加入边缘折射和内高光。', snippet=f'{component} {{ source: @image-url("background.png"); Text {{ text: "Content"; }} }}')
add('other-syntax', 'ComboBox { model: ["slint", "rust", "cpp", "python", "json"]; selected(value) => { root.code-language = value; } }\nTextEdit { height: 100px; text <=> root.code-text; font-family: "Consolas"; }\nForgeSyntaxView { height: 260px; code: root.code-text; language: root.code-language; }', 'code / language / line-numbers / copy；Slint、Rust、C/C++、Python、JSON 的词法高亮，未知语言使用普通颜色。', extra='in-out property<string> code-language: "slint";\nin-out property<string> code-text: "export component Example inherits Window {\\n    width: 420px; // 共享 Slint 界面\\n    Text { text: \\"Hello\\"; }\\n}";', snippet='ForgeSyntaxView { language: "rust"; code: "fn main() { println!(\\"Hello\\"); }"; }')
add('other-dial', 'HorizontalLayout { spacing: 20px; dial := ForgeDial { width: 200px; height: 200px; value <=> root.number; minimum: 0; maximum: 100; suffix: "%"; } Slider { minimum: 0; maximum: 100; value <=> root.number; } }\nText { text: @tr("当前值：{}", round(root.number)); color: Palette.foreground; }', 'value / minimum / maximum / step；上下拖动、滚轮、方向键、Home、End；支持禁用。', snippet='ForgeDial { value: 30; minimum: 0; maximum: 100; suffix: "%"; }')
add('other-tour', 'Rectangle { height: 370px; Button { x: 30px; y: 12px; text: @tr("开始引导"); clicked => { guide.active = true; } } LineEdit { x: 30px; y: 100px; width: 260px; placeholder-text: @tr("目标一：输入"); } Button { x: 30px; y: 210px; width: 180px; text: @tr("目标二：执行"); } guide := ForgeTour { steps: [{title: @tr("输入参数"), description: @tr("此处输入工程参数。"), x: 30px, y: 100px, width: 260px, height: 36px}, {title: @tr("执行任务"), description: @tr("确认后执行下一步。"), x: 30px, y: 210px, width: 180px, height: 36px}]; finished => { root.status = @tr("引导已完成"); } dismissed => { root.status = @tr("引导已跳过"); } } }\nText { text: root.status; color: Palette.foreground; }', 'steps: [ForgeTourStep] / active / current-step / next / previous；目标高亮、完成、跳过与 Escape。', snippet='ForgeTour { active: true; steps: [{title: "Input", description: "Type here", x: 20px, y: 20px, width: 200px, height: 40px}]; }')

add('navigation', 'ForgeNavigationView { height: 280px; show-app-bar: false; active-route-id: root.draft == "" ? "demo-home" : root.draft; items: [{id: "demo-home", title: @tr("首页"), kind: ForgeNavItemKind.page, enabled: true}, {id: "demo-project", title: @tr("工程"), kind: ForgeNavItemKind.page, enabled: true}]; navigate-requested(id, params) => { root.draft = id; root.status = id; } }\nText { text: root.status; color: Palette.foreground; }', '复用现有 ForgeNavigationView：items、active-route-id、navigate-requested。', snippet='ForgeNavigationView { items: []; active-route-id: "home"; }')
add('breadcrumb', 'ForgeBreadcrumb { items: [{id: "home", title: @tr("首页")}, {id: "project", title: @tr("工程")}, {id: "page", title: @tr("页面")}]; selected(id) => { root.status = id; } }\nText { text: root.status; color: Palette.foreground; }', '复用现有 ForgeBreadcrumb.items 和 selected(id)。', snippet='ForgeBreadcrumb { items: [{id: "home", title: "Home"}, {id: "page", title: "Page"}]; }')
add('tabbar', 'ForgeTabBar { items: [{id: "a", title: @tr("概览")}, {id: "b", title: @tr("详情")}, {id: "c", title: @tr("设置")}]; active-id: root.draft == "" ? "a" : root.draft; selected(id) => { root.draft = id; root.status = id; } }\nText { text: root.status; color: Palette.foreground; }', '复用现有 ForgeTabBar.items / active-id / selected(id)。', snippet='ForgeTabBar { items: [{id: "a", title: "A"}, {id: "b", title: "B"}]; active-id: "a"; }')
add('pageheader', 'ForgePageHeader { title: @tr("工程详情"); subtitle: @tr("可返回的页面标题"); can-go-back: true; back-requested => { root.count += 1; } }\nText { text: @tr("返回次数：{}", root.count); color: Palette.foreground; }', '复用现有 ForgePageHeader.title / subtitle / can-go-back / back-requested。', snippet='ForgePageHeader { title: "Project"; subtitle: "Details"; can-go-back: true; }')
add('about', 'Text { text: "SlintForge Gallery · v0.1.2"; color: Palette.foreground; font-size: 28px; font-weight: 600; }\nText { text: @tr("共享 Slint 组件 · Rust / C++ SDK\n96 个导航页面已全部提供演示。\n原生控件示例与组件 API 使用同一套 UI 契约。"); color: Palette.foreground; wrap: word-wrap; }', 'Slint 1.18.0 / fluent / Windows x64；接口与实现记录见 docs/COMPONENT_AUDIT.md。', snippet='// SlintForge 共享 UI\nimport { ForgeWindow } from "@slintforge";\nexport component App inherits ForgeWindow { title: "SlintForge"; }')

def component_name(key):
    return 'Gallery' + ''.join(part.title() for part in key.split('-')) + 'Page'

# 以下示例使用 Slint 的富文本类型，并为材质示例提供可编译的空图来源。
DEMOS['popups-tooltip']['body'] = 'Button { text: @tr("悬停查看提示"); Tooltip { Text { text: @tr("Slint 原生 Tooltip，支持自定义内容。"); color: Palette.foreground; } } }'
DEMOS['popups-tooltip']['snippet'] = 'Button { text: "Hover"; Tooltip { text: @markdown("Native tooltip"); } }'
DEMOS['other-syntax']['extra'] = 'in-out property<string> code-language: "slint";\nin-out property<string> code-text: ' + json.dumps('export component Example inherits Window {\n    width: 420px; // 共享 Slint 界面\n    Text { text: "Hello"; }\n}', ensure_ascii=False) + ';'
DEMOS['other-syntax']['snippet'] = 'ForgeSyntaxView { language: "rust"; code: "fn main() { println!(\\"Hello\\"); }"; }'
for key in ['theme-acrylic', 'theme-liquid-glass']:
    DEMOS[key]['snippet'] = CUSTOM[key] + ' { Text { text: "Content"; } }'
DEMOS['other-tour']['body'] = DEMOS['other-tour']['body'].replace('guide.active = true;', 'guide.start();')
DEMOS['form-number-box']['body'] = DEMOS['form-number-box']['body'].replace('ForgeNumberBox {', 'field := ForgeNumberBox {', 1).replace('@tr("当前值：{}", root.number)', '@tr("当前值：{}", field.value)')
PREVIEW_ACTIONS = {
    'form-date-picker': 'if open { picker.open(); } else { picker.close(); }',
    'form-calendar-picker': 'if open { picker.open(); } else { picker.close(); }',
    'form-time-picker': 'if open { picker.open(); } else { picker.close(); }',
    'form-auto-suggest': 'suggest.open = open;',
    'inputs-progress-button': 'root.busy = open;',
    'inputs-dropdown-button': 'if open { control.show(); } else { control.close(); }',
    'form-multi-select-combo': 'if open { control.show(); } else { control.close(); }',
    'form-multi-tree-combo': 'if open { control.show(); } else { control.close(); }',
    'popups-content-dialog': 'root.dialog-open = open;',
    'popups-drawer': 'root.drawer-open = open;',
    'other-tour': 'if open { guide.start(); } else { guide.dismiss(); }',
}
for key, name in [('inputs-dropdown-button', 'ForgeDropDownButton'), ('form-multi-select-combo', 'ForgeMultiSelectComboBox'), ('form-multi-tree-combo', 'ForgeMultiTreeSelectComboBox')]:
    DEMOS[key]['body'] = DEMOS[key]['body'].replace(name + ' {', 'control := ' + name + ' {', 1)

def escape_literal_newlines(source):
    output, quoted, escaped = [], False, False
    for character in source:
        if character == '\n' and quoted:
            output.append('\\n')
            continue
        output.append(character)
        if escaped:
            escaped = False
        elif character == '\\' and quoted:
            escaped = True
        elif character == '"':
            quoted = not quoted
    return ''.join(output)

def widget_imports(source, symbols, library):
    code = re.sub(r'"(?:\\.|[^"\\])*"', '', source)
    used = [name for name in symbols.split(', ') if re.search(r'\b' + re.escape(name) + r'\b', code)]
    return f'import {{ {", ".join(used)} }} from "{library}";\n' if used else ''

def generate():
    prepare_presentations(DEMOS)
    examples = ROOT / "examples/component-usage"
    examples.mkdir(parents=True, exist_ok=True)
    titles = {line.split('|')[1]: line.split('|')[3] for line in (ROOT/'Gallery/data/navigation.txt').read_text(encoding='utf-8').splitlines() if line.startswith('P|')}
    for key, demo in DEMOS.items():
        native = key in NATIVE
        api = NATIVE.get(key, CUSTOM.get(key, EXISTING_DEMOS.get(key, 'Gallery')))
        content = demo['body'] + demo['extra'] + demo['overlay']
        header = widget_imports(content, STD, 'std-widgets.slint') + widget_imports(content + ' ForgeTypography', FORGE, '@slintforge') + 'import { GallerySamplePage } from "../component/sample-page.slint";\nimport { GalleryDemoModels } from "../component/demo-models.slint";\n'
        snippet = demo['snippet'] or (api.split(' / ')[0] + ' { }')
        if key in {'form-time-picker', 'form-date-picker', 'form-calendar-picker'}:
            snippet = CUSTOM[key] + ' { x: 20px; y: 20px; language: "zh"; ' + ('minute-increment: 5;' if key == 'form-time-picker' else 'show-time: true;' if key == 'form-calendar-picker' else '') + ' }'
        example_height = 720 if key in ['form-date-picker', 'form-calendar-picker', 'form-time-picker', 'form-color-picker', 'form-shortcut-picker'] else 320
        if snippet.startswith('export '):
            snippet = widget_imports(snippet, STD, 'std-widgets.slint') + snippet
        if not snippet.startswith(('export ', '//', 'import ')):
            imported = api.split('.')[0].split(' / ')[0]
            if imported in CUSTOM.values() or imported.startswith('Forge'):
                snippet = widget_imports(snippet, STD, 'std-widgets.slint') + f'import {{ {imported} }} from "@slintforge";\nexport component Example inherits Window {{\n    width: 520px; height: {example_height}px;\n    {snippet}\n}}'
            else:
                snippet = widget_imports(snippet, STD, 'std-widgets.slint') + f'export component Example inherits Window {{\n    width: 520px; height: {example_height}px;\n    {snippet}\n}}'
        (examples / f'{key}.slint').write_text(with_license_header(snippet), encoding='utf-8')
        source = header + f'''\n// {'Slint 原生示例' if native else '共享组件演示'}：{titles[key]}。
export component {component_name(key)} inherits Rectangle {{
    in-out property<int> count: 0;
    in-out property<bool> flag: true;
    in-out property<float> number: 30;
    in-out property<string> draft;
    in-out property<string> status: @tr("就绪");
    callback prepare-preview(bool);
    prepare-preview(open) => {{ {PREVIEW_ACTIONS.get(key, '')} }}
    {('callback configure-preview(bool, bool); configure-preview(use-24-hour, clock) => { root.use-24-hour-format = use-24-hour; picker.clock-mode = clock; }' if key == 'form-time-picker' else '')}
    {demo['extra']}
    sample := GallerySamplePage {{
        title: {json.dumps(titles[key], ensure_ascii=False)};
        subtitle: @tr({json.dumps(demo['description'], ensure_ascii=False)});
        example-title: @tr({json.dumps(demo['section'], ensure_ascii=False)});
        preview-height: {demo['height_expression']} * ForgeTypography.scale;
        VerticalLayout {{
            x: 20px; y: 20px; width: Math.max(0px, Math.min(620px, sample.preview-width - 40px));
            height: Math.max(0px, sample.preview-area-height - 40px);
            spacing: 16px; alignment: start;
            {demo['body']}
        }}
    }}
    {demo['overlay']}
}}
'''
        # 选择器与延时按钮使用人工维护的参考布局，生成器只维护路由封装。
        if key not in {'form-time-picker','form-date-picker','form-calendar-picker','form-color-picker','form-shortcut-picker','inputs-delay-button'}:
            (ROOT/f'Gallery/ui/page/{key}.slint').write_text(with_license_header(escape_literal_newlines(source)),encoding='utf-8')
    screen = ROOT/'Gallery/ui/screen/main-screen.slint'
    source = screen.read_text(encoding='utf-8')
    for key in DEMOS:
        name = component_name(key)
        source = re.sub(r'^import \{ ' + name + r' \}[^\n]*\n', '', source, flags=re.M)
        source = re.sub(r'^        ' + name + r' \{[^\n]*\n', '', source, flags=re.M)
    # 按类别分组，保持页面实例常驻，并限制 MSVC 生成代码的分支嵌套深度。
    groups = {}
    for key in DEMOS:
        category = key.split('-')[0] if '-' in key else 'navigation' if key in ['navigation', 'breadcrumb', 'tabbar', 'pageheader'] else 'other'
        groups.setdefault(category, []).append(key)
    for category, keys in groups.items():
        name = 'Gallery' + category.title() + 'Pages'
        source = re.sub(r'^import \{ ' + name + r' \}[^\n]*\n', '', source, flags=re.M)
        source = re.sub(r'^        (?:[\w-]+ := )?' + name + r' \{[^\n]*\n', '', source, flags=re.M)
        header = '\n'.join(f'import {{ {component_name(key)} }} from "../page/{key}.slint";' for key in keys)
        body = '\n'.join(f'    page-{index} := {component_name(key)} {{ width: root.width; height: root.height; ' + ('language: root.language; ' if key in ['form-date-picker', 'form-calendar-picker', 'form-time-picker', 'form-color-picker', 'form-shortcut-picker'] else '') + f'visible: root.active-route-id == "{key}"; }}' for index, key in enumerate(keys))
        actions = ' '.join(f'if route == "{key}" {{ page-{index}.prepare-preview(open); }}' for index, key in enumerate(keys))
        language_property = '    in property<string> language: "zh";\n    callback configure-time-preview(bool, bool);\n    configure-time-preview(use-24-hour, clock) => { page-0.configure-preview(use-24-hour, clock); }\n    callback configure-calendar-preview(bool);\n    configure-calendar-preview(with-time) => { page-2.configure-preview(with-time); }\n' if category == 'form' else ''
        (ROOT/f'Gallery/ui/component/{category}-pages.slint').write_text(with_license_header(header) + f'\nexport component {name} inherits Rectangle {{\n    in property<string> active-route-id;\n{language_property}    callback prepare-preview(string, bool);\n    prepare-preview(route, open) => {{ {actions} }}\n{body}\n}}\n', encoding='utf-8')
    imports = '\n'.join(f'import {{ Gallery{category.title()}Pages }} from "../component/{category}-pages.slint";' for category in groups)
    instances = '\n'.join(f'        {category}-components := Gallery{category.title()}Pages {{ width: root.page-width; height: root.page-height; ' + ('language: root.language; ' if category == 'form' else '') + 'active-route-id: root.active-route-id; }' for category in groups)
    source = source.replace('import { GalleryPendingItem, GalleryPendingPage }', imports+'\nimport { GalleryPendingItem, GalleryPendingPage }')
    source = source.replace('        for pending in root.pending-items', instances+'\n        for pending in root.pending-items')
    source = source.replace('visible: root.active-route-id == "layouts-page-stack";', 'visible: root.active-route-id == "layouts-page-stack" || root.active-route-id == "navigation-stack-page";')
    source = re.sub(r'^    callback qa-prepare-component\(bool\);\n    qa-prepare-component\(open\) => \{[^\n]*\n', '', source, flags=re.M)
    actions = 'grid-components.prepare-preview(root.active-route-id, open); chart-components.prepare-preview(root.active-route-id, open); ' + ' '.join(f'{category}-components.prepare-preview(root.active-route-id, open);' for category in groups)
    source = source.replace('export component GalleryWindow inherits ForgeWindow {', 'export component GalleryWindow inherits ForgeWindow {\n    callback qa-prepare-component(bool);\n    qa-prepare-component(open) => { ' + actions + ' }')
    screen.write_text(source,encoding='utf-8')
    print(f'GALLERY_PAGES_WRITTEN new={len(DEMOS)} existing_alias=1')

if __name__ == '__main__':
    generate()
