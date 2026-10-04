# Gallery 新增组件 API

基线为 Slint 1.18.0、fluent 样式，所有视觉实现位于 `ui/`，Rust/C++ 使用同一个 `@slintforge` 入口。96 个导航页面均已有演示，无暂缓页面。DataGrid 和 TreeDataGrid 的模型、编辑、选择及服务绑定见 [表格组件](GRIDS.md)。7 类基础图表见 [基础图表组件](CHARTS.md)，专业二维图表 ForgePlot 见 [专业图表组件](PLOT.md)。原生替代清单见 [组件审查](COMPONENT_AUDIT.md)，逐项机器可读状态见 `Gallery/data/component-status.txt`。

## 输入与表单

| 组件 | 主要属性、函数与回调 |
|---|---|
| `ForgeInputValidation` | `text`、`required`、`minimum-length`、`maximum-length`、`error-message`、`touched`；输出 `valid`、`validation-message`；`edited(text)`、`accepted(text)`。长度按 Unicode 字符计数；外部错误也会阻止提交。 |
| `ForgePillButton` | `text`、`primary`、`enabled`；输出 `pressed`；`focus-button()`、`clicked()`；鼠标、Enter、Space 和可访问性激活。 |
| `ForgeDelayButton` | `text`、`delay`、`enabled`；输出 `progress`；`clicked()`。持续按住达到时长后松开确认；提前松开、移出或禁用不提交。 |
| `ForgeDropDownButton` | `text`、`items: [ForgeChoiceItem]`、`enabled`；输出 `open`；`show()`、`close()`、`selected(id)`；原生 PopupWindow 提供外部点击和 Escape 关闭。 |
| `ForgeProgressButton` | `text`、`busy`、`progress`、`enabled`；`clicked()`。busy 阻止重复提交，progress<0 为不确定状态。 |
| `ForgeChip` | `text`、`selected`、`removable`、`enabled`；`toggled(bool)`、`removed()`。 |
| `ForgeTagInputBox` | 外部 `tags: [string]`、`draft`、`maximum-tags`、`enabled`；`submit()`、`add-requested(text)`、`remove-requested(index)`。应用更新模型；Gallery 示例执行去空白、去重和数量限制。 |
| `ForgeColorPicker` | `red`、`green`、`blue`、`alpha`；输出 `value: color`；`color-changed(color)`。输出通道限制到 0～255，滑块和整数框联动。 |
| `ForgeShortcutPicker` | `shortcut`、`recording`、`enabled`；`start-recording()`、`clear()`、`shortcut-changed(text)`。支持 Ctrl/Alt/Shift/Meta、方向键和 F1～F12；Escape 保留原值，单独修饰键不提交。 |
| `ForgeAutoSuggestBox` | `text`、`suggestions: [string]`、`open`、`enabled`；`choose(text)`、`edited(text)`、`selected(text)`。不区分大小写的前缀匹配；Down 展开，Escape 收起，Enter 提交当前文字；建议列表使用原生可聚焦按钮。禁用时自动收起且不能展开。 |
| `ForgeMultiSelectComboBox` | `items: [ForgeChoiceItem]`、`selection-text`、`placeholder-text`、`enabled`；输出 `open`；`show()`、`close()`、`selection-changed(id,bool)`。直接更新 items 的 selected 字段，保留关闭前的选中状态。 |
| `ForgeMultiTreeSelectComboBox` | `items: [ForgeTreeItem]`、`selection-text`、`enabled`；`show()`、`close()`、`expansion-changed(id,bool)`、`selection-changed(id,bool)`。父子选择和可见模型由应用回传。 |
| `ForgeNumberBox` | 浮点 `value`、`minimum`、`maximum`、`step`、`enabled`；输出 `input-valid`；`set-value(float)`、`commit()`、`value-changed(float)`。初始值、外部赋值及范围变化限制到 `[minimum, max(minimum, maximum)]`；这些归一化操作不触发用户操作回调。支持小数步进与上下键，非法输入恢复当前值。整数场景使用原生 SpinBox。 |

## 导航、数据与反馈

| 组件 | 主要属性、函数与回调 |
|---|---|
| `ForgeSegmentedControl` | `items: [ForgeChoiceItem]`、`selected-id`、`enabled`；`selected(id)`。内部复用原生 Button 的选择、焦点和键盘行为。 |
| `ForgePagination` | `total-items`、`page-size`、`current-page`、`enabled`；输出 `page-count`；`go-to(page)`、`page-changed(page)`。页码从 1 开始，空数据/非正 page-size 使用第 0 页，页码变化自动限制范围。 |
| `ForgeDataGrid`、`ForgeTreeDataGrid` | `rows`、`columns`、`filter`、排序、多选、列宽、冻结/隐藏列和编辑；树组件增加同级排序、祖先路径筛选与展开。需要绑定 `ForgeGridServices`，完整契约见 [表格 API](GRIDS.md)。 |
| `ForgePlot` | 默认轴、专业系列、多轴/绘图区、色图、图元、选择、缩放、平移和五种格式导出；绑定 `ForgePlotServices` 后即可使用，完整契约见 [专业图表 API](PLOT.md)。 |
| `ForgeFileTree` | `items: [ForgeTreeItem]`、`multi-select`、`enabled`；`expansion-changed(id,bool)`、`selection-changed(id,bool)`。组件展示扁平可视树，不读取文件系统。 |
| `ForgeTimeline` | `items: [ForgeTimelineItem]`；显示时间、标题、描述和完成状态。 |
| `ForgeStatistic` | `title`、格式化字符串 `value`、`suffix`、`trend`、`show-trend`。数值精度和本地化由应用决定。 |
| `ForgeQRCode` | `text`；输出 `result: ForgeQRCodeResult`；`regenerate()`。SDK 返回实际 QR 像素，包含四模块静区和 UTF-8 ECI；空文本、NUL 和超容量输入返回错误。 |
| `ForgeInfoBadge` | `value`、`maximum`、`dot`、`show-zero`。默认 maximum=99，超限显示 99+。 |
| `ForgeResult` | `level`、`title`、`description`、`action-text`；`action-requested()`。成功、错误、警告、信息四种状态。 |
| `ForgeRatingControl` | 整数 `value`、`maximum`、`read-only`、`enabled`；`set-value(int)`、`value-changed(int)`。初始值及外部赋值限制到 `[0, max(0, maximum)]`。支持鼠标、方向键、Home 和 End。 |
| `ForgeShimmer` | `running`、`cycle`；输出 `phase`。暂停保留相位，非正周期静止。 |
| `ForgeSkeletonView` | `loading`、`animate`、`rows`、`@children`。加载时遮住真实内容；切换不会销毁子实例。 |

## 弹层、材质与其他

| 组件 | 主要属性、函数与回调 |
|---|---|
| `ForgeContentDialog` | `open`、`title`、`message`、`primary-text`、`cancel-text`、`primary-enabled`、`dismiss-on-outside`、`@children`；`dismiss()`、`accepted()`、`canceled()`。遮罩阻止背景鼠标输入，Escape 取消，Tab 在操作按钮之间循环。 |
| `ForgeDrawer` | `open`、`title`、`panel-width`、`edge: ForgeDrawerEdge`、`dismiss-on-outside`、`@children`；`dismiss()`、`dismissed()`。左右两侧、外部点击和 Escape 关闭，内容常驻。 |
| `ForgeAcrylic` | `source`、`blur-radius`、`refraction`、`tint`、`content-padding`、`@children`；输出 `processed`；`refresh()`。SDK 对应用提供的图片作透明度加权模糊，Slint 合成圆角、染色与内容。 |
| `ForgeLiquidGlass` | 继承 ForgeAcrylic，默认较低模糊半径、边缘折射与内高光。 |
| `ForgeTour` | `steps: [ForgeTourStep]`、`active`、`current-step`；`start()`、`next()`、`previous()`、`dismiss()`；`step-changed(index)`、`finished()`、`dismissed()`。目标坐标相对组件，start 可在任意状态重新开始。 |
| `ForgeSyntaxView` | `code`、`language`、`line-numbers`、`font-size`；输出 `lines: [ForgeSyntaxLine]`；`refresh()`、`copy()`。支持 Slint/Rust/C/C++/Python/JSON 的词法高亮、跨行块注释、行号和原始文本复制。 |
| `ForgeDial` | `value`、`minimum`、`maximum`、`step`、`enabled`、`suffix`；输出 `needle-angle`；`set-value(float)`、`value-changed(float)`。初始值及外部赋值遵守最小/最大范围。支持上下拖动、竖向滚轮、方向键、Home/End；横向滚动及非正步长不改变数值。 |

内容对话框、抽屉和引导应放在业务页面最后，尺寸覆盖需要阻挡的区域；关闭后子内容保留。图库示例覆盖当前页面，导航区域仍由 Gallery 主窗口管理。

材质效果处理的是显式 source 图片。需要动态背景时，由应用更新该图；本实现不自动采集桌面，也不提供系统窗口背板。语法视图提供词法颜色而非语言服务器或编辑器；未知语言使用普通颜色。

## 公共模型

`ForgeChoiceItem`：`id`、`label`、`enabled`、`selected`。默认结构字段值为 false/空字符串，应用应明确设置 enabled。

`ForgeTreeItem`：`id`、`parent-id`、`label`、`depth`、`folder`、`expanded`、`selected`、`visible`、`enabled`。模型顺序为显示顺序；depth 控制缩进，visible 控制折叠后的可视状态。Gallery 的宿主示例根据父子关系刷新深度/可见性，并排除禁用节点后传播多选。

`ForgeTimelineItem`：`title`、`description`、`time`、`completed`。`ForgeTourStep`：`title`、`description`、`x`、`y`、`width`、`height`。`ForgeQRCodeResult`：`ok`、`image`、`modules`、`error`。`ForgeSyntaxLine`：`number`、`content: styled-text`。

## 两端服务接入

纯 Slint 控件直接导入即可；二维码、高亮和材质需要为对应窗口绑定 `ForgeWidgetServices`。宿主必须在 UI 线程绑定和调用；source 更新或输入变化时同步执行处理。较大图片或高频采样可在应用中预处理后回传图片。

```slint
import { ForgeQRCode } from "@slintforge";
export { ForgeWidgetServices, ForgeQRCodeResult, ForgeSyntaxLine } from "@slintforge";
export component App inherits Window {
    width: 320px; height: 320px;
    ForgeQRCode { text: "SlintForge"; }
}
```

```rust
slint::include_modules!();
use slint::ComponentHandle;
let ui = App::new()?;
slintforge::bind_widget_services!(ui, ForgeWidgetServices, ForgeQRCodeResult, ForgeSyntaxLine);
ui.run()?;
```

```cpp
#include "app.h"
#include "ForgeWidgets.hpp"
auto ui = App::create();
forge_bind_widget_services<ForgeWidgetServices, ForgeQRCodeResult, ForgeSyntaxLine>(ui);
ui->run();
```

可运行的两端消费者位于 `examples/ui/widgets.slint`，运行已有消费者的 `--widgets-smoke` 参数验证服务绑定、生成结果和窗口截图。C++ 安装包导出 `ForgeWidgets.hpp` 与静态实现；与 Slint 相同，消费工程需能加载 `slint_cpp.dll`。

底层 API：Rust 为 `qr_code_pixels`、`highlight_syntax`、`process_backdrop`；C++ 为 `forge_qr_code_image`、`forge_highlight_syntax`、`forge_process_backdrop`。模糊半径限制为 0～32；像素使用非预乘 RGBA8，滤波时按透明度加权，避免透明区域带入颜色光晕。
