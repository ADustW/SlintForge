# ForgePlot 透明叠加与左侧角落验证

日期：2026-10-03。用户确认“光标位置”指双光标测量信息框。

## 最终行为

- 图例和双光标测量信息框的位置菜单均包含左上、左下，对应组件的 `top-left/bottom-left`。已有的位置契约保持不变，本轮核对了菜单索引映射及实际窗口布局。
- 图例在区域内、区域外均保持透明背景；边框样式只绘制边框。
- 测量信息框在波形区域内使用完全透明背景，框下方的波形和填充继续显示；区域外继续使用图表主题背景。文字及细边框保持主题颜色。
- 区域内左上、左下距离波形边界为 6px；两个信息框选择同位置时继续按已有布局规则错开。

## 验证结果

平台：Windows x64、Slint 1.18.0、`winit-software`。Rust/C++ 均使用共用 `.slint` 组件；C++ 构建加载 `D:\.clion-env\qt-6.8.3-msvc.bat`。

| 检查 | 结果 |
|---|---|
| 左上/左下及透明底色 | 每端 4 组：明暗主题 × 两种左側角落交叉布局；分别捕获测量开启/关闭的相同波形 |
| 波形像素对比 | 测量框右半部分排除文字区域后，左下有 5934 个彩色像素、左上有 53 个；99.410%～100% 像素完全保留，其余差异来自 A/B 光标线 |
| 八位置、区域组合与多绘图区 | 每端 68 组通过；双端 SVG 完全相同，布局坐标一致 |
| 实际窗口截图 | 每端 76 张，共 152 张；另有两端 Gallery 各 4 张中英文、主题和窄窗口截图 |
| 公共组件交互 | Rust 3 项通过：光标拖动、选择、缩放、禁用交互及临时配置错误恢复 |
| 契约及安装 | 96 页目录、66 个文档片段编译、第一方 C++ namespace 检查及翻译同步通过；本轮安装的 4 个头文件和 5 份 Slint 契约与源码相同 |

已有八位置和区域外布局继续复验百万点有序历史，每帧追加 32 点、共 64 帧；完整帧包含追加、场景准备与 1080 × 760 窗口绘制/截图。性能样本的图例和测量框均放在区域外。

| 宿主 | 完整帧中位数 | 完整帧 p95 | 追加 p95 |
|---|---:|---:|---:|
| Rust Debug，绘制依赖启用优化 | 20.160 ms | 21.027 ms | 0.054 ms |
| C++ Release | 14.305 ms | 15.048 ms | 0.025 ms |

## 证据与程序

本轮证据位于 [`build/plot-overlay-qa`](../build/plot-overlay-qa)，保留之前的布局和主题记录。`rust-placement/cpp-placement` 中的 `overlay.csv`、`*-corner-*-on/off.png` 用于透明对比；`verification.json` 记录布局、透明度对比、性能与源码、程序 SHA-256。

复验：两端消费者运行 `--placement-smoke`，设置 `SLINTFORGE_PLOT_QA_DIR` 为对应目录；运行 `python tools/verify_plot_placement.py build/plot-overlay-qa --require-overlay`。

常用程序已重新构建：`target/debug/slintforge-gallery-rust.exe`、`build/cpp/Gallery/cpp/Debug/slintforge_gallery_cpp.exe`。退出旧 Gallery 后重新打开即可使用更新。
