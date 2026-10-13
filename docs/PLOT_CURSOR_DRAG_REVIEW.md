# ForgePlot 实时光标拖动修复

> 阶段记录：本文保留该组件实施时的范围、测试数量和验证结果；v0.1.1 当前能力见接入与交付说明，本次验收以时间戳发布目录中的 ACCEPTANCE.md 和 verification 为准。

验证日期：2026-10-04；Windows x64，Slint 1.18.0。C++ 使用 `D:\.clion-env\qt-6.8.3-msvc.bat` 环境。

## 故障与修复

原光标 TouchArea 放在 `scene.rects` 的动态重复项中。实时数据刷新返回新的绘图区模型，会重建重复项并丢失正在按住的鼠标捕获。旧静态测试只在刷新前后之间发送一次完整拖动，没有覆盖按住期间的连续更新。

修复前新增复现测试失败：追加第一批实时数据后，光标 A 仍为 122.600006，鼠标目标对应 122.81615。

光标命中和拖动统一交给持续存在的图表 TouchArea；可命中竖线左右 8px 及顶部字母标记。按下时锁定 A/B，交叉时保持正在拖动的光标；实时窗口更新期间保持光标位于鼠标处。拖动不触发波形平移、选择和双击缩放，并显示左右拖动指针。Esc、取消输入、禁用组件和关闭测量会清除拖动状态。

## 验证

- Gallery 图表测试 5/5，包括新增的 A/B 按住期间连续追加、刷新、拖动断言；原有静态光标拖动、数据保留、测量、选择、平移和缩放测试通过。
- Rust/C++ 实际窗口分别通过 `--cursor-drag-smoke`。以累计 130 秒模拟数据起步，分别拖动 A 竖线、B 顶部标记，每个光标五步，每步之间追加并刷新数据；每端 10 步、10 次更新。
- 每步光标像素与鼠标目标误差小于 0.02px；视图仅按新数据时间推进，未被光标拖动平移。A 穿过 B 后仍保持拖动 A。
- 每端保存两条光标拖动前后的四张截图与 `drag.csv`。测试使用实际窗口指针事件及计时器，模拟时间加速输入。
- 两端 Gallery 重新构建，并验证共享图表示例页面能正常渲染；SDK 安装契约与源码核对。

## 证据

本轮证据在 [`build/plot-cursor-drag-qa`](../build/plot-cursor-drag-qa)，之前验证目录保留。`rust-drag/cpp-drag` 包含坐标及截图；`verification.json` 记录每步坐标、误差与相关源码、程序 SHA-256。

复验时设置 `SLINTFORGE_PLOT_QA_DIR` 为对应宿主目录，运行独立消费者 `--cursor-drag-smoke`，再运行本轮 `verify.py`。

常用 Gallery 程序仍为 `target/debug/slintforge-gallery-rust.exe` 和 `build/cpp/Gallery/cpp/Debug/slintforge_gallery_cpp.exe`。退出旧窗口并重新打开后使用更新。另保留 Rust 本轮编译副本 `build/plot-cursor-drag-qa/slintforge-gallery-rust.exe`。
