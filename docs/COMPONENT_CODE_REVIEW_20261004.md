# 组件代码审查与修复（2026-10-04）

> 阶段记录：本文保留 v0.1.0 审查时的测试数量和结果；v0.1.1 当前验收以时间戳发布目录的 ACCEPTANCE.md 和 verification 为准。

本轮确认并修复 9 类问题，新增 9 个 Rust 回归测试，并补充 C++ 导航、基础图表和 ForgePlot 原生测试。最终 Rust 工作区 46 项测试、C++ 13 项测试全部通过；两套 Gallery 各完成 96 页、308 个状态的实际渲染检查。

## 范围与基线

- 项目：`D:\ai_workspace\SlintForge\SlintForge`；Windows x64、Slint 1.18.0、fluent 样式。
- 检查公共 Slint 入口、控件数值/禁用状态、布局几何、导航树、表格排序/编辑/选择、基础图表几何，以及 ForgePlot 生命周期、增量缓存、绘制顺序与绑定。
- Rust/C++ 共用 Slint 契约；导航和基础图表算法的修复同步到两端，共用绘图核心仅修改一次。
- 保留已有修改及历史验证目录；参考工程 `FluentUI-Pro-main`、真实第三方实现和生成代码不属于本轮修改范围。
- 当前目录没有 Git 元数据。审查前保存 111 个源码文件的 SHA-256；本轮修改其中 15 个，另新增 2 个测试文件。源码差异清单及最终程序校验值位于本轮证据目录。
- 修复前原有 Rust 基线测试通过；新增用例揭示了原测试未覆盖的边界。

证据统一保存到 `build/component-review-20261004/`。

## 问题与解决结果

| 编号 | 优先级 | 复现或依据 | 修复结果 |
|---|---|---|---|
| R1 | P2 | NumberBox 的范围为 -10～10，宿主赋值 25 后仍返回 25；初始 `value=25, minimum=5, maximum=10` 也不归一化。Dial、Rating 存在相同的初始化回调缺口。 | NumberBox 外部赋值执行范围约束；NumberBox、Dial、Rating 增加初始化归一化。宿主归一化不触发用户操作回调。 |
| R2 | P2 | Dial 收到 `delta_x=20, delta_y=0` 时，值从 40 错误降到 35；负步长与方向键的行为不一致。 | 横向/零竖向滚动及非正步长返回 reject，既不改值也不发出回调；正常正负竖向滚动仍有效。 |
| R3 | P2 | FormRow 说明换行后，控件容器仍按固定 64px 起点计算高度；回归窗口中控件底部 276px，行高度仅 230px。 | 控件容器高度改用实际 `control-origin-y` 计算，消除底部裁切；原有高控件和横向布局测试继续通过。 |
| R4 | P2 | SplitLayout 的外部比例、负分隔宽度或超大分隔宽度使第二块原点超出组件边界。 | 绘制使用 0～1 的有效比例；分隔宽度限制到可用轴长度。两块与分隔条均保持在组件内；水平/垂直拖动原有 0.15～0.85 约束不变。 |
| R5 | P2 | AutoSuggestBox 展开后禁用，`open` 仍为 true；禁用状态还可被宿主重新展开。 | 初始化、禁用及 open 变化均维护关闭状态，禁用的建议列表不会占据布局。 |
| R6 | P1 | 导航树逐节点重新检查整个父链，深层链校验为 O(n²)，展开使用递归。旧版 2,000 层链创建单次采样约 576.9ms。 | 两端使用三色标记做 O(n) 校验，显式栈按原先顺序展开；通过 20,000 层、逆序输入、折叠和独立环检测测试。 |
| R7 | P2 | 基础分类图表的 `x=-1,y=10000` 和 `x=2,y=-10000` 没有对应类别，却参与 A/B 两个有效类别的自动范围，结果范围变为 -10000～10000。 | 自动范围与实际绘制使用相同类别域；没有有效类别点时返回空数据状态。Rust/C++ 同步修复。 |
| R8 | P2 | 手动范围端点均有限，但 `maximum-minimum` 溢出为无穷时仍返回正常状态，产生无效归一化坐标/刻度。 | 两端额外校验 X/Y 跨度有限，溢出返回无效范围状态 2。 |
| R9 | P1 | ForgePlot 从折线切为参数曲线时继续使用原折线排序，输出与新建同类型场景不同。继续追加会使旧索引长度小于数据长度，存在越界读取风险；从其他类型切回折线也没有建立排序。 | 创建、追加共用排序维护函数；类型变化时清除或重建顺序，断点段分别稳定排序，保留原始索引和已追加历史；仅外观变化仍复用缓存。 |

R1 的回归先验证外部赋值，再扩大覆盖确认初始化缺口，第二轮修复完成后通过。其余问题在第一轮修复后通过。开发与复验均由本任务执行，没有超过三轮的自动修复项。

## 涉及实现

- 数值与禁用状态：`ui/form/number-box.slint`、`ui/form/auto-suggest.slint`、`ui/other/dial.slint`、`ui/feedback/rating.slint`。
- 布局：`ui/layouts/form-row.slint`、`ui/layouts/split-layout.slint`。
- 导航：`sdk/rust/src/lib.rs`、`sdk/cpp/src/ForgeSdk.cpp`、`sdk/cpp/include/ForgeSdk.hpp`。
- 基础图表：`sdk/rust/src/charts.rs`、`sdk/cpp/src/ForgeCharts.cpp`。
- 共用绘图引擎：`sdk/common/ForgePlotCore.cpp`。
- 新测试：`sdk/rust/tests/component_review.rs`、`Gallery/rust/tests/component_review.rs`；同步扩展 `tests/cpp/ForgeSdkTests.cpp`、`ForgeChartTests.cpp`、`ForgePlotTests.cpp`。
- 更新组件 API、基础图表及专业图表文档，README 增加本轮记录入口。

## 最终验证

| 检查 | 结果 | 证据 |
|---|---|---|
| Rust 工作区测试 | 46/46；包含表格、容器、布局、输入、弹层、图表、SDK 和本轮回归 | `workspace-tests.log` |
| 新增专项回归 | SDK 4 项、共享 UI 5 项；类型切换含 4 种转换和各自继续追加 | `regressions-final.log` |
| C++ 构建与测试 | CLion MSVC/Qt 环境构建成功；13/13 测试通过，约 189s，包含实际窗口消费者 | `final-cpp-build.log`、`ctest-final.log` |
| Gallery 实际渲染 | 每端 96 个深色中文、96 个浅色英文/125% 字号、96 个窄窗口中文和 20 个操作状态；共 616 张 PNG；覆盖及截图时间均核对通过 | `gallery-final/{rust,cpp}`、`gallery-validation.log` |
| 图例对齐 | 56 张实际截图、160 个图例色块检查通过 | `legend-validation.log` |
| 几何一致性 | 基础图表 41 行、403 个字段差异为 0；表格 10 个场景、59 条契约记录一致 | `gallery-validation.log`；`tools/verify_grids.py` |
| 组件独立编译 | 66 个用法示例通过；96 页目录、译文及第一方 C++ namespace 规则通过 | `static-checks.log` |
| 生成绑定 | Plot bindings、FFI 检查通过 | `tools/generate_plot_bindings.py --check`、`tools/generate_plot_ffi.py --check` |
| 实时测量光标 | 更新后的 Rust 独立示例，两光标共 10 步拖动期间穿插 10 次实时更新，通过 | `plot-cursor-drag.log`、`plot-cursor-drag/drag.csv` |
| 原生绘图专项 | 增量百万点历史、峰谷保留、图例 8 方位/3 样式、768 种布局组合及类型切换通过 | `plot-benchmark.log` |

人工图像复核查看全部 96 页深色概览，并查看两端 12 个重点页面的深/浅/窄窗和相关操作状态。概览位于 `visual-review/`。截图覆盖检查表示渲染和输出完整，不等同于逐个手工操作所有功能。

### 本机性能采样

导航链 2,000 层的创建从约 **576.9ms** 降至 **2.25ms**；20,000 层创建约 **22ms**、展开约 **10ms**。这是同机单次采样，用于确认平方级遍历已消除，不是固定时限承诺。

原生 ForgePlot 场景生成：

| 输入点数 | 降采样 | 输出点数 | 场景生成 | 1,000 次命中查询 |
|---:|---|---:|---:|---:|
| 100,000 | 开 | 1,604 | 9.76ms | 1.15ms |
| 1,000,000 | 开 | 2,462 | 15.54ms | 1.14ms |
| 100,000 | 关 | 100,000 | 318.09ms | 1.15ms |
| 1,000,000 | 关 | 1,000,000 | 3,215.72ms | 1.11ms |

百万格色图场景生成约 126ms。以上包含原生几何与 SVG 场景生成，不包含 Slint 栅格化、完整窗口帧或显示器刷新；不能据此宣称完整界面达到某个 FPS。原始记录为 `plot-benchmark/core-performance.csv`、`map-performance.csv`。

## 复验命令

在项目根目录运行：

```powershell
cargo test --workspace --offline --locked
cargo test -p slintforge -p slintforge-gallery-rust --test component_review --offline --locked -- --nocapture
```

C++ 使用 `build/component-review-20261004/build-cpp.cmd`，该脚本加载 `D:\.clion-env\qt-6.8.3-msvc.bat` 并构建 `build/cpp`。运行 CTest 前将 Slint SDK 的 `lib` 加入 PATH，将 `SLINTFORGE_COMPONENT_QA_DIR`、`SLINTFORGE_PLOT_QA_DIR` 指向新的输出目录：

```powershell
ctest --test-dir build/cpp -C Debug --output-on-failure
build/cpp/Debug/slintforge_plot_tests.exe --benchmark
target/debug/slintforge-gallery-rust.exe --components-smoke
target/debug/slintforge-plot-example.exe --cursor-drag-smoke
```

本轮 Python 检查使用 `C:\Users\wzp\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe`；下面的 `python` 代表该解释器。截图、图例与双宿主基础图表几何复验使用：

```powershell
python tools/verify_gallery_presentation.py --geometry --qa-root build/component-review-20261004/gallery-final --host rust --host cpp
python tools/verify_chart_legends.py --qa-root build/component-review-20261004/gallery-final --host rust --host cpp
python tools/verify_grids.py
```

## 更新后的程序与边界

已更新常用 Rust/C++ Gallery 以及 Rust ForgePlot 独立示例；对应路径和 SHA-256 位于 `artifacts-sha256.json`。正在运行的旧进程需要关闭后重新启动才能加载本轮代码。

本轮结果针对上述 Windows 构建和项目内可复验场景。没有制作新的安装发布包，也没有重新验收其他操作系统或任意调用方定制内容。
