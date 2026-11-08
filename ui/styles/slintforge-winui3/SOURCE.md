# Slint 标准控件来源

`upstream/` 来自 Slint 1.18.0 的 `i-slint-compiler/widgets`，逐文件保持原样，SHA-256 见 `upstream-sha256.json`。许可见同目录 `LICENSE-*`。

未引入上游 `styling.slint`、`style-base.slint` 与 `color-scheme.slint`；同名主题适配文件由 SlintForge 提供。控件在编译时通过 include 路径读取本项目主题，不修改上游文件、依赖缓存或 Slint SDK。
