// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

use std::collections::HashMap;

fn main() {
    slintforge_build::configure_style();
    let config = slint_build::CompilerConfiguration::new().with_style("slintforge-winui3".into()).with_include_paths(slintforge_build::style_include_paths()).with_library_paths(HashMap::from([(
        "slintforge".into(),
        slintforge_build::ui_library_path(),
    )]));
    slint_build::compile_with_config("../ui/minimal.slint", config).unwrap();
}
