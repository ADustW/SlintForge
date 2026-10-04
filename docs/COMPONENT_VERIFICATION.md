# Gallery 组件与图表阶段验证记录

日期：2026-10-03。工程根目录：`D:\ai_workspace\SlintForge\SlintForge`。

此记录保留表格实现前的阶段结果；当前完整结果见 [表格验证记录](GRIDS_VERIFICATION.md)。

阶段状态：验证完成。Gallery 的 95 个路由中，93 个已有演示，仅 DataGrid 和 TreeDataGrid 两个表格页面暂缓。7 类图表已实现，全部展示页已参考本地 FluentUI-Pro Gallery 调整，页面不显示用法代码面板。

## 审查与实现结果

前置对话 `01a0fd07-7f1f-70e3-b894-7d5ee1d401d4` 完成后，先审查 Slint 原生能力并补齐非表格、非图表组件。随后按本轮要求实现图表，演示页面由 86 个增加到 93 个。

| 当时分类 | 页面数 | 处理 |
|---|---:|---|
| Slint 原生能力 | 27 | 使用原生控件展示，不增加重复 Forge 控件 |
| 新增共享组件 | 38 | 原 31 个组件与本轮 7 类图表，提供 Rust/C++ 接入 |
| 已有演示、导航及辅助页 | 28 | 保留行为并调整展示结构 |
| 暂缓表格 | 2 | `data-grid`、`data-tree-grid` |

图表包括柱状图、折线图、饼图、极区图、气泡图、散点图和雷达图。共享 Slint 层绘制图形，两端 SDK 提供数据校验、几何计算与命中测试。API 及模型更新约定见 [图表文档](CHARTS.md)。

原生审查依据见 [组件审查](COMPONENT_AUDIT.md)，当前分类见 [共享状态清单](../Gallery/data/component-status.txt)，逐页展示变更见 [95 页展示清单](GALLERY_PRESENTATION.md)。

## 展示页调整

参考本地 `FluentUI-Pro-main` 的 ScrollablePage、CardHighlight 及对应组件页面，统一页边距、分区间距、卡片圆角、边框与内边距。预览高度按内容指定并随字体缩放；按钮、勾选框、滑块和输入框展示常用及禁用状态。

移除展示页中的 API 文字、用法代码、代码展开与复制用法面板。63 段使用示例保存在 `examples/component-usage`，可独立编译。SyntaxView 页仍演示语法高亮、文本编辑与原文复制，这是该组件本身的功能。

复核中修正了平滑折线填充边界、鼠标悬停事件、进度环演示、英文换行翻译，以及大字体图标目录的横向溢出。C++ 独立消费者发现的默认空模型访问问题已在服务绑定中修正，并增加空模型和空嵌套模型的回归检查。

## 构建环境

- Windows x64；Slint、slint-build、C++ SDK 固定为 1.18.0，fluent 样式。
- C++ 配置、构建和检查加载 `D:\.clion-env\qt-6.8.3-msvc.bat`，使用 CLion 2026.1.1 的 CMake，实际编译器为 MSVC 19.51。
- Rust 使用锁定依赖离线构建。窗口使用 winit 后端和 software 渲染器；交互测试使用 Slint 官方测试后端。
- Python 检查使用 Codex 工作区依赖加载器提供的 Python。独立二维码解码器位于 `build/qa-python`。
- 工程目录没有 Git 仓库。原组件与本轮图表修改前快照分别保存在 `build/backups/components_20261003_002327` 和 `build/backups/charts_gallery_20261003_074616`。

## 最终检查

| 检查 | 最终结果 | 证据 |
|---|---|---|
| Rust 工作区 | 24 项通过，0 失败 | `build/charts-workspace-tests.log` |
| Rust 构建与整库窗口 | 构建成功，302 帧完成 | `build/charts-rust-build.log`、`build/charts-rust-gallery-render.log` |
| C++ 构建与 CTest | 构建成功，9/9 通过，171.10 秒 | `build/charts-cpp-all.log`、`build/charts-cpp-ctest.log` |
| 分类、公共导出与 C++ namespace 规则 | 95 路由、27 原生、38 新增、2 暂缓；通过 | `build/charts-static-final.log` |
| 翻译同步 | 通过 | `Gallery/i18n/sync.py --check` |
| 63 段独立使用示例 | 全部编译通过 | `build/charts-static-final.log`、`build/component-qa/snippets/results.txt` |
| 图表跨宿主几何 | 41 行、403 个数值字段，最大差异 0 | `build/charts-presentation-final.log`、`build/charts-qa/*-geometry.csv` |
| 全部页面与截图完整性 | 两端各 302 帧，共 604 帧；集合、尺寸与更新时间通过 | `build/charts-presentation-final.log` |
| 两端独立图表消费者 | 7 类公共组件均通过并生成 PNG | `build/charts-qa/consumer-{rust,cpp}-*.png` |
| 安装后的 C++ 消费者 | 独立配置、构建、7 类图表运行通过 | `build/charts-installed-consumer-build.log`、`build/charts-qa/installed-run/installed-charts.log` |
| 安装文件与源码一致性 | 59 个共享 Slint 文件与图表头文件一致 | `build/charts-final-evidence.log` |

最终计数、日志校验与安装文件核对记录在 `build/charts-final-evidence.log`；本轮被验证源码的 SHA-256 清单位于 `build/charts-qa/final-source-sha256.json`。

### 图表数据与交互

Rust/C++ 数学检查覆盖分组柱图、正负值分别堆叠、非有限值、缺失点、隐藏系列、手动范围和空数据；同时检查单项完整圆、环形中心排除、极区面积比例、气泡像素尺寸与雷达闭合。两端生成同一几何合同并逐字段比对。

实际 Slint 交互测试覆盖悬停提示、点击选择、方向键与 Home/End/Escape、禁用、图例隐藏与恢复、模型替换、嵌套模型更新后 refresh、无效范围和全部 7 种图表。C++ 消费者额外调用默认空模型、空点模型及空命中模型，确认服务绑定安全处理 Slint 的空模型指针。

消费者共享 `examples/ui/charts.slint`，通过公共 `@slintforge` 入口实例化全部 7 个组件。折线消费者启用平滑曲线与面积填充；Gallery 操作状态另检查堆叠柱图、环形饼图及各类选中反馈。

### 其他组件回归

保留并通过已有的按钮取消与重复触发、输入验证、标签与快捷键、下拉多选、文件树、分页、评分、旋钮、骨架、弹层焦点与引导交互检查。

SDK 检查仍覆盖二维码、语法着色、背景模糊和折射、捕获与 PNG 输出。二维码输入为 `SlintForge|Rust|C++`、`中文组件🙂|SlintForge` 和 `https://slint.dev`；两端 RGBA 像素一致，独立解码器均还原原文。第三方来源见 [Nayuki 来源记录](../third_party/qrcodegen/SOURCE.md)。

### 实际窗口复核

每个宿主的 302 帧由以下状态组成：

| 状态 | 帧数 | 窗口 |
|---|---:|---|
| 深色中文 | 95 | 1280×900，字体 100% |
| 浅色英文 | 95 | 1100×800，字体 125% |
| 窄窗口中文 | 95 | 960×720，字体 100% |
| 弹层、忙碌及图表操作 | 17 | 深色中文 |

17 个操作状态覆盖日期、时间、建议列表、进度按钮、操作下拉、多选、树多选、对话框、抽屉、引导及 7 类图表。检查模式不读写用户设置；每张 PNG 必须晚于对应可执行文件，旧版截图保存在归档目录中。

复核全部页面的三组汇总和操作状态，并在原始尺寸检查图标目录、进度环、英文换行、平滑折线与环形饼图。图片位于 `build/component-qa/{rust,cpp}`，汇总位于 `build/component-qa/contact-sheets`。

[图表及弹层操作汇总](../build/component-qa/contact-sheets/cpp-opened-2.png) · [英文反馈组件汇总](../build/component-qa/contact-sheets/rust-light-7.png) · [窄窗口图标与辅助页](../build/component-qa/contact-sheets/cpp-narrow-12.png)

### 安装消费检查

C++ SDK 安装到 `build/component-qa/install`，使用安装后的 `find_package(SlintForge)` 与 UI 库路径独立配置 `examples/cpp`，构建目录为 `build/component-qa/external-cpp`。

核对全部 59 个共享 Slint 文件和 `ForgeCharts.hpp` 与源码一致。独立消费者的 `--charts-smoke` 完成全部 7 类图表检查；运行日志及 PNG 位于 `build/charts-qa/installed-run`。既有 widgets 与 capture 消费者检查也在 CTest 中通过。

## 复现

在工程根目录执行：

```powershell
cargo test --workspace --offline --locked
cargo build --workspace --offline --locked
$env:SLINT_BACKEND = 'winit-software'
& '.\target\debug\slintforge-gallery-rust.exe' --components-smoke
& '.\target\debug\slintforge-example-rust.exe' --charts-smoke

cmd.exe /d /c build\charts-cpp-all.cmd
$env:PATH = 'D:\ai_workspace\SlintForge\slint-sdk-1.18.0\lib;' + $env:PATH
& 'C:\Program Files\JetBrains\CLion 2026.1.1\bin\cmake\win\x64\bin\ctest.exe' --test-dir build\cpp -C Debug --output-on-failure

cmd.exe /d /c build\components-installed-consumer.cmd
& '.\build\component-qa\external-cpp\Debug\slintforge_example_cpp.exe' --charts-smoke

$env:PYTHONIOENCODING = 'utf-8'
& 'C:\Users\wzp\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe' Gallery\i18n\sync.py --check
& 'C:\Users\wzp\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe' tools\verify_components.py --compiler 'D:\ai_workspace\SlintForge\slint-sdk-1.18.0\bin\slint-compiler.exe' --qr
& 'C:\Users\wzp\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe' tools\verify_gallery_presentation.py --geometry --host rust --host cpp
```

本记录对应本机 Debug 构建、交互与窗口验证。DataGrid、TreeDataGrid 继续暂缓；本轮交付为源码、文档和验证产物。
