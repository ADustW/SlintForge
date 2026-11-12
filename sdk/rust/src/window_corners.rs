// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#[cfg(windows)]
use raw_window_handle::{HasWindowHandle, RawWindowHandle};

#[cfg(windows)]
#[link(name = "dwmapi")]
extern "system" {
    fn DwmSetWindowAttribute(
        hwnd: *mut std::ffi::c_void,
        attribute: u32,
        value: *const std::ffi::c_void,
        size: u32,
    ) -> i32;
}

/// 向 Windows 11 请求圆角或直角；未创建窗口句柄或系统不支持时返回 false。
pub fn apply_window_corners(window: &slint::Window, rounded: bool) -> bool {
    #[cfg(windows)]
    {
        let handle = window.window_handle();
        let Ok(native) = handle.window_handle() else {
            return false;
        };
        let RawWindowHandle::Win32(win32) = native.as_raw() else {
            return false;
        };
        let value: i32 = if rounded { 2 } else { 1 };
        // 33 是 DWMWA_WINDOW_CORNER_PREFERENCE；DWM API 返回非负值表示调用成功。
        unsafe {
            DwmSetWindowAttribute(
                win32.hwnd.get() as *mut std::ffi::c_void,
                33,
                (&value as *const i32).cast(),
                std::mem::size_of::<i32>() as u32,
            ) >= 0
        }
    }
    #[cfg(not(windows))]
    {
        let _ = (window, rounded);
        false
    }
}
