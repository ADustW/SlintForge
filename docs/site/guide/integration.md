# 接入指南

UI 符号统一从公共入口 `ui/slintforge.slint` 导入。Rust 消费工程使用 `slintforge` crate，C++ 消费工程包含 `sdk/cpp/include` 下的头文件。绑定截图、Widget、Grid、Plot 或 Flow 服务前先初始化运行库。

构建站点前运行 `npm run docs:check`，使用 `npm run docs:build` 生成 `build/api-docs-site`。
