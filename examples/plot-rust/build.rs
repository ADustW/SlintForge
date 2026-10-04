fn main() {
    let config=slint_build::CompilerConfiguration::new().with_library_paths(std::collections::HashMap::from([
        ("slintforge".into(),slintforge::ui_library_path())]));
    slint_build::compile_with_config("../ui/professional-plot.slint",config).unwrap();
}
