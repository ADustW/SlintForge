"""按 FluentUI 的用法示例组织每页内容；元数据同时用于展示页核对清单。"""
from pathlib import Path

# id|卡片高度|使用说明|英文说明。高度包含内容四周各 20px 留白。
PRESENTATIONS = """inputs-standard-button|150|普通、带图标和禁用按钮，点击查看反馈。|Standard, icon, and disabled buttons with click feedback.
inputs-filled-button|150|主色按钮用于强调当前操作。|Primary buttons emphasize the current action.
inputs-toggle-button|150|点击按钮切换选中状态。|Click a button to toggle its selected state.
inputs-radio-button|190|同组内只能选择一个选项。|Choose one option in a radio group.
inputs-hyperlink-button|100|点击链接在浏览器打开页面。|Open a page in the browser through a link.
inputs-copyable-text|340|选择或复制文本，然后在输入框粘贴。|Select or copy text and paste it into the input.
data-selection|340|使用鼠标和键盘选择、复制文本。|Select and copy text with the mouse and keyboard.
inputs-checkbox|190|展示已选、未选与禁用状态。|Checked, unchecked, and disabled states.
inputs-slider|310|拖动水平或垂直滑块调整数值。|Adjust values with horizontal or vertical sliders.
inputs-toggle-switch|190|切换开关，查看开启、关闭和禁用状态。|Explore on, off, and disabled switch states.
form-time-picker|160|选择时间，并确认或取消修改。|Select a time, then confirm or cancel.
form-date-picker|160|选择日期，并确认或取消修改。|Select a date, then confirm or cancel.
form-calendar-picker|160|切换月份，在月历中选择日期。|Browse months and select a date in the calendar.
form-textbox|240|输入、只读和禁用的单行文本框。|Editable, read-only, and disabled text fields.
form-multiline-textbox|250|编辑多行文字，输入框内可滚动。|Edit multiple lines and scroll inside the field.
form-scrollable-textbox|250|滚动查看长文本，内容仍可编辑。|Scroll through long text while keeping it editable.
form-password-box|170|输入密码，并切换显示或隐藏。|Enter a password and toggle its visibility.
form-combo-box|180|展开列表，选择一个选项。|Open a list and choose an option.
form-spin-box|180|使用按钮或键盘调整整数。|Adjust an integer with buttons or the keyboard.
navigation-tab-view|310|切换标签页，输入内容会保留。|Switch tabs while preserving the entered text.
navigation-multi-window|150|打开独立窗口，再隐藏或重新显示。|Open an independent window, then hide or show it again.
feedback-progress-bar|190|确定进度与持续加载两种状态。|Determinate progress and continuous loading.
feedback-progress-ring|220|确定进度与持续加载两种状态。|Determinate progress and continuous loading.
popups-menu|280|在演示区右击，选择菜单中的操作。|Right-click the preview and choose a menu action.
popups-dialog-window|150|打开对话窗口，确认或取消操作。|Open a dialog to confirm or cancel an action.
popups-tooltip|100|将鼠标停在按钮上查看提示。|Hover over a button to display a tooltip.
theme-colors|190|在明暗主题中比较前景、背景和主色。|Compare foreground, background, and accent colors in both themes.
inputs-validation|210|输入文字后验证长度与必填规则。|Validate required text and length limits.
inputs-pill-button|250|普通、主色和禁用的胶囊按钮。|Standard, primary, and disabled pill buttons.
inputs-delay-button|180|按住按钮完成确认，提前松开会取消。|Hold to confirm; releasing early cancels.
inputs-dropdown-button|160|从按钮下拉菜单中选择操作。|Choose an action from a button menu.
inputs-progress-button|170|启动任务，按钮显示进度并避免重复提交。|Start a task and display progress while preventing duplicate submissions.
inputs-chip|190|选择、移除和恢复标签。|Select, remove, and restore chips.
inputs-tag-input|170|输入标签并按回车添加，点击叉号移除。|Press Enter to add a tag and click its close icon to remove it.
form-color-picker|350|调整四个颜色通道，预览颜色和透明度。|Adjust four color channels and preview color and opacity.
form-shortcut-picker|160|点击录制快捷键，按 Escape 取消。|Record a keyboard shortcut or press Escape to cancel.
form-auto-suggest|190|输入前缀，展开并选择匹配的建议。|Type a prefix and select a matching suggestion.
form-multi-select-combo|120|展开列表，选中多个选项。|Open the list and select multiple options.
form-multi-tree-combo|160|展开树节点，选择一个或多个子项。|Expand tree nodes and select one or more children.
form-number-box|180|输入浮点数，或按设定步长增减。|Enter a decimal value or change it by the configured step.
navigation-segmented|150|切换日、周、月三个视图。|Switch between day, week, and month views.
navigation-pagination|170|切换页码，或跳至第一页和最后一页。|Change pages or jump to the first and last page.
data-file-tree|370|展开目录，选择文件和子节点。|Expand folders and select files and child nodes.
data-timeline|330|沿时间顺序查看已完成和待完成的步骤。|Follow completed and pending steps in chronological order.
data-statistic|230|展示数值、单位、精度和趋势。|Display values, units, precision, and trends.
data-qrcode|450|输入内容生成二维码，空内容显示提示。|Generate a QR code from text or show a message for empty input.
feedback-info-badge|150|数字、提示点和状态徽标。|Numeric, dot, and status badges.
feedback-result|230|展示操作结果，以及继续处理的入口。|Display an operation result and its follow-up action.
feedback-rating|150|点击星级评分，使用键盘继续调整。|Choose a star rating and adjust it with the keyboard.
feedback-shimmer|210|开启或暂停内容加载的扫光效果。|Start or pause the loading shimmer.
feedback-skeleton|330|切换加载占位和实际内容，输入会保留。|Switch between loading placeholders and content while preserving input.
popups-content-dialog|200|在内容对话框中编辑文字，并确认或取消。|Edit text in a content dialog, then confirm or cancel.
popups-drawer|150|从侧边展开抽屉，关闭后输入会保留。|Open a side drawer and preserve input after closing.
theme-acrylic|420|在彩色背景上比较半透明材质。|Compare translucent materials over a colorful backdrop.
theme-liquid-glass|420|在背景上预览玻璃效果及内容交互。|Preview the glass effect and interact with its content.
other-syntax|570|编辑文本，切换语言并查看高亮结果。|Edit text, switch languages, and inspect syntax highlighting.
other-dial|250|旋转拨盘或拖动滑块调整同一数值。|Rotate the dial or drag the slider to adjust the same value.
other-tour|460|逐步浏览功能提示，支持返回、继续和关闭。|Follow a guided tour with back, next, and close actions.
navigation|370|展开分组、搜索并选择导航项。|Expand groups, search, and select a navigation item.
breadcrumb|150|点击路径节点返回对应层级。|Click a breadcrumb to return to its level.
tabbar|150|切换标签，关闭可移除的标签。|Switch tabs and close removable ones.
pageheader|180|组合标题、说明、返回入口和操作按钮。|Combine a title, description, back action, and controls.
about|160|SlintForge 组件与示例应用。|SlintForge components and sample application."""

def prepare_presentations(demos):
    metadata = {}
    for row in PRESENTATIONS.splitlines():
        key, height, description, english = row.split("|")
        metadata[key] = (int(height), description, english)
    assert set(demos) == set(metadata), (set(demos) - set(metadata), set(metadata) - set(demos))
    for key, demo in demos.items():
        height, description, _ = metadata[key]
        demo.update(height=height, description=description, section="用法与状态")
    # 原生行为直接复用，参考页的常用状态在同一卡片中并列展示。
    for key, primary in [("inputs-standard-button", False), ("inputs-filled-button", True)]:
        demos[key]["body"] = f'''HorizontalLayout {{ spacing: 12px; alignment: start;
            Button {{ text: @tr("点击按钮"); primary: {str(primary).lower()}; clicked => {{ root.count += 1; }} }}
            Button {{ text: @tr("带图标") + " ↗"; primary: {str(primary).lower()}; clicked => {{ root.count += 1; }} }}
            Button {{ text: @tr("禁用"); primary: {str(primary).lower()}; enabled: false; }}
        }}
        Text {{ text: @tr("点击次数：{{}}", root.count); color: Palette.foreground; }}'''
    demos["inputs-toggle-button"]["body"] = '''HorizontalLayout { spacing: 12px; alignment: start;
        Button { text: @tr("切换按钮"); checkable: true; checked <=> root.flag; }
        Button { text: @tr("已选中"); checkable: true; checked: true; }
        Button { text: @tr("禁用"); checkable: true; enabled: false; }
    }
    Text { text: root.flag ? @tr("已选中") : @tr("未选中"); color: Palette.foreground; }'''
    demos["inputs-checkbox"]["body"] = '''CheckBox { text: @tr("启用通知"); checked <=> root.flag; }
    CheckBox { text: @tr("未选中"); checked: false; }
    CheckBox { text: @tr("禁用选项"); enabled: false; checked: true; }
    Text { text: root.flag ? @tr("已启用") : @tr("已关闭"); color: Palette.foreground; }'''
    demos["inputs-toggle-switch"]["body"] += '''
    Switch { text: @tr("关闭"); checked: false; }
    Switch { text: @tr("禁用"); checked: true; enabled: false; }'''
    demos["inputs-slider"]["body"] = '''Slider { minimum: 0; maximum: 100; value <=> root.number; }
    Slider { minimum: 0; maximum: 100; value: 75; enabled: false; }
    HorizontalLayout { height: 130px; spacing: 24px; alignment: start;
        Slider { orientation: vertical; minimum: 0; maximum: 100; value <=> root.number; width: 24px; }
        Slider { orientation: vertical; minimum: 0; maximum: 100; value: 75; enabled: false; width: 24px; }
        Text { text: @tr("当前值：{}", round(root.number)); color: Palette.foreground; vertical-alignment: center; }
    }'''
    demos["form-textbox"]["body"] += '''
    LineEdit { text: @tr("只读文本"); read-only: true; }
    LineEdit { text: @tr("禁用文本"); enabled: false; }'''
    demos["feedback-progress-ring"]["body"] = '''HorizontalLayout { height: 64px; spacing: 24px; alignment: start;
        Spinner { width: 64px; height: 64px; indeterminate: true; visible: root.flag; }
        Spinner { width: 64px; height: 64px; progress: root.number / 100; }
    }
    Slider { minimum: 0; maximum: 100; value <=> root.number; }
    Switch { text: @tr("显示加载动画"); checked <=> root.flag; }'''
    for key in ["form-combo-box", "form-spin-box"]:
        demos[key]["body"] += '\n' + ('ComboBox { model: ["Slint", "Rust", "C++"]; enabled: false; }' if key.endswith("combo-box") else 'SpinBox { value: 42; enabled: false; }')
    # 文本选择和语法编辑本身属于组件功能；只移除用法代码面板。
    demos["inputs-copyable-text"]["body"] = demos["inputs-copyable-text"]["body"].replace("此示例使用 Slint 原生 TextEdit。", "选中内容可复制到其他应用。")
    demos["data-selection"]["body"] = demos["data-selection"]["body"].replace("此示例使用 Slint 原生 TextEdit。", "选中内容可复制到其他应用。")
    return metadata

def write_translations():
    catalog = Path(__file__).resolve().parents[1] / "Gallery/i18n/en.tsv"
    source = catalog.read_text(encoding="utf-8")
    existing = {line.split("|")[0] for line in source.splitlines() if "|" in line}
    pairs = [(row.split("|")[2], row.split("|")[3]) for row in PRESENTATIONS.splitlines()]
    pairs += [("基本示例", "Basic example"), ("用法与状态", "Usage and states"),
              ("带图标", "With icon"), ("只读文本", "Read-only text"), ("禁用文本", "Disabled text")]
    additions = {}
    for key,value in pairs:
        if key not in existing: additions[key] = value
    source += "\n" + "\n".join(f"{key}|{value}" for key,value in additions.items()) + "\n"
    catalog.write_text(source, encoding="utf-8")

if __name__ == "__main__":
    write_translations()
