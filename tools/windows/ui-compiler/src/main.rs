use std::{collections::HashMap, path::PathBuf};
fn main() {
    let args: Vec<_> = std::env::args().collect();
    assert!(
        args.len() >= 4,
        "usage: compiler ui library output-directory [translations]"
    );
    let file = PathBuf::from(&args[1]).canonicalize().unwrap();
    let output = PathBuf::from(&args[3]);
    std::fs::create_dir_all(&output).unwrap();
    let output = output.canonicalize().unwrap();
    std::env::set_var("OUT_DIR", &output);
    std::env::set_var("CARGO_MANIFEST_DIR", file.parent().unwrap());
    std::env::set_var("SLINT_STYLE", "fluent");
    std::env::set_var("SLINT_EMBED_RESOURCES", "true");
    let mut config =
        slint_build::CompilerConfiguration::new().with_library_paths(HashMap::from([(
            "slintforge".into(),
            PathBuf::from(&args[2]).canonicalize().unwrap(),
        )]));
    if args.len() > 4 {
        config = config
            .with_bundled_translations(PathBuf::from(&args[4]).canonicalize().unwrap())
            .with_default_translation_context(slint_build::DefaultTranslationContext::None);
    }
    slint_build::compile_with_config(&file, config).unwrap();
    println!("UI_COMPILE_OK {}", file.display());
}
