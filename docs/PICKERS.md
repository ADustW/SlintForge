# 延时按钮与 Fluent 选择器 API

> **兼容提示：**本页继续保留选择器专题说明。逐组件中英文 API 见 [中文 API 站点源码](site/index.md) 与 [English API source](site/en/index.md)。

所有控件从 `@slintforge` 导入；同一套 Slint 文件用于 Rust 和 C++。默认中文，`language: "en"` 切换英文；`dark` 支持实例独立主题，`enabled` 控制输入。取消、Esc、禁用和关闭都不提交草稿。

## ForgeDelayButton

进度直接填充按钮内部。`mode` 使用 `ForgeDelayButtonMode`：

| 值 | 确认方式 |
|---|---|
| `hold` | 持续按住达到 `delay` 后触发一次 `clicked()` |
| `hold-release` | 默认值；按住达到时长后，松开才触发，提前松开取消 |
| `click` | 点击开始倒计时，再次点击取消，计时完成触发一次 |

`text`、`delay`、`primary`、`pill`、`show-progress` 控制文本、时长、主色、胶囊外形和百分比；`progress` 为 0—1，`running`、`pressed`、`has-focus` 可读取状态。`begin()`、`finish()`、`cancel()`、`focus-button()` 提供程序入口；`canceled()` 通知未完成的操作。支持空格、Enter、Esc 和焦点丢失取消，计时基于实际经过的时间。

## ForgeTimePicker

`time: Time` 与 `has-value` 为已提交状态；未选择时显示时、分占位。`use-24-hour-format`、`minute-increment`、`show-seconds` 分别控制 12/24 小时、分钟步长和秒字段。默认 12 小时制，中文上午/下午和英文 AM/PM 由 `language` 决定。滚轮支持鼠标滚轮、拖动、点击行、方向键和 PageUp/PageDown。

`accepted(Time)` 在确认时触发，`canceled()` 表示取消；`open()`、`close()`、`is-open` 控制弹层。原有 `ForgeTimePickerPopup` 保留 `set-hour()`、`set-minute()`、`set-second()`、`set-period()`、`confirm()`、`cancel()`。旧 `clock-mode` 属性现用于切换默认滚轮与兼容数字输入。

## ForgeDatePicker

`date: Date`、`has-value` 为已提交状态；中文按年/月/日排列，英文按月/日/年排列。`show-year`、`show-month`、`show-day` 控制字段；`minimum-date` 和 `maximum-date` 限定有效范围。年份变化自动修正闰年和月末，倒置范围收敛到最小日期。

`current-date` 为草稿，`set-year()`、`set-month()`、`set-day()` 只修改草稿，`confirm()` 才提交并发出 `accepted(Date)`；`cancel()`、`canceled()`、`open()`、`close()`、`is-open` 与时间选择器一致。

## ForgeCalendarPicker

`date`、`time`、`has-value` 保存已提交值，`show-time` 展开时、分、秒滚轮。按钮默认随文字和日历图标自适应宽度；两种模式未选择时均显示紧凑的“选择日期”按钮，选择后随日期文本扩展，也可显式设置 `width`。点击标题依次进入月份、年份视图，右侧上下箭头翻页，点击月份或年份返回更细的视图；日期范围与分钟步长同上。月历保留六行，中文从周一开始，英文从周日开始，相邻月份日期以弱化文字显示并受日期范围限制。仅选择日期时点击有效日期立即提交，包含时间时使用时间栏底部的确定按钮整体提交。日期与时间在发出回调前一起提交；`accepted(Date)` 和 `time-accepted(Time)` 分别通知日期、时间。`set-draft-time(Time)` 调整草稿并按分钟步长归整，`confirm()`、`cancel()` 提供程序控制。

原有 `ForgeDatePickerPopup` 保留日期计算、`choose-day()`、`browse-month()` 和草稿确认入口；它是日历弹层，分栏日期选择请使用 `ForgeDatePicker`。

## ForgeColorPicker

按钮显示当前色块及取色入口。弹层提供 HSV 色谱、调色板、RGB 通道、不透明度和十六进制输入。颜色文本接受 `#RRGGBB` 或 `#RRGGBBAA`，透明度位于最后。`red`、`green`、`blue`、`alpha` 为已提交的 0—255 通道，`value` 为已提交颜色，`draft-value` 为草稿。`set-draft(color)`、`edit-hex(string)`、`confirm()`、`cancel()` 提供编辑入口；`accepted(color)` 在确认后触发，`color-changed(color)` 通知通道变化。

十六进制处理及 Windows 屏幕取色需要绑定 `ForgeWidgetServices`，使用现有 `bind_widget_services!` / `forge_bind_widget_services` 即可。`screen-color-supported` 为真时，取色实时预览鼠标所指屏幕像素，释放启动点击后再次点击或按 Enter 选取，Esc 退出取色；仍需确认才提交。其他平台不启用屏幕取色。`picking-screen` 可读取取色状态。

## ForgeShortcutPicker

`shortcut` 保存已提交字符串。点击按钮或调用 `start-recording()` / `open()` 打开录制弹层，键盘输入修改 `draft-shortcut`，直到 `confirm()` / 保存才提交。`reset()` 恢复打开前的组合，`cancel()` / Esc 取消，`clear()` 清空已提交组合。`accepted(string)`、`shortcut-changed(string)` 通知提交；支持 Ctrl、Alt、Shift、Meta、功能键、方向键以及 Enter、Tab、空格。

## 接入示例

```slint
import { ForgeTimePicker, ForgeDatePicker, ForgeCalendarPicker, ForgeDelayButton, ForgeDelayButtonMode } from "@slintforge";
export component AppWindow inherits Window {
    VerticalLayout {
        ForgeTimePicker { language: "zh"; minute-increment: 5; }
        ForgeDatePicker { language: "zh"; }
        ForgeCalendarPicker { language: "zh"; show-time: true; }
        ForgeDelayButton { mode: ForgeDelayButtonMode.hold; text: "按住确认"; }
    }
}
```
