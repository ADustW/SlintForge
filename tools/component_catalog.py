# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

"""维护 Gallery 的组件状态，并检查导航、演示和公共导出是否一致。"""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
NATIVE = {
    "inputs-standard-button": "Button",
    "inputs-filled-button": "Button.primary",
    "inputs-hyperlink-button": "StyledText / Platform.open-url",
    "inputs-toggle-button": "Button.checkable / checked",
    "inputs-radio-button": "RadioGroup",
    "inputs-copyable-text": "TextEdit.read-only / select-all / copy",
    "inputs-checkbox": "CheckBox",
    "inputs-slider": "Slider",
    "inputs-toggle-switch": "Switch",
    "form-textbox": "LineEdit",
    "form-multiline-textbox": "TextEdit",
    "form-scrollable-textbox": "TextEdit.content-x / content-y",
    "form-password-box": "LineEdit.input-type=password",
    "form-combo-box": "ComboBox",
    "form-spin-box": "SpinBox",
    "navigation-tab-view": "TabWidget",
    "navigation-multi-window": "Window / 宿主 show、hide",
    "data-selection": "TextEdit.read-only / 文本选择与复制",
    "feedback-progress-bar": "ProgressIndicator",
    "feedback-progress-ring": "Spinner",
    "popups-menu": "ContextMenuArea / Menu / MenuItem",
    "popups-dialog-window": "Dialog / StandardButton",
    "popups-tooltip": "Tooltip",
    "theme-colors": "Palette",
}
EXISTING_DEMOS = {
    "navigation": "ForgeNavigationView", "breadcrumb": "ForgeBreadcrumb",
    "navigation-stack-page": "ForgePageStack / ForgeStackPage", "tabbar": "ForgeTabBar",
    "pageheader": "ForgePageHeader",
}
CUSTOM = {
    "flow-basic": "ForgeFlow",
    "flow-custom": "ForgeFlowCanvas",
    "flow-flowchart": "ForgeFlow",
    "flow-topology": "ForgeFlow",
    "flow-mindmap": "ForgeFlow",

    "form-time-picker": "ForgeTimePicker",
    "form-date-picker": "ForgeDatePicker",
    "form-calendar-picker": "ForgeCalendarPicker",
    "inputs-validation": "ForgeInputValidation",
    "inputs-pill-button": "ForgePillButton",
    "inputs-delay-button": "ForgeDelayButton",
    "inputs-dropdown-button": "ForgeDropDownButton",
    "inputs-progress-button": "ForgeProgressButton",
    "inputs-chip": "ForgeChip",
    "inputs-tag-input": "ForgeTagInputBox",
    "form-color-picker": "ForgeColorPicker",
    "form-shortcut-picker": "ForgeShortcutPicker",
    "form-auto-suggest": "ForgeAutoSuggestBox",
    "form-multi-select-combo": "ForgeMultiSelectComboBox",
    "form-multi-tree-combo": "ForgeMultiTreeSelectComboBox",
    "form-number-box": "ForgeNumberBox",
    "navigation-segmented": "ForgeSegmentedControl",
    "navigation-pagination": "ForgePagination",
    "data-file-tree": "ForgeFileTree",
    "data-timeline": "ForgeTimeline",
    "data-statistic": "ForgeStatistic",
    "data-qrcode": "ForgeQRCode",
    "feedback-info-badge": "ForgeInfoBadge",
    "feedback-result": "ForgeResult",
    "feedback-rating": "ForgeRatingControl",
    "feedback-shimmer": "ForgeShimmer",
    "feedback-skeleton": "ForgeSkeletonView",
    "popups-content-dialog": "ForgeContentDialog",
    "popups-drawer": "ForgeDrawer",
    "theme-acrylic": "ForgeAcrylic",
    "theme-liquid-glass": "ForgeLiquidGlass",
    "other-tour": "ForgeTour",
    "other-syntax": "ForgeSyntaxView",
    "other-dial": "ForgeDial",
    "chart-bar": "ForgePlot",
    "chart-line": "ForgePlot",
    "chart-pie": "ForgePlot",
    "chart-polar": "ForgePlot",
    "chart-bubble": "ForgePlot",
    "chart-scatter": "ForgePlot",
    "chart-radar": "ForgePlot",
    "chart-professional": "ForgePlot",
    "data-grid": "ForgeDataGrid",
    "data-tree-grid": "ForgeTreeDataGrid",
}
DEFERRED = set()

def rows():
    return [line.split("|") for line in (ROOT / "Gallery/data/navigation.txt").read_text(encoding="utf-8").splitlines() if line.startswith("P|")]

def main():
    navigation = rows()
    deferred = DEFERRED
    lines = ["# id|状态(existing/native/custom/deferred)|组件或原生替代"]
    for row in navigation:
        key = row[1]
        status, api = ("deferred", "表格，按用户要求暂缓") if key in deferred else ("native", NATIVE[key]) if key in NATIVE else ("custom", CUSTOM[key]) if key in CUSTOM else ("existing", EXISTING_DEMOS.get(key, row[3]))
        lines.append(f"{key}|{status}|{api}")
    manifest = ROOT / "Gallery/data/component-status.txt"
    data = "\n".join(lines) + "\n"
    if "--check" in sys.argv:
        # 导航各层级必须提供中文名称，避免中文模式回退到英文组件名。
        for line in (ROOT / "Gallery/data/navigation.txt").read_text(encoding="utf-8").splitlines():
            if not line or line.startswith('#'):
                continue
            fields = line.split('|')
            if fields[0] != 'S':
                assert len(fields) == 7 and re.search(r'[\u4e00-\u9fff]', fields[6]), f"缺少导航中文名称: {fields[1]}"
        assert manifest.read_text(encoding="utf-8") == data, "组件状态目录未同步"
        sources = "\n".join(p.read_text(encoding="utf-8") for p in (ROOT / "Gallery/ui").rglob("*.slint"))
        routes = set(re.findall(r'active-route-id == "([^"]+)"', sources))
        for row in navigation:
            assert row[1] in deferred or row[1] in routes, f"缺少演示页面: {row[1]}"
        public = (ROOT / "ui/slintforge.slint").read_text(encoding="utf-8")
        for api in CUSTOM.values():
            assert re.search(r"\b" + api + r"\b", public), f"缺少公共导出: {api}"
        print(f"COMPONENT_CATALOG_OK pages={len(navigation)} native={len(NATIVE)} custom={len(CUSTOM)} deferred={len(deferred)}")
    else:
        manifest.write_text(data, encoding="utf-8")
        print(f"CATALOG_WRITTEN pages={len(navigation)} native={len(NATIVE)} custom={len(CUSTOM)} deferred={len(deferred)}")

if __name__ == "__main__":
    main()
