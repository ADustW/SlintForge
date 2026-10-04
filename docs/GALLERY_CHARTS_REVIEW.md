# Gallery 图表入口与图例对齐复核

日期：2026-10-03。根据用户反馈，复查 ForgePlot 示例入口，以及“图表”分类下全部子页面的图例色块和说明文字。

## 问题与修改

- 常用路径的 Rust/C++ Gallery 程序比此前隔离验收构建旧。此次从当前源码重新构建 `target/debug` 与 `build/cpp/Gallery/cpp/Debug` 的 Gallery，日常启动路径已更新；此前固定源码和验收产物保持独立。
- ForgePlot 原导航名称只有“专业图表 / Professional Plot”，不能按组件名直接搜索。改为“ForgePlot 专业图表 / ForgePlot”，页面标题为 ForgePlot；首页“最近添加的示例”加入 ForgePlot、DataGrid 和 TreeDataGrid。路由 ID `chart-professional` 保持稳定。
- 七类基础图表共用的图例色块固定贴在行顶端，而原生复选框与说明文字居中。旧截图的色块相对复选框中心偏移 8 px。共享 `ui/charts/charts.slint` 改为填满图例行的容器，色块在其中居中；尺寸跟随字号比例。系列图例和饼图/极区图图例均已修改。
- Gallery README 的页面数量、暂缓状态和截图数量已同步。

## 当前检查结果

验收环境：Windows x64、Slint 1.18.0、fluent、`winit-software`；C++ 使用 CLion 的 `D:\.clion-env\qt-6.8.3-msvc.bat` 环境。

| 检查 | 结果 | 证据 |
|---|---|---|
| 常用 Gallery 构建 | Rust/C++ 均成功，输出到日常启动路径 | `build/gallery-followup-qa/{rust,cpp}-build.log` |
| Gallery smoke | 两端通过 | `{rust,cpp}-smoke.log` |
| Gallery Rust 单元测试 | 2 项通过；导航目录检查含中英文 ForgePlot 名称 | `rust-tests.log` |
| 八个图表页面 | 每端 32 帧，共 64 帧；包括深色中文、英文 125% 字号、960×720 窄窗口和操作状态 | `gallery/{rust,cpp}`、`{rust,cpp}-charts.log` |
| 图例对齐 | 七类基础图表共 56 帧、160 个图例项通过实际像素检查；色块与复选框中心误差不超过 1 px，可见字形中心误差不超过 3.5 px | `legend-after.log`、`tools/verify_chart_legends.py` |
| 旧版反例 | 同一检查器拒绝此前截图，检出柱图的 8 px 偏移 | `legend-before.log` |
| 共享契约、译文与代码规则 | 66 段独立示例编译，96 页目录和第一方 C++ namespace 检查通过 | `static-checks.log` |

表中简写证据路径均相对 `build/gallery-followup-qa`。字形升部、降部及抗锯齿会影响可见包围框中心，因此文字检查留有小幅容差。这里检查的是实际渲染结果，没有用源码中的坐标公式替代截图验证。ForgePlot 四种截图状态另有场景有效性断言。

本次重点修复用户明确指出的入口和图例对齐问题。前一轮网格、专业图表引擎、导出和大数据验证仍记录在 [专业图表验证](PLOT_VERIFICATION.md) 与 [网格审查](GRID_REVIEW.md)，其固定源码和截图用于保留当时证据。

## 复现

```powershell
$env:SLINT_BACKEND = 'winit-software'
$env:SLINTFORGE_COMPONENT_QA_FILTER = 'chart'
$env:SLINTFORGE_COMPONENT_QA_DIR = "$PWD\build\gallery-followup-qa\gallery\rust"
& target/debug/slintforge-gallery-rust.exe --components-smoke
$env:SLINTFORGE_COMPONENT_QA_DIR = "$PWD\build\gallery-followup-qa\gallery\cpp"
# 运行 C++ 前将 Slint SDK 的 lib 目录加入 PATH。
& build/cpp/Gallery/cpp/Debug/slintforge_gallery_cpp.exe --components-smoke
$taskPython = 'C:\Users\wzp\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $taskPython -X utf8 tools/verify_chart_legends.py
```

日常运行可通过首页卡片、导航搜索 `ForgePlot` 或 `--preview-route=chart-professional` 直接进入新增示例。
