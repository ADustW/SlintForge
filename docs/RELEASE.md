# SlintForge Windows 发布与接入

每次发布保存到新的 `dist/dist_SlintForge_年月日_时分秒`。该目录只包含一个未压缩的 `SlintForge-版本` 文件夹和一个 Setup 安装包。两者使用同一份载荷，不创建 ZIP，只包含公共 Rust 接口封装源码，不包含核心实现源码、Gallery 源码、测试、业务示例或构建缓存。历史发布保持原样。

## 发布目录

- `Gallery-Rust.exe`、`Gallery-Cpp.exe`：两个 Gallery 启动入口，实际程序位于 `rust/`、`cpp/`。
- `sdk/cpp/`：Debug/Release 预编译静态库、公共头文件、CMake 接入文件及共享组件。
- `sdk/rust/`：Rust 公共安全封装、计算 DLL、UI 编译器和接入入口；`sdk/runtime-api/` 为 C 数据契约，`sdk/rust-build/` 为不加载 DLL 的构建辅助包。
- `sdk/slint/`：Slint 1.18.0 官方 C++ SDK 接入依赖。
- `docs/`：API 及接入说明，选择器与按钮参见 [控件 API](PICKERS.md)。
- `licenses/`、`prerequisites/`：必要许可与官方 Visual C++ x64 运行库安装文件。

公共 `.h/.hpp` 是 C++ 使用接口；`.slint` 是组件库直接使用所必需的组件文件。发布包提供 Rust 公共类型、接口转换和绑定宏源码，核心算法 `.cpp/.rs` 与完整工程不提供。详细构建、截图、外部消费者及安装验收证据存放在工作区 `build/release-evidence-*`。

## Rust 二进制 SDK

需要 Windows x64、Rust 1.98.0 和 MSVC 链接环境，支持和验收统一以 Rust 1.98.0 为基线。客户编译接口封装与 Slint 1.18.0，核心 DLL 通过版本化 C ABI 调用。依赖首次由 Cargo 联网获取，已有缓存可使用 `-Offline`；标准 Cargo 接入见 [DLL 接入说明](RUST_DLL.md)。

应用自行编写 `app.slint` 与 `main.rs`；Rust 文件使用以下入口引用生成的 UI：

```rust
include!(env!("SLINTFORGE_GENERATED_UI"));
use slint::ComponentHandle;
fn main() {
    let ui = AppWindow::new().unwrap();
    ui.run().unwrap();
}
```

在已加载 MSVC 工具环境的 PowerShell 中执行：

```powershell
& "安装目录/sdk/rust/Compile-Rust.ps1" -SourceFile ./main.rs -UiFile ./app.slint -OutputFile ./app.exe
```

该入口只编译公共封装与客户应用，不编译 SlintForge 核心实现。它使用锁定的 Slint 1.18.0，自动将计算 DLL 复制到应用 EXE 同目录。首次构建会增加依赖获取和编译时间，后续使用 Cargo 缓存；构建目录不属于交付包。需要宿主服务时，在显示窗口前绑定，详见 [接入说明](INTEGRATION.md)。

## C++ SDK

CMake 配置时指定包内 `sdk/cpp/lib/cmake/SlintForge` 为 `SlintForge_DIR`，以及 `sdk/slint/lib/cmake/Slint` 为 `Slint_DIR`。项目使用 C++20、`find_package(SlintForge CONFIG REQUIRED)`、`SlintForge::Sdk` 和 `slintforge_target_sources(目标 界面文件)`。导入目标按 Debug/Release 自动选择库，界面编译时自动找到公共组件并嵌入资源。

## Gallery 与运行库

两个启动入口先检查运行库，缺失时返回退出码 2 并提示用户手动执行 `prerequisites/vc_redist.x64.exe`。安装包只复制该安装文件，不自动修改系统运行库。C++ Gallery 的 Slint DLL 从自身目录加载。

本机安装、卸载和系统路径隔离运行验收与干净 Windows 验收分别记录。只有完成对应实际测试才能认定该环境通过。
