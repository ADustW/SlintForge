// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

/// 颜色文本采用 RRGGBB 或 RRGGBBAA，六位颜色默认不透明。
pub fn color_from_hex(text: &str) -> Option<crate::pixels::Color> {
    let value = text.strip_prefix('#').unwrap_or(text);
    if !matches!(value.len(), 6 | 8) || !value.bytes().all(|ch| ch.is_ascii_hexdigit()) {
        return None;
    }
    let mut packed = u32::from_str_radix(value, 16).ok()?;
    if value.len() == 6 {
        packed = (packed << 8) | 255;
    }
    Some(crate::pixels::Color::from_argb_u8(
        packed as u8,
        (packed >> 24) as u8,
        (packed >> 16) as u8,
        (packed >> 8) as u8,
    ))
}

pub fn color_to_hex(color: crate::pixels::Color) -> String {
    if color.alpha() == 255 {
        format!(
            "#{:02X}{:02X}{:02X}",
            color.red(),
            color.green(),
            color.blue()
        )
    } else {
        format!(
            "#{:02X}{:02X}{:02X}{:02X}",
            color.red(),
            color.green(),
            color.blue(),
            color.alpha()
        )
    }
}

pub fn screen_color() -> crate::pixels::Color {
    #[cfg(windows)]
    unsafe {
        let mut point = ForgeColorPoint { x: 0, y: 0 };
        if GetCursorPos(&mut point) != 0 {
            let screen = GetDC(std::ptr::null_mut());
            if !screen.is_null() {
                let pixel = GetPixel(screen, point.x, point.y);
                ReleaseDC(std::ptr::null_mut(), screen);
                if pixel != u32::MAX {
                    return crate::pixels::Color::from_rgb_u8(
                        pixel as u8,
                        (pixel >> 8) as u8,
                        (pixel >> 16) as u8,
                    );
                }
            }
        }
    }
    crate::pixels::Color::default()
}

pub fn screen_color_input() -> i32 {
    #[cfg(windows)]
    unsafe {
        if GetAsyncKeyState(0x1B) < 0 {
            return 2;
        }
        if GetAsyncKeyState(0x0D) < 0 {
            return 3;
        }
        if GetAsyncKeyState(0x01) < 0 {
            return 1;
        }
    }
    0
}

#[cfg(windows)]
#[repr(C)]
struct ForgeColorPoint {
    x: i32,
    y: i32,
}
#[cfg(windows)]
#[link(name = "user32")]
extern "system" {
    fn GetCursorPos(point: *mut ForgeColorPoint) -> i32;
    fn GetDC(window: *mut std::ffi::c_void) -> *mut std::ffi::c_void;
    fn ReleaseDC(window: *mut std::ffi::c_void, context: *mut std::ffi::c_void) -> i32;
    fn GetAsyncKeyState(key: i32) -> i16;
}
#[cfg(windows)]
#[link(name = "gdi32")]
extern "system" {
    fn GetPixel(context: *mut std::ffi::c_void, x: i32, y: i32) -> u32;
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn hex_round_trips_rgb_rgba_and_rejects_malformed_values() {
        for (text, expected) in [
            ("#0078d4", "#0078D4"),
            ("FFFFFF80", "#FFFFFF80"),
            ("00000000", "#00000000"),
        ] {
            assert_eq!(color_to_hex(color_from_hex(text).unwrap()), expected);
        }
        for text in ["", "#fff", "#12345G", "#123456789", "１２３４５６"] {
            assert!(color_from_hex(text).is_none());
        }
    }
}
