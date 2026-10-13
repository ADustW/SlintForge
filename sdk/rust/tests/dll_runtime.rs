use slintforge::{NavItem, NavItemKind, NavTree, Router};

#[test]
fn cloned_handles_keep_independent_state_and_survive_original_drop() {
    let mut router = Router::new(["首页", "详情"], "首页").unwrap();
    router.go("详情", vec![]).unwrap();
    let mut clone = router.clone();
    drop(router);
    assert_eq!(clone.back().unwrap().id, "首页");
    let items = vec![
        NavItem::new("root", None, "分组", "", NavItemKind::Group),
        NavItem::new("child", Some("root"), "页面", "", NavItemKind::Page),
    ];
    let tree = NavTree::new(items).unwrap();
    let mut copy = tree.clone();
    copy.toggle("root").unwrap();
    assert_eq!(tree.visible_items().len(), 1);
    drop(tree);
    assert_eq!(copy.visible_items().len(), 2);
}

#[test]
fn concurrent_calls_return_owned_outputs() {
    let threads = (0..8)
        .map(|n| {
            std::thread::spawn(move || {
                for index in 0..20 {
                    let input = format!("并发 {n}/{index}");
                    let (pixels, modules) = slintforge::qr_code_pixels(&input).unwrap();
                    drop(input);
                    assert!(modules > 0);
                    assert_eq!(
                        pixels.as_bytes().len(),
                        pixels.width() as usize * pixels.height() as usize * 4
                    );
                    let lines = slintforge::highlight_syntax("let 中文 = 3;", "rust");
                    assert_eq!(lines[0].plain, "let 中文 = 3;");
                }
            })
        })
        .collect::<Vec<_>>();
    for thread in threads {
        thread.join().unwrap();
    }
}

#[test]
fn loader_child() {
    let Some(path) = std::env::var_os("SLINTFORGE_TEST_DLL") else {
        return;
    };
    let expected = std::env::var("SLINTFORGE_TEST_DLL_ERROR").unwrap();
    let error = slintforge::runtime::initialize_from(path)
        .unwrap_err()
        .to_string();
    assert!(error.contains(&expected), "{error}");
}

#[cfg(windows)]
#[test]
fn loader_rejects_missing_wrong_arch_and_incompatible_abi() {
    let root = std::env::temp_dir().join(format!("SlintForge DLL 边界 {}", std::process::id()));
    std::fs::create_dir_all(&root).unwrap();
    let source = root.join("incompatible.c");
    let library = root.join("incompatible.dll");
    std::fs::write(&source,"struct ForgeMockHeader { unsigned version; unsigned size; };\n__declspec(dllexport) const void* slintforge_runtime_get_api_v1(void) { static const struct ForgeMockHeader header = { 999, 8 }; return &header; }\n").unwrap();
    let result = std::process::Command::new("cl.exe")
        .current_dir(&root)
        .args(["/nologo", "/LD"])
        .arg(&source)
        .arg(format!("/Fe:{}", library.display()))
        .output()
        .unwrap();
    assert!(
        result.status.success(),
        "{}",
        String::from_utf8_lossy(&result.stderr)
    );
    let wrong = root.join("wrong-arch.dll");
    let mut bytes = vec![0u8; 70];
    bytes[..2].copy_from_slice(b"MZ");
    bytes[60..64].copy_from_slice(&64u32.to_le_bytes());
    bytes[64..68].copy_from_slice(b"PE\0\0");
    bytes[68..70].copy_from_slice(&0x14cu16.to_le_bytes());
    std::fs::write(&wrong, bytes).unwrap();
    let cases = [
        (root.join("missing.dll"), "无法加载"),
        (wrong, "不是 Windows x64"),
        (library, "ABI 不兼容"),
    ];
    for (path, message) in cases {
        let output = std::process::Command::new(std::env::current_exe().unwrap())
            .args(["--exact", "loader_child", "--nocapture"])
            .env("SLINTFORGE_TEST_DLL", path)
            .env("SLINTFORGE_TEST_DLL_ERROR", message)
            .output()
            .unwrap();
        assert!(
            output.status.success(),
            "{}{}",
            String::from_utf8_lossy(&output.stdout),
            String::from_utf8_lossy(&output.stderr)
        );
    }
}
