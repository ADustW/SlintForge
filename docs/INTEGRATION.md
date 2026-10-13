# v0.1.2 SDK 接入约定

Rust 与 C++ 编译同一套 `.slint` 契约。应用为每个顶层窗口绑定其使用的服务；绑定持有窗口弱引用，关闭窗口后不得继续直接写窗口状态。

| 使用能力 | Rust / C++ 绑定 | 刷新约定 |
|---|---|---|
| 二维码、高亮、材质、建议输入键盘导航、颜色文本与 Windows 取色 | `bind_widget_services!` / `forge_bind_widget_services`；`ForgeWidgetServices` | 修改输入属性后同步重算；替换建议模型或改筛选文字会清除候选索引 |
| 基础图表 | `bind_chart_services!` / `forge_bind_chart_services`；`ForgeChartServices` | 更换顶层属性自动重算；嵌套模型修改后调用对应组件 `refresh()` |
| DataGrid、TreeDataGrid | `bind_grid_services!` / `forge_bind_grid_services`；`ForgeGridServices` | 更换顶层模型自动重算；原模型行或嵌套 cells 原位修改后调用 `refresh()` |
| ForgePlot | `bind_plot_services!` / `forge_bind_plot_services`；`ForgePlotServices` | 场景/模型刷新与实时批次追加见 [Plot API](PLOT.md) |
| ForgeFlow | `bind_flow_services!` / `forge_bind_flow_services`；`ForgeFlowServices` | 每个画布独立文档；编辑请求由核心校验并按稳定 ID 更新模型行，见 [Flow API](FLOW.md) |
| 窗口/页面截图 | `bind_capture_service!` / `forge_bind_capture_service` | 窗口必须已显示且布局完成；捕获当前可见区域 |

完整绑定参数见 [组件 API](COMPONENTS.md)、[基础图表](CHARTS.md)、[表格](GRIDS.md)、[专业图表](PLOT.md) 和 [截图](API.md)。服务未绑定时对应控件不会产生有效计算结果，应在显示窗口前完成绑定。无需服务的纯 Slint 控件可直接导入使用。

所有 Slint 属性、模型通知、绑定、回调和刷新在 UI 线程执行。后台线程完成 IO/计算后，通过 Rust `slint::invoke_from_event_loop` 或 C++ `slint::invoke_from_event_loop` 回传数据，在回调中升级弱引用再操作 UI。不要从采样线程直接调用组件函数。同步服务适合中小规模模型；建议候选查找为 O(n)，大模型应在后台预筛选并分页。网格、材质及图表的适用范围和实测性能见对应 API/验收记录，已有 Windows 软件渲染结果不能外推到任意数据规模或其他平台。

## 建议输入

`ForgeAutoSuggestBox.current-suggestion` 为源模型索引，未选择时为 -1；显示匹配候选后 Up/Down 跳过不匹配行，首末边界保持当前行，输入或模型替换时重置。Enter 选中当前候选；尚未导航时选第一个匹配项，无匹配时仍提交输入文本。Escape 关闭列表，Tab 关闭并继续默认焦点遍历。禁用时收起并拒绝导航。候选按钮保留鼠标选择。

键盘匹配查找使用 `ForgeWidgetServices.suggestion-index`，需绑定 widget 服务；`keyboard-navigation-ready` 可检查连接状态。选中项与源模型保持同一索引，不能按过滤列表位置回写。列表绘制沿用 Slint 的大小写转换；跨语言查找也采用 Unicode 小写前缀匹配。

## 业务对话框

`ForgeContentDialog` 的 `@children` 常驻，关闭仅隐藏，不清空输入。默认确认框仍在两个操作按钮间遍历。添加业务输入后，由调用方处理以下兼容增补回调，返回 true 表示已处理：

- `initial-focus-requested() -> bool`：打开时为首个业务控件聚焦。
- `focus-next-requested(reverse: bool) -> bool`：处理 Tab/Shift+Tab，依据真实焦点查找下一个可用控件并跳过禁用项。
- `focus-return-requested()`：关闭后恢复打开控件的焦点。

`focus-primary-button()`、`focus-cancel-button()` 允许把操作按钮加入业务焦点环；`primary-has-focus`、`cancel-has-focus` 供调用方读取真实状态。返回 false 时回退到默认行为。焦点委托应同步执行，不能把焦点操作推迟到后台线程；在委托中不要再次切换 `open`。`dismiss-on-outside` 只控制遮罩点击，Escape 保留取消语义。

完整独立窗口见 `examples/ui/business-interactions.slint`。两个宿主提供 `--suggestion-demo`、`--dialog-demo` 及同名 `--*-smoke`，验证原生键盘事件、跳过禁用项、状态保留和关闭后的焦点恢复。

## 构建依赖

Rust SDK 使用 `slintforge_runtime.dll` 和公共 Rust 安全封装。支持和验收基线为 Rust 1.98.0，客户使用该版本与 MSVC 链接环境，通过 Cargo 获取并编译 Slint 1.18.0；核心实现只以 DLL 交付。现有 `sdk/rust/Compile-Rust.ps1` 参数保持兼容，新增 `-Offline` 和 Debug/Release 配置，自动部署 DLL。标准 Cargo 接入由 `slintforge-build` 提供 UI 编译和 `deploy_runtime(output_dir)`。首次联网下载与编译时间、缓存占用属于客户构建环境，不计入 SDK 主包体积。C++ SDK 继续提供 Debug/Release 静态库。详见 [DLL 接入](RUST_DLL.md) 和 [交付说明](RELEASE.md)。

C++ 的 `slintforge_target_sources()` 默认设置目标属性 `SLINT_EMBED_RESOURCES=embed-files`，将图片、字体等资源嵌入程序；调用方显式设置的资源策略仍会保留。直接使用官方 `slint_target_sources()` 的应用应自行设置该属性，避免运行时引用开发机绝对路径。Gallery 也嵌入全部资源，便携运行无需访问原源码目录。

ForgeFlow 的画布需要整体缩放，应用启动前选择 `winit-skia` 或 `winit-skia-software`；不能在进入页面后切换渲染器。Rust 需开启 `renderer-skia` 及 SDK `flow` 特性，二进制接入入口使用 `Compile-Rust.ps1 -Flow`。Windows 示例默认选择 DX12，已有 `WGPU_BACKEND` 设置优先。独立接入验证入口为 `tools/windows/Test-FlowConsumers.ps1`，从安装后的 C++ SDK、Rust 接口包和运行 DLL 构建相同窗口。
