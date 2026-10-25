use slintforge_runtime_api::models::*;
use slintforge_runtime_api::{Arena, Wire};
pub fn color_from_hex(text: &str) -> Option<slint::Color> {
    let text = text.to_owned();
    let mut arena = Arena::default();
    let args = ArgsColorParse {
        text: text.view(&mut arena),
    };
    crate::runtime::value_or::<Option<u32>, _>(OP_COLORPARSE, &args, || None).map(crate::wire_color_from)
}
pub fn color_to_hex(color: slint::Color) -> String {
    crate::runtime::value_or::<String, _>(
        OP_COLORTEXT,
        &ArgsColorText {
            color: crate::wire_color(color),
        },
        String::new,
    )
}
pub fn screen_color() -> slint::Color {
    crate::wire_color_from(crate::runtime::value_or::<u32, _>(
        OP_SCREENCOLOR,
        &ArgsScreenColor { reserved: 0 },
        || 0xff00_0000,
    ))
}
pub fn screen_color_input() -> i32 {
    // 失败时按 Esc（2）处理，让屏幕取色结束而不是持续轮询。
    crate::runtime::value_or::<i32, _>(OP_SCREENINPUT, &ArgsScreenInput { reserved: 0 }, || 2)
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
