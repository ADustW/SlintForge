# Rust DLL SDK 接入

版本：0.1.2。首期平台：Windows x64/MSVC，Slint 1.18.0。

Rust 业务代码仍使用原生 Slint 的组件、模型、图像和事件循环。公共类型、路由导航、五类绑定宏和专业绘图接口保持兼容；计算实现通过 `slintforge_runtime.dll` 调用。公共封装负责所有 FFI 转换、结果复制与释放，客户无需手写 DLL 调用。应用端的 Slint 对象、视图缓存和实时点缓冲保持本地，批量绘图直接调用 DLL，避免逐点传输历史数据。

## 标准 Cargo 接入

支持和验收基线为 Rust 1.98.0，客户使用该版本和 MSVC 链接环境。公共接口的最低编译版本为 Rust 1.98.0；核心 DLL 也使用 Rust 1.98.0 构建，客户端通过版本化 C ABI 接入。

```toml
[dependencies]
slint = { version = "=1.18.0", features = ["raw-window-handle-06"] }
slintforge = { path = "安装目录/sdk/rust" }

[build-dependencies]
slintforge-build = { path = "安装目录/sdk/rust-build" }

[profile.dev.package."*"]
opt-level = 3
```

客户的 `build.rs`：

```rust
fn main() {
    slintforge_build::compile_ui("app.slint").expect("界面编译失败");
}
```

客户的 `main.rs`：

```rust
slint::include_modules!();
use slint::ComponentHandle;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    slintforge::runtime::initialize()?;
    let ui = AppWindow::new()?;
    // 按使用的能力，继续调用原有 slintforge::bind_* 服务宏。
    ui.run()?;
    Ok(())
}
```

显式初始化可以提前处理缺失 DLL 或 ABI 不兼容的错误；绑定宏也会自动初始化。已有 `slint-build` 工程可继续使用原构建方式，将 `slintforge_build::ui_library_path()` 传给 `with_library_paths()`。构建辅助包不加载计算 DLL。

## 原脚本入口

```powershell
& "安装目录/sdk/rust/Compile-Rust.ps1" -SourceFile ./main.rs -UiFile ./app.slint -OutputFile ./app.exe
```

原有三个必填参数不变；原来的 `include!(env!("SLINTFORGE_GENERATED_UI"));` 仍可使用。脚本内部建立隔离 Cargo 消费工程，自动把 DLL 复制到 EXE 同目录。可添加 `-Configuration Debug`、`-Offline`、`-TargetDirectory` 和 `-Jobs`。默认使用当前客户 Cargo 工具链，编译目录位于用户 LocalAppData 的 SlintForge 缓存。

首次构建需要联网获取并编译 Slint 等依赖；已有完整缓存与锁文件时可离线构建。SDK 主包体积、首次下载量、冷/热构建时间和本地编译缓存分别统计。当前内部验证包提供公共接口源码，但核心实现源码不随包交付；对外分发二进制前须满足 [对应源码要求](RELEASE.md)。

## 发布应用

应用 EXE 同目录必须附带 `slintforge_runtime.dll`，并满足 Visual C++ x64 运行库要求。原脚本与 `compile_ui()` 会自动部署开发输出目录；自定义发布目录通过 `slintforge_build::deploy_runtime(output_dir)` 复制。

加载顺序为显式 `initialize_from(path)`、EXE 同目录、SDK 接口包内的运行库目录。正式发布应使用 EXE 同目录的 DLL，并在原 SDK 不可访问、系统路径隔离的环境验证。成功初始化后不支持替换或卸载运行库。

## ABI 与内存

`slintforge_runtime_get_api_v2` 返回带 ABI 版本 2、函数表大小和契约指纹的函数表。旧版入口、版本或指纹不匹配都会明确报错。旧图表操作编号 21–23 永久保留，不再分配；其他操作编号保持原值。输入文本、数组和像素视图只在同步调用期间借用，状态对象使用不透明句柄；DLL 输出由 DLL 释放，封装在释放之前完成复制。Slint 对象、Rust 集合和客户端闭包不跨 DLL 传递。Rust panic 与 C++ 异常分别在各自入口内部处理。

所有 Slint 属性、模型通知和回调仍在 UI 线程执行，线程约定见 [接入指南](INTEGRATION.md)。
