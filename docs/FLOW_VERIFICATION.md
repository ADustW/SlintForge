# ForgeFlow 验证记录

本文件保留首版验收证据。后续的连线路由及样式修改见 [连线增量验收](FLOW_CONNECTIONS_VERIFICATION.md)，以各自源指纹区分验证范围。

验证日期：2026-10-13。组件、共用核心和双语言接入已经实现。本记录区分实际窗口操作、SDK 接入、性能采样和仍待补齐的验收项；编译成功不代表全部验收完成。

## 环境

- Windows，机器 `LAPTOP-SPSBRPK9`；AMD Ryzen 9 9955HX，16 核／32 线程，约 64 GB 内存。
- NVIDIA GeForce RTX 5060 Laptop GPU，驱动 `32.0.15.7709`。
- Slint 1.18.0、Rust 1.98.0；CLion 的 `D:/.clion-env/qt-6.8.3-msvc.bat` 环境，Visual Studio 18 2026 生成器。
- 图模型、历史、几何、树布局和 JSON 使用同一份 `sdk/common/ForgeFlowCore.cpp`；JSON 库固定为 nlohmann/json 3.12.0。

## 核心与交互

两端 Release 示例各执行以下六种真实窗口配置，每种执行同一份 `examples/ui/flow-qa.json` 的 114 步，共 12 组，全部通过。最终矩阵证据位于 `build/flow-final/result.json`：

| 渲染路径 | 应用缩放比例 |
| --- | --- |
| `winit-skia`，Windows DX12 | 100%、150%、200% |
| `winit-skia-software` | 100%、150%、200% |

缩放比例由 `SLINT_SCALE_FACTOR` 设置，鼠标和键盘事件通过 Slint 窗口事件接口进入真实窗口的命中、焦点和渲染流程。此记录不是人工跨显示器拖窗或修改 Windows 全局 DPI 的记录。

覆盖卡片输入与提交、未提交文字在拖动后保留、按钮和下拉弹层、缩放后点击、单选／追加选择／框选／批量移动、复制删除、拖动一次撤销、端口连接、端点重连、连线命中删除、业务连接拒绝、中键及空格平移、鼠标锚点缩放、只读、深浅主题、640×500 窄窗口、折叠和文件保存加载。还覆盖从 Slint 提交完整节点、未知类型的数据保留、更新类型和尺寸，以及逐步撤销。截图保存于各配置的证据子目录。

C++ 核心检查覆盖非法引用、方向、自连接、重复连接、非法线型、损坏 JSON、未知版本、原子加载、多文档隔离、业务数据、连续拖动事务、树防环、分支折叠恢复、确定性布局、尺寸变化后的多端口坐标，以及树连线标签／线型／箭头的保存和重布局保留。500 节点、1000 连线的单节点拖动只提交一个节点及最多四条相关连线的增量行；本轮核心调用 P95 为 **0.357 ms**。

Rust 另验证 Unicode／未知卡片业务数据的 DLL 往返、独立文档和 100 步历史上限。回归发现损坏 JSON 触发进程终止，定位为 MSVC 未启用异常展开；增加 `/EHsc` 后，原失败用例与另一项 Flow DLL 测试均通过。失败和修复日志保留在 `build/flow-qa/runtime-exceptions-fix-*`，不将失败尝试记为成功。

## 性能

Release，1100×760 物理像素，100% 缩放。场景均包含 500 节点和 1000 连线；混合场景循环使用表单、设备按钮和图片卡片。先预热，再依次发出约 100 次节点拖动、100 次视口平移和 100 次鼠标锚点缩放。节点使用可见性裁剪，测试起始视口缩放为 100%，并非把全部节点同时缩小到屏幕内。

| 宿主／实际渲染路径 | 场景 | 帧样本 | 帧间隔 P95 | 输入反馈 P95 | 结果 |
| --- | --- | ---: | ---: | ---: | --- |
| C++ / `winit-skia-opengl` | 标准卡片 | 658 | 14.573 ms | 14.840 ms | 达标 |
| C++ / `winit-skia-opengl` | 混合卡片 | 621 | 15.158 ms | 15.628 ms | 达标 |
| Rust / `winit-skia`，DX12 | 标准卡片 | 563 | 17.048 ms | 16.560 ms | 达标 |
| Rust / `winit-skia`，DX12 | 混合卡片 | 548 | 17.496 ms | 16.830 ms | 达标 |

目标为帧间隔 P95 ≤33 ms、反馈 P95 ≤100 ms。帧间隔取相邻 `AfterRendering` 时间，反馈取窗口输入派发到下一次 `AfterRendering`，不是显示器实际呈现到人眼的延迟。样本不足时报告 `measurement_complete=false`、耗时为 `null`，不会将空样本当作达标。原始记录与截图：`build/flow-final/cpp-performance/`、`rust-performance/`。

**尚未闭环：C++ 默认 DX12 的帧耗时。** 官方 C++ Slint DLL 的该路径未产生帧回调，因此上述 C++ 性能数字来自明确指定的 Skia OpenGL 路径。默认 DX12 已通过实际窗口交互矩阵，但不能据此宣称帧耗时达标。外部 PresentMon 采集尝试因当前账户缺少 ETW 会话权限而失败；没有修改系统权限、渲染器默认值或第三方 DLL。后续需在具备采集权限的环境补测此项。两端示例默认仍是 `winit-skia`。

## 外部 SDK

`build/flow-consumers-accepted/result.json` 已通过。独立接入目录为 `D:/ai_workspace/SlintForge/Flow SDK 接入 20261013-124508`，包含中文与空格。C++ 使用安装后的 `SlintForge::Sdk`、公共头文件及 `.slint`；Rust 使用复制出的接口包和运行库，不复制共用核心源码。两端复用同一示例窗口，分别通过 114 步交互。

Rust 独立运行目录只放应用和 `slintforge_runtime.dll`，运行时将 PATH 限制为 Windows 系统目录。构建复用了本机 Cargo 依赖缓存，此项是仓库外接入及独立部署验证，不能当作无缓存或全新机器验收。

本轮运行 DLL SHA-256：`1183B461BDB1F2007A9D569D20DE4D889DF61C9C717742CD7E4AC03FD2D3DAF8`。

## Gallery 与回归

两端五个 Flow 页面已通过 15 个主题／语言／窗口状态，包含中英文导航搜索、各页面独立文档及离开返回后的标题保留。Gallery 根导航在“图表”之后新增“节点与连线”。首次打开思维导图会适应全图，后续返回保留视口。

完整回归 `build/flow-regression-accepted/result.json` 通过 24 项检查，包括 Rust 76 项测试、C++ 20 项测试、68 个组件示例编译，以及每端 101 个页面／498 个窗口截图状态。两端均测试窗口最小化后的恢复。导航、本地化、生成绑定、ABI 指纹、第一方 C++ 命名规范、图表几何、图例和表格契约检查均通过。

旧组件使用 `winit-software` 单独回归，排除五个 Flow 页面，两端各 483 个截图状态均通过，证据为 `build/flow-legacy-verified/result.json`。不以 Skia 检查替代此软件渲染回归。

最终应用源码指纹：`86beb95f4e33c99d7f613043a86cd9f8bd55ced1b14cc2410f159d438cabeaab`；完整回归从开始到结束的验证输入指纹保持一致。

## 重现入口

```powershell
tools/windows/Test-Flow.ps1
tools/windows/Test-FlowConsumers.ps1
tools/windows/Verify.ps1 -Mode Full
tools/windows/Test-FlowLegacy.ps1
```

验证脚本按目录统计页面、示例和截图，不写死新页面总数。`Test-FlowLegacy.ps1` 使用已构建的 Debug Gallery，在 `winit-software` 下排除需要整体缩放的 Flow 页面，保留旧组件回归。每份证据记录工具参数、退出码或窗口完成标记、源指纹及输出路径。没有执行最终发布、独立机器安装或设备现场验收。

指针／键盘窗口验收应串行执行，避免其他验收窗口切换焦点。一次并发实验在 C++ 150% Skia 软件渲染下发生拖动位置断言失败，记录保留在 `build/flow-accepted/`；最终关闭其他验收窗口，完整重跑 12 组 114 步场景与性能采样，全部通过。最终结论采用 `build/flow-final/`，不采用失败的中间批次。
