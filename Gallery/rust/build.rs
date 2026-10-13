use std::collections::HashMap;

fn main() {
    if std::env::var("CARGO_CFG_TARGET_OS").as_deref() == Ok("windows") {
        let _ = embed_resource::compile("gallery.rc", embed_resource::NONE);
        // Gallery 的固定页面同时创建，Windows 主线程需为 Slint 初始化预留栈空间。
        println!("cargo:rustc-link-arg-bin=slintforge-gallery-rust=/STACK:8388608");
    }
    let library = slintforge_build::ui_library_path();
    let config = slint_build::CompilerConfiguration::new()
        .with_library_paths(HashMap::from([("slintforge".into(), library)]))
        .with_bundled_translations("../i18n")
        .with_default_translation_context(slint_build::DefaultTranslationContext::None);
    slint_build::compile_with_config("../ui/gallery.slint", config).unwrap();
}
