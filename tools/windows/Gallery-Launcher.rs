// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#![windows_subsystem = "windows"]

use std::ffi::{c_void, OsStr};
use std::os::windows::ffi::{OsStrExt, OsStringExt};
use std::os::windows::process::CommandExt;
use std::path::{Path, PathBuf};

// 此入口单独静态链接 CRT，缺少 Gallery 的运行库时仍能显示安装说明。
#[link(name = "kernel32")]
extern "system" {
    #[link_name = "GetSystemDirectoryW"]
    fn forge_system_directory(buffer: *mut u16, size: u32) -> u32;
}
#[link(name = "version")]
extern "system" {
    #[link_name = "GetFileVersionInfoSizeW"]
    fn forge_version_size(filename: *const u16, handle: *mut u32) -> u32;
    #[link_name = "GetFileVersionInfoW"]
    fn forge_version_info(filename: *const u16, handle: u32, size: u32, data: *mut c_void) -> i32;
    #[link_name = "VerQueryValueW"]
    fn forge_query_version(data: *const c_void, key: *const u16, value: *mut *mut c_void, size: *mut u32) -> i32;
}
#[link(name = "user32")]
extern "system" {
    #[link_name = "MessageBoxW"]
    fn forge_message_box(window: isize, text: *const u16, title: *const u16, flags: u32) -> i32;
}

#[repr(C)]
#[derive(Clone, Copy)]
struct ForgeFixedVersion {
    signature: u32,
    structure_version: u32,
    file_version_ms: u32,
    file_version_ls: u32,
    product_version_ms: u32,
    product_version_ls: u32,
    flags_mask: u32,
    flags: u32,
    os: u32,
    file_type: u32,
    subtype: u32,
    date_ms: u32,
    date_ls: u32,
}

fn forge_wide(text: &OsStr) -> Vec<u16> {
    text.encode_wide().chain(std::iter::once(0)).collect()
}

fn forge_runtime_file_ready(path: &Path) -> bool {
    let filename = forge_wide(path.as_os_str());
    let size = unsafe { forge_version_size(filename.as_ptr(), std::ptr::null_mut()) };
    if size == 0 { return false; }
    let mut data = vec![0u8; size as usize];
    if unsafe { forge_version_info(filename.as_ptr(), 0, size, data.as_mut_ptr().cast()) } == 0 {
        return false;
    }
    let mut value = std::ptr::null_mut();
    let mut length = 0;
    let key = forge_wide(OsStr::new("\\"));
    if unsafe { forge_query_version(data.as_ptr().cast(), key.as_ptr(), &mut value, &mut length) } == 0
        || value.is_null() || length < std::mem::size_of::<ForgeFixedVersion>() as u32 {
        return false;
    }
    // 版本资源返回的是字节缓冲区中的地址，不能假定它满足结构体对齐。
    let version = unsafe { std::ptr::read_unaligned(value.cast::<ForgeFixedVersion>()) };
    version.signature == 0xfeef04bd && version.file_version_ms >= ((14 << 16) | 51)
}

fn forge_runtime_ready() -> bool {
    let mut buffer = vec![0u16; 32768];
    let length = unsafe { forge_system_directory(buffer.as_mut_ptr(), buffer.len() as u32) } as usize;
    if length == 0 || length >= buffer.len() { return false; }
    let system = PathBuf::from(std::ffi::OsString::from_wide(&buffer[..length]));
    ["MSVCP140.dll", "MSVCP140_ATOMIC_WAIT.dll", "VCRUNTIME140.dll", "VCRUNTIME140_1.dll"]
        .iter().all(|name| forge_runtime_file_ready(&system.join(name)))
}

fn forge_alert(title: &str, message: &str) {
    let title = forge_wide(OsStr::new(title));
    let message = forge_wide(OsStr::new(message));
    unsafe { forge_message_box(0, message.as_ptr(), title.as_ptr(), 0x40); }
}

fn main() {
    let ready = forge_runtime_ready();
    // 检查模式与正常启动共用判断逻辑，不显示窗口，不执行安装程序。
    if std::env::args_os().any(|arg| arg == OsStr::new("/check") || arg == OsStr::new("--check")) {
        std::process::exit(if ready { 0 } else { 2 });
    }
    let executable = match std::env::current_exe() {
        Ok(path) => path,
        Err(error) => {
            forge_alert("SlintForge - Launch Failed", &format!("Unable to locate the application folder.\n{error}"));
            std::process::exit(1);
        }
    };
    let root = executable.parent().expect("Windows executable has a parent directory");
    if !ready {
        forge_alert("SlintForge - Runtime Required", &format!(
            "Microsoft Visual C++ x64 Runtime is missing or outdated.\n\nPlease manually run the bundled installer:\n{}\n\nThen start SlintForge Gallery again. No compiler or Visual Studio is needed.",
            root.join("prerequisites").join("vc_redist.x64.exe").display()
        ));
        std::process::exit(2);
    }
    let cpp = executable.file_stem().is_some_and(|name| name.to_string_lossy().eq_ignore_ascii_case("Gallery-Cpp"));
    let application = if cpp { root.join("cpp").join("slintforge_gallery_cpp.exe") }
        else { root.join("rust").join("slintforge-gallery-rust.exe") };
    // 宿主保留命令行验收接口；通过桌面入口运行时隐藏它的控制台窗口。
    if let Err(error) = std::process::Command::new(&application).current_dir(root).creation_flags(0x0800_0000).spawn() {
        forge_alert("SlintForge - Launch Failed", &format!("Unable to start {}.\n\n{error}", application.display()));
        std::process::exit(1);
    }
}
