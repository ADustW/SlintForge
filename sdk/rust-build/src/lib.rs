// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

//! 不加载运行库的 Cargo 构建辅助入口。
use std::path::{Path, PathBuf};
pub type BuildError = Box<dyn std::error::Error + Send + Sync>;

/// 启用固定版本标准控件的原有属性标记，并为内联 Slint 测试传递同一配置。
pub fn configure_style() {
    std::env::set_var("SLINT_STYLE", "slintforge-winui3");
    std::env::set_var("SLINT_ENABLE_EXPERIMENTAL_FEATURES", "1");
    println!("cargo:rustc-env=SLINT_STYLE=slintforge-winui3");
    println!("cargo:rustc-env=SLINT_ENABLE_EXPERIMENTAL_FEATURES=1");
    // Slint 的相对 include_path 会逐文件重定位，内联测试必须使用构建时生成的绝对路径。
    let mut inline = String::from("macro_rules! slintforge_ui { ($($tokens:tt)*) => { slint::slint! { #[style = \"slintforge-winui3\"]\n");
    for path in style_include_paths() {
        let path = path.to_string_lossy().replace('\\', "/");
        inline.push_str(&format!("#[include_path = {path:?}]\n"));
    }
    inline.push_str("$($tokens)* } }; }\n");
    if let Some(output) = std::env::var_os("OUT_DIR") {
        std::fs::write(PathBuf::from(output).join("slintforge_style.rs"), inline)
            .expect("无法生成内联 Slint 样式入口");
    }
}

/// 所有路径均为绝对路径，避免消费者位于不同目录时回退到其他样式。
pub fn style_include_paths() -> Vec<PathBuf> {
    let library = ui_library_path();
    let styles = library.parent().expect("UI 入口需要父目录").join("styles");
    let style = styles.join("slintforge-winui3");
    vec![styles, style.clone(), style.join("upstream/fluent"), style.join("upstream/common")]
}

pub fn ui_library_path() -> PathBuf {
    println!("cargo:rerun-if-env-changed=SLINTFORGE_UI_LIBRARY");
    if let Some(path) = std::env::var_os("SLINTFORGE_UI_LIBRARY") {
        return PathBuf::from(path);
    }
    let base = Path::new(env!("CARGO_MANIFEST_DIR"));
    let packaged = base.join("../cpp/share/slintforge/ui/slintforge.slint");
    if packaged.is_file() {
        packaged
    } else {
        base.join("../../ui/slintforge.slint")
    }
}
pub fn runtime_library_path() -> Result<PathBuf, BuildError> {
    println!("cargo:rerun-if-env-changed=SLINTFORGE_RUNTIME_LIBRARY");
    if let Some(path) = std::env::var_os("SLINTFORGE_RUNTIME_LIBRARY") {
        let path = PathBuf::from(path);
        if path.is_file() {
            return Ok(path);
        }
    }
    let base = Path::new(env!("CARGO_MANIFEST_DIR"));
    let packaged = base.join("../rust/runtime/slintforge_runtime.dll");
    if packaged.is_file() {
        return Ok(packaged);
    }
    let root = base.join("../..");
    let target = std::env::var_os("CARGO_TARGET_DIR")
        .map(PathBuf::from)
        .map(|p| if p.is_absolute() { p } else { root.join(p) })
        .unwrap_or_else(|| root.join("target"));
    let profile = std::env::var("PROFILE").unwrap_or_else(|_| "debug".into());
    for directory in [
        target.join(&profile),
        target.join("release"),
        target.join("debug"),
    ] {
        let file = directory.join("slintforge_runtime.dll");
        if file.is_file() {
            return Ok(file);
        }
    }
    Err("找不到运行 DLL，请先构建 slintforge-runtime 或使用完整 SDK 包".into())
}
pub fn deploy_runtime(output_dir: impl AsRef<Path>) -> Result<PathBuf, BuildError> {
    let source = runtime_library_path()?.canonicalize()?;
    let output = output_dir.as_ref();
    std::fs::create_dir_all(output)?;
    let destination = output.join("slintforge_runtime.dll");
    if destination.is_file() && destination.canonicalize()? == source {
        return Ok(destination);
    }
    std::fs::copy(source, &destination)?;
    Ok(destination)
}
pub fn compile_ui(ui: impl AsRef<Path>) -> Result<(), BuildError> {
    compile_ui_with_translations(ui, None::<&Path>)
}
pub fn compile_ui_with_translations(
    ui: impl AsRef<Path>,
    translations: Option<impl AsRef<Path>>,
) -> Result<(), BuildError> {
    configure_style();
    let base = Path::new(env!("CARGO_MANIFEST_DIR"));
    let packaged = base.join("../rust/bin/slintforge-ui-compiler.exe");
    let compiler = if packaged.is_file() {
        packaged
    } else {
        base.join("../../build/ui-compiler-release/release/slintforge-ui-compiler.exe")
    };
    let ui = ui.as_ref().canonicalize()?;
    let output = PathBuf::from(
        std::env::var_os("OUT_DIR").ok_or("compile_ui 需要在 Cargo build.rs 中调用")?,
    );
    let mut command = std::process::Command::new(compiler);
    command.arg(&ui).arg(ui_library_path()).arg(&output);
    if let Some(path) = translations {
        command.arg(path.as_ref());
    }
    let result = command.output()?;
    if !result.status.success() {
        return Err(format!(
            "Slint 界面编译失败：{}",
            String::from_utf8_lossy(&result.stderr)
        )
        .into());
    }
    for line in String::from_utf8_lossy(&result.stdout).lines() {
        if line.starts_with("cargo:") {
            println!("{line}");
        }
    }
    let generated = output
        .join(ui.file_stem().ok_or("界面文件名无效")?)
        .with_extension("rs");
    println!("cargo:rerun-if-changed={}", ui.display());
    println!("cargo:rerun-if-changed={}", ui_library_path().display());
    println!(
        "cargo:rustc-env=SLINT_INCLUDE_GENERATED={}",
        generated.display()
    );
    println!(
        "cargo:rustc-env=SLINTFORGE_GENERATED_UI={}",
        generated.display()
    );
    println!("cargo:rustc-link-arg=/STACK:8388608");
    // Cargo 构建布局固定为 profile/build/包/out；同时照顾测试和示例。
    if let Some(profile) = output.ancestors().nth(3) {
        deploy_runtime(profile)?;
        deploy_runtime(profile.join("deps"))?;
        deploy_runtime(profile.join("examples"))?;
    }
    Ok(())
}
