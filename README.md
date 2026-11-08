[简体中文](README.md) | [English](README.en.md)

# SlintForge v0.1.2

SlintForge 是一套共用 `.slint` 实现、可由 Rust 和 C++ 分别接入的桌面组件库，是独立的第三方项目，并非 Slint 官方项目。首版以 Windows x64、Slint **1.18.0**、`fluent` 样式为基线。`ui/` 是组件和公共 UI 类型的唯一来源，`ui/slintforge.slint` 是 `@slintforge` 的公共导出入口；两个 SDK 提供路由、导航树、完整图标目录、窗口／页面截图与 PNG 保存、二维码、词法高亮、背景图处理，以及 Windows 窗口边角适配。

Gallery 采用分类导航、顶栏、侧栏、首页分区与示例卡片布局；本项目的 Slint 代码和视觉元素均独立编写。

## 目录

| 路径 | 用途 |
|---|---|
| `ui/slintforge.slint` | `@slintforge` 的公共导出入口，不存放组件实现 |
| `ui/types` | 导航、内容和反馈的公共数据类型 |
| `ui/basic` | 按组件分别存放基础扩展实现 |
| `ui/navigation` | 按组件分别存放导航实现；`internal` 中的导航行和面板不对外导出 |
| `ui/layouts` | 分割、流式、自适应网格、五区停靠、页面堆叠、响应式表单行与瀑布流，按组件分别实现 |
| `ui/surfaces` | 页面截图、圆角图片、图文、折叠、水印与跑马灯容器，按组件分别实现 |
| `ui/inputs`、`ui/form` | 输入验证、扩展按钮、标签、颜色／快捷键、建议输入、多选和浮点数 |
| `ui/data`、`ui/feedback`、`ui/popups`、`ui/theme`、`ui/other` | 文件树、时间线、二维码、反馈、弹层、材质、引导、高亮和旋钮 |
| `ui/window` | 无边框窗口组件；`internal` 中的标题栏按钮不对外导出 |
| `ui/typography`、`ui/icons`、`ui/assets/fonts` | 文字字体全局配置、图标字体组件和有独立许可的字体资源 |
| `sdk/rust` | `Router`、`NavTree`、`StaggeredLayout`、`IconCatalog`、截图／PNG、窗口边角适配、构建路径 |
| `sdk/cpp` | `ForgeRouter`、`ForgeNavTree`、`ForgeStaggeredLayout`、`ForgeIconCatalog`、截图／PNG、窗口边角适配、可安装的 CMake 包 |
| `Gallery/ui/screen`、`Gallery/ui/component`、`Gallery/ui/page` | Rust/C++ 共用的窗口、Gallery 内部组件和分类页面；`gallery.slint` 仅作导出入口 |
| `Gallery/data/navigation.txt` | 两端共用的分类、层级、页面 ID 与页脚导航目录 |
| `Gallery/data/component-status.txt` | 原生、共享组件、已有演示和暂缓页面的逐项状态 |
| `Gallery/i18n` | 中英文 Gallery 译文和 PO 生成脚本 |
| `Gallery/ui/assets/brand` | Gallery 软件图标源图、PNG 与 Windows ICO |
| `Gallery/rust`、`Gallery/cpp` | 两个薄宿主；采样线程、路由和 UI 状态同步 |
| `examples/rust`、`examples/cpp` | 最小消费者，共用 `examples/ui/minimal.slint` |

Gallery 的注册页面均提供实际演示，包含原生控件、共享组件、导航、基础图表、ForgePlot、表格和 ForgeFlow。“节点与连线”位于“图表”之后，提供基础编辑、自定义卡片、流程图、拓扑图和思维导图五页。导航与示例数量由目录统计。Gallery 默认使用 Skia，两端共用设置、语言及界面契约。详见 [ForgeFlow 接入](docs/FLOW.md)、[组件 API](docs/COMPONENTS.md)、[表格 API](docs/GRIDS.md)、[基础图表 API](docs/CHARTS.md)、[ForgePlot API](docs/PLOT.md)。

ForgePlot 参考 QCustomPlot 的专业二维绘图能力，提供曲线、阶梯/脉冲、散点、误差棒、系列间填充、分组/堆叠柱、统计箱、金融图、色图、极坐标、多轴/多绘图区和标注图元；支持平移、缩放、选择、实时数据及 PNG/JPEG/BMP/SVG/PDF 导出。两端绑定一次 `ForgePlotServices` 即可使用默认坐标和样式。独立示例位于 `examples/plot-rust` 和 `examples/plot-cpp`，Gallery 的“专业图表”页面复用同一演示组件。实际平台和性能范围见 [专业图表验证](docs/PLOT_VERIFICATION.md)。

## 测试与检查范围

每次任务默认只测试和检查本次修改影响的内容及其直接依赖，不运行全量检查。Rust 按包、测试文件和用例名过滤，C++ 按构建目标和 CTest 名称过滤，UI 只检查受影响页面；仅修改文档或规则时不构建、不跑功能测试。具体执行规则和定向命令见 [AGENTS.md](AGENTS.md)，Claude 通过 [CLAUDE.md](CLAUDE.md) 使用同一份规则。

下文及历史报告中的工作区测试、完整 CTest、全 Gallery 截图和完整验收命令均为按需参考，不是日常任务的必做步骤。共享代码变更也应先分析实际影响，再扩大到相关调用方；只有明确要求全量或整个项目确实受影响时才执行全量验证。

## Windows 构建与运行

工作区 Rust 固定使用 **1.98.0** 的 `x86_64-pc-windows-msvc` 工具链，并且需要 C++ 编译器构建公共绘图核心。C++ 使用 Visual Studio 2026 x64、CMake 3.21+，以及官方 Slint 1.18.0 Windows MSVC SDK。默认构建环境由 `D:\.clion-env\qt-6.8.3-msvc.bat` 加载。独立图表示例已进入根 Cargo 工作区并使用同一 Cargo.lock。

现有 Windows 打包脚本生成一个未压缩目录和一个安装包，提供 API、Rust/C++ Gallery 与可直接接入的组件及二进制 SDK。该脚本目前不包含完整对应源码，生成的二进制包仅供内部验证；对外分发前须按 GPLv3 提供 SlintForge 及所附 Slint SDK 的对应源码，详见 [Windows 交付](docs/RELEASE.md)。服务绑定、模型刷新、线程及性能约定见 [接入指南](docs/INTEGRATION.md)。

```powershell
# 综合验收入口，局部修改不要默认运行；Quick 仍包含工作区测试。
pwsh -File tools/windows/Verify.ps1 -Mode Quick
pwsh -File tools/windows/Verify.ps1 -Mode Full -EvidenceDirectory build/acceptance
pwsh -File tools/windows/Package.ps1 -VerificationReport build/acceptance/result.json
```

所有依赖路径均可通过脚本参数覆盖；已有缓存时默认 `--offline --locked`，首次下载可加 `-Online`。脚本保存每项命令、退出码、日志和结果 JSON，并以源码指纹关联完整验收与发布。

```powershell
# 全量测试仅按上方范围规则选用；日常使用 AGENTS.md 中的定向命令。
cargo test --workspace
cargo run -p slintforge-gallery-rust
cargo run -p slintforge-example-rust

cmake -S . -B build/cpp -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_PREFIX_PATH="D:/path/to/slint-cpp-1.18.0" -DSLINT_STYLE=fluent
cmake --build build/cpp --config Debug
ctest --test-dir build/cpp -C Debug --output-on-failure

# 运行前将 SDK 的 lib 目录加入 PATH，以加载 slint_cpp.dll。
$env:PATH = "D:\path\to\slint-cpp-1.18.0\lib;$env:PATH"
& build/cpp/Gallery/cpp/Debug/slintforge_gallery_cpp.exe
& build/cpp/examples/cpp/Debug/slintforge_example_cpp.exe
```

`--smoke` 在不显示窗口的条件下检查 Gallery 的设置页与字体页路由、完整图标目录搜索与分页、中英文切换、文字字体、字号比例、置顶、边角状态、参数、返回、导航展开、导航图标、标题栏图标切换、三种位置和面板状态、13 个布局／容器页面路由、页面堆叠输入状态、瀑布流重排及后台采样。`--run-smoke` 打开真实窗口并运行事件循环约 850 ms 后自动退出，检查采样值；两种参数均适用于 Rust/C++ Gallery。
手工检查顶部下拉导航和右侧导航时，可分别使用 `--preview-top`、`--preview-right` 启动 Gallery；使用 `--preview-route=forge-window` 等稳定 ID 可直接打开指定页面。

## 安装 C++ 包供其他工程使用

```powershell
cmake --install build/cpp --config Debug --prefix build/install
cmake -S examples/cpp -B build/external-cpp -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_PREFIX_PATH="D:/path/to/slint-cpp-1.18.0;D:/path/to/SlintForge/build/install"
cmake --build build/external-cpp --config Debug
```

外部工程通过 `find_package(SlintForge 0.1.2 CONFIG REQUIRED)` 得到 `SlintForge::Sdk`、`slintforge_target_sources()` 和 Slint 依赖。`slintforge_target_sources()` 为目标注册 `@slintforge` 路径。发布包同时安装 Debug/Release 库；开发环境仍需独立提供官方 Slint 1.18.0 SDK。

图标字体使用 Microsoft Fluent System Icons Regular，资源和 MIT 许可文件位于 `ui/assets/fonts`；C++ 安装包随同安装字体、码位表和许可文件。SlintForge 原创代码按 GPL-3.0-only 授权；Slint 自身保留官方许可声明，详见 [许可说明](LICENSE.md)。
二维码采用 Project Nayuki v1.8.0 的 MIT 源码，保持上游源文件原样；来源与许可位于 `third_party/qrcodegen`，C++ 安装包也安装对应许可证。

## 容器与截图验证

本节列出较大范围的验证入口；局部修改只选受影响容器和截图路径，不整段执行下面的命令。

容器分类现有 8 个独立组件：`ForgePage`、`ForgeRoundImageView`、`ForgeIconLabel`、`ForgeExpander`、`ForgeWatermark`、`ForgeMarquee`、`ForgeCard`、`ForgeWindow`。原生圆角和裁剪直接使用 `Rectangle`。截图 API 和替代写法见 [中文 API](docs/API.md)，两端接入说明也提供 [English capture guide](docs/CAPTURE.en.md)。

```powershell
cargo test --workspace --locked
cargo run --locked -p slintforge-gallery-rust -- --capture-smoke
cargo run --locked -p slintforge-example-rust -- --capture-smoke
# C++ 构建和 CTest 后，也可单独运行 Gallery/消费者的 --capture-smoke。
```

`--capture-smoke` 打开真实窗口，检查整窗及嵌套页面截图、调整尺寸和最大化后的物理尺寸、错误路径，以及 PNG 写入／解码。Gallery 测试目录和截图路径打印到终端。C++ CTest 包含 SDK、像素裁剪／PNG、Gallery smoke 和真实窗口截图测试。坐标单元测试覆盖 100%／125%／150%／200% 缩放；容器交互测试覆盖折叠与键盘、状态保留、水印点击、动画暂停继续及文本换行。实际截图仍须检查显示效果。

## 新组件与逐页验证

新增或修改组件只验证该组件及其直接受影响页面。以下逐页入口保留用于明确需要的广泛验证，不作为新增组件后的默认检查清单。

`--components-smoke` 在实际窗口检查全部 全部路由的深色中文、浅色英文/125% 字号和窄窗口中文状态，保留 20 个弹层、忙碌、图表与表格操作状态，并覆盖 55 个页面的新增标签示例与典型响应式布局。每个宿主按路由和场景目录输出 PNG 到 `build/component-qa/rust` 或 `build/component-qa/cpp`，完成后自动退出且不改用户设置。Gallery 中可直接通过并排卡片、滚动区域和标签查看这些效果，见 [示例组织说明](docs/GALLERY_PRESENTATION.md)。`--widgets-smoke` 用于两个最小消费者，验证二维码、高亮和材质服务；`--charts-smoke` 验证 7 类基础图表；`--grids-smoke` 验证公共表格组件的七种状态。专业图表的独立示例使用 `--smoke` 和 `--benchmark`，见 [ForgePlot API](docs/PLOT.md)。

```powershell
cargo test --workspace --offline --locked
cargo run -p slintforge-gallery-rust --offline --locked -- --components-smoke
cargo run -p slintforge-example-rust --offline --locked -- --widgets-smoke
cargo run -p slintforge-example-rust --offline --locked -- --grids-smoke
ctest --test-dir build/cpp -C Debug --output-on-failure
```

`tools/verify_components.py --compiler <slint-compiler路径>` 会独立编译 目录中的全部新增页面的用法示例，并校验状态目录、译文和第一方 C++ 命名规则。`--qr` 另外使用测试环境中的 zxing-cpp 解码并比较两端 QR 像素；解码器仅为 QA 依赖，不属于运行时 SDK。

最新测试、逐页截图、安装后消费者结果和复现命令见 [表格接手审查](docs/GRID_REVIEW.md) 和 [专业图表验证](docs/PLOT_VERIFICATION.md)；开发阶段的表格证据保留在 [原验证记录](docs/GRIDS_VERIFICATION.md)。

上述专题报告属于各组件实施阶段；v0.1.2 的当前验收结果以时间戳发布目录中的 `ACCEPTANCE.md`、`verification` 和 `manifest.json` 为准。

2026-10-04 的公共组件边界、导航树性能和 ForgePlot 增量更新审查见 [组件代码审查与修复](docs/COMPONENT_CODE_REVIEW_20261004.md)。

## Rust DLL SDK

Rust 计算核心以 DLL 交付，Slint 对象和服务绑定保留原生 Rust 接口。支持和验收基线为 Rust 1.98.0，客户使用该版本编译公共封装与 Slint 1.18.0；核心 DLL 通过版本化 C ABI 接入。首次构建需要依赖下载与编译，应用发布时携带 DLL，工具可自动复制。交付体积与客户缓存分别统计。详见 [Rust DLL 接入](docs/RUST_DLL.md)。

## 许可

Copyright © 2026 陕西禾又科技有限责任公司。SlintForge 自有代码按 [GPL-3.0-only](LICENSE) 授权，与 Slint 的开源许可选项兼容；第三方内容保留各自许可。与 Slint 三重授权的关系、闭源使用条件和第三方清单见 [许可说明](LICENSE.md)，逐路径声明见 [REUSE.toml](REUSE.toml)。
