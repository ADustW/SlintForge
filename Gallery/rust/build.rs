// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

use std::collections::HashMap;

fn main() {
    slintforge_build::configure_style();
    if std::env::var("CARGO_CFG_TARGET_OS").as_deref() == Ok("windows") {
        let _ = embed_resource::compile("gallery.rc", embed_resource::NONE);
        // Gallery 的固定页面同时创建，Windows 主线程需为 Slint 初始化预留栈空间。
        // 组件页统一模板后每页包含多个示例与 API 表，调试构建的初始化超过 8MB，预留 32MB（仅保留地址空间，按需提交）。
        println!("cargo:rustc-link-arg-bin=slintforge-gallery-rust=/STACK:33554432");
    }
    let library = slintforge_build::ui_library_path();
    let config = slint_build::CompilerConfiguration::new().with_style("slintforge-winui3".into()).with_include_paths(slintforge_build::style_include_paths())
        .with_library_paths(HashMap::from([("slintforge".into(), library)]))
        .with_bundled_translations("../i18n")
        .with_default_translation_context(slint_build::DefaultTranslationContext::None);
    slint_build::compile_with_config("../ui/gallery.slint", config).unwrap();
}
