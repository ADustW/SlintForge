# ForgePlot 区域内外布局验证

> 阶段记录：本文保留该组件实施时的范围、测试数量和验证结果；v0.1.1 当前能力见接入与交付说明，本次验收以时间戳发布目录中的 ACCEPTANCE.md 和 verification 为准。

日期：2026-10-03。根据 Gallery 反馈移除默认曲线示例的红色图片图元，并增加图例和测量信息框的独立区域、位置设置。

## 最终行为

- Gallery 提供图例区域内/外、测量区域内/外及各自的八种位置。组件对应属性为 `legend-placement`、`legend-position`、`measurement-placement`、`measurement-position`。
- 图例默认在波形区域内，所有位置和样式均保持透明背景。测量框默认在下方区域外，明暗主题均使用主题文字和图表背景色。
- 区域内距波形边界 6px；区域外距刻度、轴标题的占用边界 6px，预留独立空间。同侧外部信息框分开排列，间距 6px；区域内发生重叠且空间足够时向内错开。
- 多绘图区中，测量信息框跟随所选曲线所在绘图区，只显示一次。测量文字分三行显示；位置和区域切换保留追加历史。
- 移除的是默认示例的红色图片；图片图元 API 和矢量、位图、PDF 导出功能继续由核心回归测试覆盖。

## 验证结果

Windows x64、Slint 1.18.0、`winit-software`；C++ 构建加载 `D:\.clion-env\qt-6.8.3-msvc.bat`。

| 检查 | 结果 |
|---|---|
| 核心位置组合 | 8 图例位置 × 8 测量位置 × 4 区域组合 × 3 图例样式，共 768 组通过；检查组件边界、区域内外、框间重叠及图例点击 |
| 公共组件/独立示例 | Rust 公共组件 3 项、独立示例 2 项通过；覆盖游标拖动、选择、缩放、禁止交互、配置错误恢复与布局切换保留历史 |
| 实际窗口 | 每端 68 张明暗主题截图，两端共 136 张；包含八位置、四区域组合和所选第二绘图区 |
| 双端一致性 | 68 组 SVG 完全相同，布局坐标一致；测量框内部文字有足够像素对比 |
| Gallery | 两端各 4 张中英文、明暗及窄窗口截图，菜单入口和新增控件已核对 |
| 导出 | 两端各 10 种场景 SVG/PDF 与窗口捕获，另有 PNG/JPEG/BMP 导出；双端矢量、导出位图一致 |
| 契约/安装 | FFI 和绑定生成检查、96 页目录、第一方 C++ namespace 检查及 66 个文档片段通过；全新安装 SDK 的头文件和 Slint 契约与源码一致 |

百万点有序历史、每帧追加 32 点、共 64 帧。图例和测量框同时启用并放在下方区域外；完整帧包含追加、场景准备及 1080 × 760 窗口绘制/截图。

| 宿主 | 完整帧中位数 | 完整帧 p95 | 追加 p95 |
|---|---:|---:|---:|
| Rust Debug，绘制依赖启用优化 | 21.433 ms | 22.958 ms | 0.057 ms |
| C++ Release | 13.826 ms | 15.036 ms | 0.024 ms |

两端完整帧 p95 小于示例 33ms 更新周期及 50ms 回归门限。以上是本机软件渲染测试结果，具体窗口尺寸、数据类型和设备会影响耗时。

## 证据与复验

本轮证据目录为 [`build/plot-placement-qa`](../build/plot-placement-qa)，不覆盖之前的主题、性能和实时绘图记录。`rust-placement/cpp-placement` 包含截图、SVG、布局 CSV 和性能 CSV；`verification.json` 记录结果与源码、二进制 SHA-256。

运行两端独立示例 `--placement-smoke`，通过 `SLINTFORGE_PLOT_QA_DIR` 分别输出到上述目录；运行 `python tools/verify_plot_placement.py build/plot-placement-qa` 检查位置、测量文字与性能。`--smoke` 的本轮输出位于 `rust-consumer/cpp-consumer`。

常用程序已重新构建：`target/debug/slintforge-gallery-rust.exe`、`build/cpp/Gallery/cpp/Debug/slintforge_gallery_cpp.exe`。已运行的旧 Gallery 需退出后重新启动。
