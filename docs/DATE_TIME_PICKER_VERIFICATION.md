# 时间与日期选择器中英文切换验证

验证日期：2026-10-08。环境：Windows x64、Slint 1.18.0、fluent、Rust 1.98.0；C++ 使用 CLion CMake 和 `D:/.clion-env/qt-6.8.3-msvc.bat`。

Gallery 的「设置 → 语言」即时更新时间、日期和日历选择页，包含主示例、附加状态卡片与业务组合。月份、星期、年月日期格式、上午/下午（AM/PM）、时/分/秒、输入模式和确认/取消按钮均使用当前语言。切换语言不会重置未确认的选择。

Slint 1.18.0 原生选择器的日期格式和时段文字无法通过公开属性完成上述切换，因此新增共享 `ForgeDatePickerPopup`、`ForgeTimePickerPopup` 和 `ForgeDateTime`。实现复用现有标准按钮、组合框、整数框、PopupWindow 和 Palette，两端使用同一份 Slint 源码。独立应用绑定 `language: "zh" / "en"`，使用 `open()` 打开，在 `accepted` 中保存选择结果。完整属性与回调见 [组件 API](COMPONENTS.md)。

日期支持公历 1～9999 年、闰年、跨年翻月和年份/月选择。时间支持 12/24 小时制、时钟和数字输入；时钟分钟刻度为 5 分钟，数字输入可指定任意分、秒，保留秒值。取消后重新打开恢复已提交值。

| 验证项 | 结果与证据 |
|---|---|
| Rust 工作区回归 | 55 项通过，含 3 项新增日期/时间专项测试；`build/picker-locale-final/rust-workspace.log`。最终构建后专项复测仍为 3/3，见 `rust-picker-tests.log`。 |
| C++ 功能与 Gallery Smoke | 9/9 通过；`build/picker-locale-final/cpp-ctest.log`。 |
| 功能覆盖 | 中英文动态切换、保留未提交值、确认/取消、重新打开、闰年与世纪规则、星期、跨年及日期边界、午夜/正午和分秒保留。 |
| 独立组件用法 | 66 个示例编译通过，组件目录、翻译目录和第一方 C++ 命名规则检查通过；`component-examples.log`。 |
| 真实窗口渲染 | 两端分别 33 张，共 66 张截图；覆盖三个选择页、中文深色、英文浅色/125% 字号、960×720 窄窗口、12/24 小时及两种输入模式、附加示例与业务组合。 |
| 截图校验 | PNG 可解码、尺寸符合要求、截图晚于最终可执行文件；`picker-screenshots.log`。 |
| 两端显示一致性 | 33 对截图尺寸及 RGB 像素全部相同；`render-parity.json`。 |
| 视觉检查 | 已查看日期中英文弹层、窄窗口、英文时钟与数字输入、中文上午/下午以及业务组合的代表性截图，未发现本地化文本裁切或弹层越界。 |

所有日志和截图位于 `build/picker-locale-final/`，截图按 `rust/`、`cpp/` 分目录保存，执行结果见 `checks.json`。

专项测试为 `Gallery/rust/tests/date_time_picker.rs`、`tests/cpp/ForgeDateTimePickerTests.cpp`；两端共用 `tests/ui/date-time-picker.slint`。复测入口：

```powershell
cargo test -p slintforge-gallery-rust --test date_time_picker --locked --offline
ctest --test-dir build/cpp -C Debug --output-on-failure -R slintforge_date_time_picker_tests
```

Gallery 截图模式使用 `--components-smoke`，通过 `SLINTFORGE_COMPONENT_QA_FILTER` 分别选择 `form-date-picker`、`form-calendar-picker`、`form-time-picker`，通过 `SLINTFORGE_COMPONENT_QA_DIR` 指定截图目录。`tools/verify_gallery_presentation.py --route` 可只核对这些页面。
