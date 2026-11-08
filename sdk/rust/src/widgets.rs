// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

use slint::{Rgba8Pixel, SharedPixelBuffer};
#[derive(Clone, Debug)]
pub struct SyntaxLine {
    pub number: i32,
    pub plain: String,
    pub markdown: String,
}
use slintforge_runtime_api::models::*;
use slintforge_runtime_api::{Arena, Wire};
pub fn qr_code_pixels(
    text: &str,
) -> std::result::Result<(SharedPixelBuffer<Rgba8Pixel>, i32), String> {
    let mut arena = Arena::default();
    let text = text.to_owned();
    let args = ArgsQr {
        text: text.view(&mut arena),
    };
    crate::runtime::invoke::<slintforge_runtime_api::Pair<slintforge_runtime_api::Pixels,i32>,_,_>(OP_QR,&args,|v|(crate::pixels_from_view(v.first),v.second)).map_err(|e|e.message())
}
pub fn highlight_syntax(code: &str, language: &str) -> Vec<SyntaxLine> {
    let mut arena = Arena::default();
    let code = code.to_owned();
    let language = language.to_owned();
    let args = ArgsHighlight {
        code: code.view(&mut arena),
        language: language.view(&mut arena),
    };
    crate::runtime::value_or::<Vec<SyntaxLine>, _>(OP_HIGHLIGHT, &args, Vec::new)
}
pub fn process_backdrop(
    source: &SharedPixelBuffer<Rgba8Pixel>,
    radius: i32,
    glass: bool,
) -> SharedPixelBuffer<Rgba8Pixel> {
    let args = ArgsBackdrop {
        pixels: crate::pixels_view(source),
        radius,
        glass: u8::from(glass),
    };
    crate::runtime::invoke::<slintforge_runtime_api::Pixels, _, _>(
        OP_BACKDROP,
        &args,
        crate::pixels_from_view,
    )
    .expect("像素处理运行库调用失败")
}
#[macro_export]
macro_rules! bind_widget_services {
    ($ui:expr, $service:ty, $qr:ty, $line:ty) => {{
        use slint::ComponentHandle as _;
        let ui = &$ui;
        $crate::runtime::initialize().expect("组件运行库初始化失败");
        let service = ui.global::<$service>();
        service.on_color_to_hex(|color| $crate::color_to_hex(color).into());
        service.on_color_hex_valid(|text| $crate::color_from_hex(text.as_str()).is_some());
        service.on_color_from_hex(|text| $crate::color_from_hex(text.as_str()).unwrap_or_default());
        service.on_screen_color($crate::screen_color);
        service.on_screen_color_input($crate::screen_color_input);
        service.set_screen_color_supported(cfg!(windows));
        service.on_suggestion_index(|suggestions, query, start, direction| {
            use slint::Model as _;
            let prefix = query.as_str().to_lowercase();
            let step = direction.signum();
            if step == 0 {
                return -1;
            }
            let mut index = start;
            while index >= 0 && (index as usize) < suggestions.row_count() {
                if suggestions
                    .row_data(index as usize)
                    .is_some_and(|value| value.as_str().to_lowercase().starts_with(&prefix))
                {
                    return index;
                }
                let Some(next) = index.checked_add(step) else {
                    break;
                };
                index = next;
            }
            -1
        });
        service.on_qr_code(|text| {
            let mut result = <$qr>::default();
            match $crate::qr_code_pixels(text.as_str()) {
                Ok((pixels, modules)) => {
                    result.ok = true;
                    result.image = slint::Image::from_rgba8(pixels);
                    result.modules = modules;
                }
                Err(error) => {
                    result.error = error.into();
                }
            }
            result
        });
        service.on_syntax_lines(|code, language| {
            let lines = $crate::highlight_syntax(code.as_str(), language.as_str())
                .into_iter()
                .map(|source| {
                    let mut line = <$line>::default();
                    line.number = source.number;
                    line.content = slint::StyledText::from_markdown(&source.markdown)
                        .unwrap_or_else(|_| slint::StyledText::from_plain_text(&source.plain));
                    line
                })
                .collect::<Vec<_>>();
            slint::ModelRc::new(slint::VecModel::from(lines))
        });
        service.on_backdrop(|source, radius, glass| {
            source
                .to_rgba8()
                .map(|pixels| {
                    slint::Image::from_rgba8($crate::process_backdrop(&pixels, radius, glass))
                })
                .unwrap_or(source)
        });
        service.set_connected(true);
    }};
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn qr_has_quiet_zone_and_rejects_invalid_payloads() {
        let directory =
            std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../build/component-qa/qr");
        std::fs::create_dir_all(&directory).unwrap();
        for (name, text) in [
            ("ascii", "SlintForge|Rust|C++"),
            ("unicode", "中文组件🙂|SlintForge"),
            ("url", "https://slint.dev"),
        ] {
            let (pixels, modules) = qr_code_pixels(text).unwrap();
            let size = (modules + 8) as u32 * 4;
            assert_eq!((pixels.width(), pixels.height()), (size, size));
            for y in 0..size {
                for x in 0..size {
                    let pixel = pixels.as_slice()[(y * size + x) as usize];
                    assert_eq!(pixel.a, 255);
                    assert!(pixel.r == 0 || pixel.r == 255);
                    assert_eq!((pixel.g, pixel.b), (pixel.r, pixel.r));
                    if x < 16 || y < 16 || x >= size - 16 || y >= size - 16 {
                        assert_eq!(pixel.r, 255);
                    }
                }
            }
            crate::save_capture_png(&pixels, directory.join(format!("rust-{name}.png"))).unwrap();
        }
        for text in ["", "a\0b", &"x".repeat(5000)] {
            assert!(qr_code_pixels(text).is_err());
        }
    }

    #[test]
    fn highlighting_preserves_lines_escapes_markup_and_tracks_comments() {
        let source =
            "fn main() {\r\n\tlet x = \"<script>&_*中文\"; // 42\n/* multi\nend */ return 3;\n}";
        let lines = highlight_syntax(source, "rust");
        assert_eq!(lines.len(), 5);
        assert_eq!(lines[1].number, 2);
        assert_eq!(lines[0].plain, "fn main() {");
        assert_eq!(lines[1].plain, "\tlet x = \"<script>&_*中文\"; // 42");
        assert!(lines[0].markdown.contains("#c39eee"));
        assert!(lines[1].markdown.contains("&lt;script&gt;&amp;"));
        assert!(!lines[1].markdown.contains("<script>"));
        assert!(lines[2].markdown.contains("#8bb58a"));
        assert!(lines[3].markdown.contains("#8bb58a"));
        assert!(lines[3].markdown.contains("#c39eee"));
        for line in &lines {
            slint::StyledText::from_markdown(&line.markdown).expect("词法高亮生成有效富文本");
        }
        for (language, code) in [
            ("python", "def run(): # ready"),
            ("json", "{\"ok\":true,\"n\":42}"),
            ("cpp", "const auto x = 4;"),
            ("slint", "in-out property<int> value: 2;"),
        ] {
            let text = highlight_syntax(code, language);
            assert!(text[0].markdown.contains("#c39eee"));
        }
        assert_eq!(highlight_syntax("a\n", "unknown").len(), 2);
        assert!(!highlight_syntax("fn <x>", "unknown")[0]
            .markdown
            .contains("#c39eee"));
    }

    #[test]
    fn backdrop_blur_handles_alpha_bounds_and_refraction() {
        let mut image = SharedPixelBuffer::<Rgba8Pixel>::new(16, 12);
        for pixel in image.make_mut_slice() {
            *pixel = Rgba8Pixel {
                r: 37,
                g: 64,
                b: 128,
                a: 128,
            };
        }
        assert_eq!(
            process_backdrop(&image, 32, true).as_bytes(),
            image.as_bytes()
        );
        assert_eq!(
            process_backdrop(&image, -10, false).as_bytes(),
            image.as_bytes()
        );
        let mut alpha = SharedPixelBuffer::<Rgba8Pixel>::new(3, 1);
        alpha.make_mut_slice().copy_from_slice(&[
            Rgba8Pixel {
                r: 255,
                g: 0,
                b: 0,
                a: 0,
            },
            Rgba8Pixel {
                r: 0,
                g: 0,
                b: 255,
                a: 255,
            },
            Rgba8Pixel {
                r: 255,
                g: 0,
                b: 0,
                a: 0,
            },
        ]);
        for pixel in process_backdrop(&alpha, 1, false).as_slice() {
            assert_eq!(pixel.r, 0);
            assert_eq!(pixel.b, 255);
            assert!(pixel.a > 0);
        }
        for y in 0..12 {
            for x in 0..16 {
                image.make_mut_slice()[(y * 16 + x) as usize] = Rgba8Pixel {
                    r: (x * 13) as u8,
                    g: (y * 20) as u8,
                    b: 80,
                    a: 255,
                };
            }
        }
        assert_ne!(
            process_backdrop(&image, 2, false).as_bytes(),
            image.as_bytes()
        );
        // 大图的边缘折射必须改变采样，但不修改调用方的图片。
        let mut large = SharedPixelBuffer::<Rgba8Pixel>::new(96, 64);
        for y in 0..64 {
            for x in 0..96 {
                large.make_mut_slice()[(y * 96 + x) as usize] = Rgba8Pixel {
                    r: (x * 2) as u8,
                    g: (y * 3) as u8,
                    b: 80,
                    a: 255,
                };
            }
        }
        assert_ne!(
            process_backdrop(&large, 0, true).as_bytes(),
            large.as_bytes()
        );
        assert_eq!(
            process_backdrop(&large, 99, false).as_bytes(),
            process_backdrop(&large, 32, false).as_bytes()
        );
        assert!(process_backdrop(&SharedPixelBuffer::new(0, 0), 2, true)
            .as_slice()
            .is_empty());
    }
}
