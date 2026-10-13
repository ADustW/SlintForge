use std::collections::HashMap;

fn main() {
    let config = slint_build::CompilerConfiguration::new().with_library_paths(HashMap::from([(
        "slintforge".into(),
        slintforge_build::ui_library_path(),
    )]));
    slint_build::compile_with_config("../ui/minimal.slint", config).unwrap();
}
