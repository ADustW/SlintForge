# ForgePlot 菜单与测量可用性验证

验证日期：2026-10-03。环境：Windows x64、Slint 1.18.0、软件渲染器；Rust Debug（依赖启用优化），C++ Release 消费者及 Debug Gallery。C++ 构建加载 `D:\.clion-env\qt-6.8.3-msvc.bat`。

## 修复及原因

1. 默认 Fluent 下拉框仅展开六行，左上、左下在最后两行。Gallery 的两个位置选择器改为一次展开全部八个方位，支持鼠标、方向键、Home/End、回车和 Esc。以前直接设置位置属性的验证未覆盖实际展开菜单，本轮补上真实窗口点击。
2. A/B 固定在 1 秒和 3 秒，实时窗口滚动到 120 秒后已离开视图。首次启用时把不可见光标复位到当前范围的 25%/65%；实时窗口默认保持相对位置。加入橙色 A、紫色 B 竖线及顶部标记、光标复位按钮。光标坐标转换使用所选曲线的 X 轴，支持独立绘图区。
3. 测量框默认尺寸改为按字体、系列名及数值自动计算，内容排列为四行：名称、A 的 X/Y、B 的 X/Y、ΔX/ΔY。本例 340px 宽收紧至 142px，高 76px。区域内保持透明背景，区域外使用主题背景；可继续显式指定尺寸。
4. Gallery 分为四行：图表/窗口/刻度；绘制/实时设置；全部图例设置；全部测量设置。图例行包含区域、位置、样式、最新值；测量行包含开关、区域、位置、曲线、复位。中文深色、英文浅色（字体 1.25 倍）及 960px 窄窗口均保留同一行分组；较窄的英文选择值会省略，展开后可读完整选项。

## 功能与视觉验证

- Gallery 图表自动测试 4/4；独立 Rust 消费者测试 3/3。覆盖运行后启用测量、视图继续推进、关闭光标跟随后的坐标锚定、测量拖动、实际点击最后两项、独立面板 X 轴与增量历史保留。
- Rust/C++ 实际窗口各点击四次：图例左上/左下、测量左上/左下；各保存四张展开菜单和四张选择后的截图。
- 两端分别累计追加 100Hz 模拟数据到 130/140/170 秒，共 26,002/28,002/34,002 点；保留 10 秒可见窗口，A/B 均在可见范围内，保持 25%/65% 相对位置。170 秒时 A=162.5、B=166.5。这是加速输入的模拟时间，未等待 170 秒墙钟时间。
- 两端 Gallery 各四张真实页面截图：深色中文、浅色英文、窄窗口及极坐标场景；启用测量和菜单操作另由共用示例的实际消费者窗口验证。
- 共用数值引擎通过 768 组布局组合；两端实际窗口各验证 68 个深浅色、八方向、区域内外和多面板组合，原生 SVG 与布局元数据一致。
- 每端另有四组左上/左下测量框开关对照，比较框内有颜色的原波形像素，排除文字及光标覆盖。保留原波形比例超过 99%；图例所有位置仍透明，边距保持 6px。

## 性能

百万点有序历史，每帧追加 32 点，64 帧。图例和测量均显示在区域外。完整帧包含追加、准备以及 1080×760 实际窗口绘制/截图；本机结果不等同于任意机器的帧率保证。

| 宿主 | 完整帧中位数 | 完整帧 p95 | 追加 p95 |
|---|---:|---:|---:|
| Rust | 21.662 ms | 22.617 ms | 0.064 ms |
| C++ | 15.522 ms | 17.109 ms | 0.026 ms |

## 证据与复验

证据存放于 [`build/plot-usability-qa`](../build/plot-usability-qa)，此前 QA 目录保持不变。`rust-usability/cpp-usability` 保存展开菜单、位置选择、170 秒光标截图和坐标 CSV；`rust-gallery/cpp-gallery` 保存分行设置；`rust-placement/cpp-placement` 保存布局、透明对比及性能；`verification.json` 保存验证结果和源码、程序 SHA-256。

透明度验证器还通过负样本检查：复制本轮截图并给测量框填充不透明背景，验证器正确拒绝该样本，未修改原始证据。示例截图：[展开测量位置菜单](../build/plot-usability-qa/cpp-usability/measure-menu-left-bottom.png)、[170 秒时的双光标](../build/plot-usability-qa/rust-usability/live-170.png)、[Gallery 分行设置](../build/plot-usability-qa/rust-gallery/000-chart-professional-dark-zh.png)。

两端独立消费者运行 `--usability-smoke`、`--placement-smoke`，通过 `SLINTFORGE_PLOT_QA_DIR` 指向对应证据目录。Gallery 使用 `--components-smoke` 并设置 `SLINTFORGE_COMPONENT_QA_FILTER=chart-professional`。运行 `tools/verify_plot_placement.py build/plot-usability-qa --require-overlay` 验证透明度、布局、光标、性能及两端一致性。

SDK 安装头文件与共享 Slint 契约已核对。常用 Rust/C++ Gallery 均已重新构建，退出旧进程后重开生效。
