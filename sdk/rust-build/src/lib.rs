//! 不加载运行库的 Cargo 构建辅助入口。
use std::path::{Path, PathBuf};
pub type BuildError = Box<dyn std::error::Error + Send + Sync>;

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
