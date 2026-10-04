# ForgePlot 主题与图例布局修复

日期：2026-10-03。修复深色主题下测量框白底白字、图例底色遮挡波形，以及侧边图例距离波形区域过远的问题。

## 最终行为

- 测量框使用图表自身的背景色和主题文字色，边框使用主题前景色的透明版本；明暗主题均可读。测量框选择另一侧，避免与底部、左侧及左侧角落的图例重叠。
- 八种图例位置统一放在第一个绘图区内侧，距对应边界固定 6 个逻辑像素。顶部、底部水平居中；左侧、右侧垂直居中；四角分别贴边。
- 所有位置和样式的图例背景均透明。`framed` 仅保留边框；`plain/compact` 无底色及边框。
- 图例不再为侧边或上下位置额外预留空间；开关图例、切换位置或样式不会改变波形区域大小。
- Windows 图例宽度按实际字体度量，移除估算宽度的额外余量；长名称仍可截断并保留最新值。可见内容与点击区域使用同一布局。

## 验证结果

Windows x64、Slint 1.18.0、`winit-software`。Rust/C++ 独立消费者均与 Gallery 使用同一份演示组件。

| 检查 | 结果 |
|---|---|
| 明暗主题 × 8 位置 × 3 样式 | 每端 48 组，共 96 组；SVG 相同，均无图例底色 |
| 图例边界间距和波形区域 | 透明边框距对应边界均为 6 像素；各位置、样式的绘图区尺寸相同 |
| 测量框主题及位置 | 每端明暗测量及底部图例截图；深色框背景 RGB 均值中位数 27、浅色 249，均有清晰文字对比 |
| 实际窗口截图 | 每端 54 张，共 108 张；另有两端 Gallery 各 4 张中英文、主题和窄窗口截图 |
| 功能回归 | 原生核心测试、Rust 公共组件 3 项、独立示例 2 项全部通过；图例点击、光标拖动、历史追加及缩放正常 |
| 生成及安装契约 | FFI/绑定检查、第一方 C++ namespace 检查、66 个文档片段编译通过；安装头文件及 Slint 契约与源码相同 |

性能复验使用百万点有序历史，每帧追加 32 点，共 64 帧；完整帧含追加、场景准备及 1080 × 760 窗口绘制/截图。

| 宿主 | 完整帧中位数 | p95 |
|---|---:|---:|
| Rust Debug，绘制依赖启用优化 | 24.112 ms | 25.018 ms |
| C++ Release | 15.458 ms | 16.543 ms |

两端完整帧 p95 均小于示例的 33 ms 更新周期，继续满足 50 ms 性能检查门限；追加 p95 小于 5 ms。实际实时采样再次验证首点保留、跨越旧 2000 点缓存边界、暂停恢复及历史滚动。

## 复验与产物

证据目录：[`build/plot-appearance-qa`](../build/plot-appearance-qa)。[`verification.json`](../build/plot-appearance-qa/verification.json) 包含布局、配色、性能统计及本轮源码和程序 SHA-256。

两端消费者的 `--appearance-smoke` 捕获图例和测量框，`--streaming-smoke` 复验性能与历史连续性；输出目录通过 `SLINTFORGE_PLOT_QA_DIR` 指定。目录分别使用 `rust-window/cpp-window`、`rust-performance/cpp-performance`。运行 `python tools/verify_plot_appearance.py` 检查全部记录。

常用 Gallery 已重新构建：

- Rust：`target/debug/slintforge-gallery-rust.exe`。
- C++：`build/cpp/Gallery/cpp/Debug/slintforge_gallery_cpp.exe`。

C++ 构建加载 `D:\.clion-env\qt-6.8.3-msvc.bat`。旧运行进程需退出并重新打开，才能使用本轮更新。
