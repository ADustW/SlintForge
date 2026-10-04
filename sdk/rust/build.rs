fn main() {
    if std::env::var("CARGO_CFG_TARGET_OS").as_deref() == Ok("windows") {
        for library in ["gdi32", "windowscodecs", "ole32"] {
            println!("cargo:rustc-link-lib={library}");
        }
    }
    println!("cargo:rerun-if-changed=../common/ForgePlotCore.h");
    println!("cargo:rerun-if-changed=../common/ForgePlotCore.cpp");
    cc::Build::new()
        .cpp(true)
        // 数值和路径循环在 Debug 中也保持交互性能，调试信息仍由 Cargo 管理。
        .opt_level(2)
        .file("../common/ForgePlotCore.cpp")
        .flag_if_supported("/std:c++20")
        .flag_if_supported("/utf-8")
        .flag_if_supported("-std=c++20")
        .warnings(true)
        .compile("slintforge_plot_core");
}
