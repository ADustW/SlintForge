# ForgePlot 功能、性能与交付验证

日期：2026-10-03。工程：`D:\ai_workspace\SlintForge\SlintForge`。验证平台：Windows x64、Slint 1.18.0、fluent；实际窗口显式使用 `SLINT_BACKEND=winit-software`。C++ 构建使用 `D:\.clion-env\qt-6.8.3-msvc.bat`、CLion 内置 CMake、Visual Studio 18 2026 x64。

`ForgePlot` 已加入组件库、Rust/C++ SDK、Gallery 和两个独立示例。默认轴、自动范围、图例、提示、平移、缩放和选择可直接使用；应用创建窗口后绑定一次宿主服务。接入及完整字段说明见 [专业图表 API](PLOT.md)，能力边界见 [实施范围](PLOT_IMPLEMENTATION.md)，网格接手结果见 [网格审查](GRID_REVIEW.md)。

## 功能与审查后的修复

| 范围 | 验证内容与修复 |
|---|---|
| 图形 | 折线、三种阶梯、脉冲、十种点符号、参数曲线、基线/系列间填充、分组和正负堆叠柱、统计箱及离群点、蜡烛/OHLC、色图、极坐标；金融图形自动范围不采纳未使用的 y 字段 |
| 填充与对数柱 | 系列间填充只使用两条曲线重叠的有效区间，保留 NaN 断线，参考系列隐藏时取消填充；对数柱的零/异号基线裁到坐标有效边界 |
| 坐标 | f64 线性、正负对数、反向坐标、四边多轴、多绘图区、数字/文本/UTC 日期时间/时长/π刻度、误差棒范围；映射往返和非法输入回归 |
| 色图 | 四种调色板、对数色阶、最近邻/双线性插值、反向和对数坐标、无效单元透明、色标；零权重的 NaN 邻居不污染有效像素 |
| 图元 | 九种图元，数据/比例/像素坐标、九个锚点、循环与缺失锚点拒绝、层次及裁剪、图片解码与导出 |
| 命中与选择 | 阶梯/脉冲按实际几何命中，统计箱离群点和金融矩形可命中/框选，金融命中值使用 close；矩形选择查询完整原始数据，零容差和精确边界命中修复 |
| UI 交互 | 指针、滚轮、Shift 框选缩放、Ctrl 框选/点选、多选、拖动、键盘、禁用和方向限制；数据、轴、图例及选择回调 |
| 图例与演示 | 长图例按可用宽度换行并省略，命中仍保留完整名称；数据刷新保留图例显隐，切换模式恢复对应坐标轴，图例回调在模型更新后触发 |
| 生命周期 | 窗口服务自动释放场景和输入缓存，渲染错误清理旧缓存；动态删除组件需按 API 调用 release；模型发布和实时采样采用有界缓冲 |
| 导出 | PNG/JPEG/BMP/SVG/PDF，Unicode 文件名；SVG/PDF 使用实际场景，包含选择、色图和图片；PDF 的中文字体轮廓、透明掩码及图片实际渲染检查 |

十种示例模式依次为曲线/误差/填充、参数曲线、柱图、统计箱、蜡烛、OHLC、色图、时间轴/图片、极坐标、双绘图区。消费者每次捕获之后平移和缩放，再切换下一模式，检查切换后的场景仍有效。

## 检查结果

| 检查 | 结果 | 证据 |
|---|---|---|
| Rust 工作区 | 33 项通过，0 失败；不计 0 项 doc-test | `build/plot-qa/workspace-tests.log` |
| 独立 Rust 示例回归 | 1 项通过；图例隐藏后追加 20 点、变更图形模式和坐标轴恢复 | `build/plot-qa/demo-tests.log` |
| C++ CTest | 13/13 通过；含共用绘图引擎、网格、Gallery 和消费者 | `build/plot-qa/workspace-cpp-tests.log` |
| 生成绑定与静态契约 | FFI/绑定生成器 check 通过；66 段独立示例编译；翻译、分类和第一方 namespace 规则通过 | `build/plot-qa/public-contracts.log`、`final-checks.log` |
| 当前 Gallery 实际窗口 | Rust/C++ 每端 308 帧，共 616 帧；96 页全部实现，0 暂缓 | `build/plot-qa/gallery/{rust,cpp}`、`gallery-verification.log` |
| 独立图表消费者 | 两端各 10 种模式；C++ 从安装后的 SDK 配置、构建并运行独立工程 | `build/plot-qa/consumer-{rust,cpp}.log`、`{rust,cpp}-consumer` |
| 导出内容 | 20 个场景、22 个 SVG、22 个 PDF、6 个其他位图格式文件验证通过 | `build/plot-qa/export-verification.log`、`verification-results.json` |
| PDF 实际渲染 | 两端各 10 个 PDF 共 20 个，经 Poppler 渲染为非空 PNG | `build/plot-qa/pdf-render/{rust,cpp}` |
| 安装内容 | 4 个关键头文件、5 个关键 UI 契约与当前源码字节一致，安装后消费者运行通过 | `export-verification.log`、`install`、`cpp-consumer-build` |

截图状态为 96 页深色中文、96 页浅色英文/125% 字号、96 页 960×720 窄窗口中文，以及 20 个操作帧。程序逐文件检查集合、解码、尺寸和生成时间。人工复核了专业图表深色/英文大字号/窄窗口/极坐标、独立示例及中文/色图/极坐标 PDF 等重点画面；没有把截图文件存在视为所有页面逐帧人工验收。

两端十种模式的 SVG 字节完全一致；中文命名导出的 SVG 也一致。PNG/BMP 的解码 RGB 像素完全一致；JPEG 检查解码、尺寸和有效图像，不要求不同编码器的 JPEG 字节一致。PDF 检查页数、尺寸、矢量路径、图片和透明掩码，再实际渲染；没有用文件扩展名或文件非空代替格式验证。

## 性能

### 共用引擎

C++ `/O2`，相同波形包含尖峰和原始索引检查。计时从数据已交给引擎之后开始，包含场景与 SVG 生成；不包含宿主输入复制、Slint 图像加载或窗口绘制。可见区约 1000 像素宽。

| 原始点数 | 关闭自适应 ms / 绘制点数 | 开启自适应 ms / 绘制点数 |
|---:|---:|---:|
| 100,000 | 236.373 / 100,000 | 5.804 / 1,604 |
| 1,000,000 | 2,349.930 / 1,000,000 | 23.611 / 2,462 |

降采样保留像素列首点、最小值、最大值、末点和断线，选中点及原始索引经过回归；百万点测试中的尖峰仍对应原始输入索引。1000 次尖峰附近命中查询共 0.122 ms；该查询不是所有密集重叠图形的最坏情况指标。

最近邻、线性坐标色图改为保留原网格大小的嵌入位图，避免小网格生成整块视口分辨率图像。对数或插值色图继续按屏幕坐标采样：

| 单元数 | 修改前场景 ms | 修改后 ms | 修改后 SVG 字节 |
|---:|---:|---:|---:|
| 25 | 34.546 | 0.199 | 5,103 |
| 10,000 | 35.129 | 1.184 | 58,434 |
| 1,000,000 | 36.890 | 36.556 | 1,994,061 |

原始记录：`core-performance.csv`、`map-performance-before.csv`、`map-performance.csv`、`core-final.log`。

### 实际窗口

独立示例 Release、1080×760、强制软件渲染。场景准备包含 Slint 模型读取/转换、输入复制、降采样和 SVG 图像加载；窗口捕获单独计时。表中的平移和缩放是修改视图后场景准备时间，不包含后续显示器呈现。

| 宿主 / 点数 | 首次场景准备 ms | 窗口捕获 ms | 平移准备 ms | 缩放准备 ms |
|---|---:|---:|---:|---:|
| Rust / 100,000 | 9.963 | 11.944 | 4.761 | 3.577 |
| C++ / 100,000 | 11.380 | 10.925 | 4.606 | 3.442 |
| Rust / 1,000,000 | 63.285 | 13.524 | 15.237 | 4.937 |
| C++ / 1,000,000 | 87.148 | 15.377 | 14.549 | 4.759 |

数据模型最初创建另有独立列 `model_ms`，不包含在表中；十万/百万点各形成 1,842/2,701 个可绘制点。两个宿主的场景 SVG 字节数一致。原始记录在 `{rust,cpp}-consumer/window-performance.csv` 及 `window-{rust,cpp}-perf.log`。

这些是本机固定输入的耗时样本；不作为持续 FPS、所有数据形状或所有机器的保证。实时示例采用最多 2000 点的有界队列和 33 ms 发布周期，O(1) 追加；共享组件默认 16 ms 合并刷新，视图改变复用原始输入。

## 复现

以下命令在项目根目录执行；Python 和 Poppler 来自 Codex 工作区依赖：

```powershell
$env:CARGO_TARGET_DIR = "$PWD\build\plot-qa\rust-target"
$env:SLINT_BACKEND = 'winit-software'
cargo test --workspace --offline --locked
cargo test --manifest-path examples/plot-rust/Cargo.toml --offline --locked
cargo run --release --manifest-path examples/plot-rust/Cargo.toml --offline --locked -- --smoke
cargo run --release --manifest-path examples/plot-rust/Cargo.toml --offline --locked -- --benchmark
cmd /c build\plot-qa\workspace-cpp.cmd
cmd /c build\plot-consumer-cpp.cmd
cmd /c build\plot-consumer-release.cmd
cmd /c build\plot-core-tests.cmd --benchmark
```

```powershell
$taskPython = 'C:\Users\wzp\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
$taskPoppler = 'C:\Users\wzp\.cache\codex-runtimes\codex-primary-runtime\dependencies\native\poppler\Library\bin\pdftoppm.exe'
& $taskPython tools/generate_plot_ffi.py --check
& $taskPython tools/generate_plot_bindings.py --check
& $taskPython tools/verify_components.py --compiler D:/ai_workspace/SlintForge/slint-sdk-1.18.0/bin/slint-compiler.exe
& $taskPython tools/verify_plot.py --pdftoppm $taskPoppler
& $taskPython tools/verify_gallery_presentation.py --host rust --host cpp `
  --qa-root build/plot-qa/gallery --cpp-build-dir build/plot-cpp `
  --rust-build-dir build/plot-qa/rust-target/debug
```

## 运行边界与证据固定

当前验收为 Windows x64。C++ JPEG/BMP 编码依赖 Windows WIC，其他平台需要对应编码器；没有进行 Linux/macOS 运行验收。Slint 模型数值为 f32，直接宿主引擎接口为 f64；精确时间序列请按 API 使用相对值和字符串时间基准。PDF 文字保存为字体轮廓，中文显示不依赖阅读端字体，但不可作为可搜索文字。组件参考 QCustomPlot 的二维功能体系，不提供 Qt/QCustomPlot API 兼容层。

当前源码和验证程序的 SHA-256 清单：`build/plot-qa/verified-source-sha256.json`；汇总：`build/plot-qa/final-evidence.json`。开发方的 `build/grids-qa` 固定验收文件未被覆盖。工作目录没有 Git 仓库，交付为工作区代码、安装测试产物、示例与验证记录。
