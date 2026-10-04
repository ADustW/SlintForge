use slint::{Rgba8Pixel, SharedPixelBuffer};

use slintforge_qrcodegen as qrcodegen;

/// 返回带四模块静区的二维码；UTF-8 文本采用 ECI 26。
pub fn qr_code_pixels(
    text: &str,
) -> std::result::Result<(SharedPixelBuffer<Rgba8Pixel>, i32), String> {
    use qrcodegen::{QrCode, QrCodeEcc, QrSegment};
    if text.is_empty() || text.contains('\0') {
        return Err("二维码内容不能为空或包含空字符".into());
    }
    let qr = if text.is_ascii() {
        QrCode::encode_text(text, QrCodeEcc::Medium)
    } else {
        QrCode::encode_segments(
            &[
                QrSegment::make_eci(26),
                QrSegment::make_bytes(text.as_bytes()),
            ],
            QrCodeEcc::Medium,
        )
    }
    .map_err(|_| "二维码内容超过编码容量".to_string())?;
    let modules = qr.size();
    let size = (modules + 8) as u32 * 4;
    let mut image = SharedPixelBuffer::<Rgba8Pixel>::new(size, size);
    for y in 0..size {
        for x in 0..size {
            let color = if qr.get_module(x as i32 / 4 - 4, y as i32 / 4 - 4) {
                0
            } else {
                255
            };
            image.make_mut_slice()[(y * size + x) as usize] = Rgba8Pixel {
                r: color,
                g: color,
                b: color,
                a: 255,
            };
        }
    }
    Ok((image, modules))
}

#[derive(Clone, Debug)]
pub struct SyntaxLine {
    pub number: i32,
    pub plain: String,
    pub markdown: String,
}

fn escaped_token(text: &str) -> String {
    let mut output = String::new();
    for ch in text.chars() {
        match ch {
            '&' => output.push_str("&amp;"),
            '<' => output.push_str("&lt;"),
            '>' => output.push_str("&gt;"),
            ' ' => output.push('\u{a0}'),
            '\t' => output.push_str("\u{a0}\u{a0}\u{a0}\u{a0}"),
            '\\' | '*' | '_' | '[' | ']' | '#' | '`' | '~' => {
                output.push('\\');
                output.push(ch);
            }
            _ => output.push(ch),
        }
    }
    output
}

/// 词法高亮保留源文本；跨行块注释、字符串转义、数字与语言关键字分别着色。
pub fn highlight_syntax(code: &str, language: &str) -> Vec<SyntaxLine> {
    let keywords = match language {
        "rust" => "fn let mut pub use mod struct enum impl trait match if else for while loop return const static true false self Self move async await where type as in crate super",
        "cpp" | "c++" | "c" => "auto const static class struct enum public private protected return if else for while switch case break void int float double bool true false char include define template typename using new delete nullptr virtual override",
        "python" => "def class import from as if elif else for while return True False None pass try except finally with lambda async await in and or not is yield",
        "json" => "true false null",
        "slint" => "import export component inherits property callback in out in-out private public function struct enum if for return true false changed init animate states transitions",
        _ => "",
    };
    let mut block_comment = false;
    code.split('\n')
        .enumerate()
        .map(|(line_index, raw)| {
            let raw = raw.strip_suffix('\r').unwrap_or(raw);
            let chars: Vec<char> = raw.chars().collect();
            let mut markdown = String::new();
            let mut index = 0;
            while index < chars.len() {
                let start = index;
                let color;
                if block_comment {
                    while index < chars.len() {
                        if chars[index] == '*' && chars.get(index + 1) == Some(&'/') {
                            index += 2;
                            block_comment = false;
                            break;
                        }
                        index += 1;
                    }
                    color = "#8bb58a";
                } else if (language == "python" && chars[index] == '#')
                    || (language != "python"
                        && language != "json"
                        && chars[index] == '/'
                        && chars.get(index + 1) == Some(&'/'))
                {
                    index = chars.len();
                    color = "#8bb58a";
                } else if language != "python"
                    && language != "json"
                    && chars[index] == '/'
                    && chars.get(index + 1) == Some(&'*')
                {
                    block_comment = true;
                    index += 2;
                    while index < chars.len() {
                        if chars[index] == '*' && chars.get(index + 1) == Some(&'/') {
                            index += 2;
                            block_comment = false;
                            break;
                        }
                        index += 1;
                    }
                    color = "#8bb58a";
                } else if chars[index] == '"' || chars[index] == '\'' {
                    let quote = chars[index];
                    index += 1;
                    while index < chars.len() {
                        if chars[index] == '\\' {
                            index = (index + 2).min(chars.len());
                        } else {
                            let end = chars[index] == quote;
                            index += 1;
                            if end {
                                break;
                            }
                        }
                    }
                    color = "#efb285";
                } else if chars[index].is_ascii_digit() {
                    index += 1;
                    while index < chars.len()
                        && (chars[index].is_ascii_alphanumeric()
                            || chars[index] == '.'
                            || chars[index] == '_')
                    {
                        index += 1;
                    }
                    color = "#aad4a7";
                } else if chars[index].is_alphabetic() || chars[index] == '_' {
                    index += 1;
                    while index < chars.len()
                        && (chars[index].is_alphanumeric()
                            || chars[index] == '_'
                            || (language == "slint" && chars[index] == '-'))
                    {
                        index += 1;
                    }
                    let word: String = chars[start..index].iter().collect();
                    color = if keywords.split_whitespace().any(|keyword| keyword == word) {
                        "#c39eee"
                    } else {
                        "#dce3f0"
                    };
                } else {
                    index += 1;
                    color = "#dce3f0";
                }
                let token: String = chars[start..index].iter().collect();
                markdown.push_str(&format!(
                    "<font color='{color}'>{}</font>",
                    escaped_token(&token)
                ));
            }
            SyntaxLine {
                number: line_index as i32 + 1,
                plain: raw.into(),
                markdown,
            }
        })
        .collect()
}

fn blur_pass(
    source: &SharedPixelBuffer<Rgba8Pixel>,
    radius: i32,
    horizontal: bool,
) -> SharedPixelBuffer<Rgba8Pixel> {
    let width = source.width() as i32;
    let height = source.height() as i32;
    let mut output = SharedPixelBuffer::<Rgba8Pixel>::new(width as u32, height as u32);
    let (lines, length) = if horizontal {
        (height, width)
    } else {
        (width, height)
    };
    for line in 0..lines {
        let pixel_at = |position: i32| {
            let position = position.clamp(0, length - 1);
            let index = if horizontal {
                line * width + position
            } else {
                position * width + line
            };
            source.as_slice()[index as usize]
        };
        let mut sums = [0_i64; 4];
        let mut add = |pixel: Rgba8Pixel, sign: i64| {
            sums[0] += i64::from(pixel.r) * i64::from(pixel.a) * sign;
            sums[1] += i64::from(pixel.g) * i64::from(pixel.a) * sign;
            sums[2] += i64::from(pixel.b) * i64::from(pixel.a) * sign;
            sums[3] += i64::from(pixel.a) * sign;
        };
        for offset in -radius..=radius {
            add(pixel_at(offset), 1);
        }
        for position in 0..length {
            if position > 0 {
                let old = pixel_at(position - radius - 1);
                let new = pixel_at(position + radius);
                for (channel, delta) in [
                    (
                        0,
                        i64::from(new.r) * i64::from(new.a) - i64::from(old.r) * i64::from(old.a),
                    ),
                    (
                        1,
                        i64::from(new.g) * i64::from(new.a) - i64::from(old.g) * i64::from(old.a),
                    ),
                    (
                        2,
                        i64::from(new.b) * i64::from(new.a) - i64::from(old.b) * i64::from(old.a),
                    ),
                    (3, i64::from(new.a) - i64::from(old.a)),
                ] {
                    sums[channel] += delta;
                }
            }
            let color = |channel| {
                if sums[3] == 0 {
                    0
                } else {
                    (sums[channel] / sums[3]) as u8
                }
            };
            let index = if horizontal {
                line * width + position
            } else {
                position * width + line
            };
            output.make_mut_slice()[index as usize] = Rgba8Pixel {
                r: color(0),
                g: color(1),
                b: color(2),
                a: (sums[3] / i64::from(radius * 2 + 1)) as u8,
            };
        }
    }
    output
}

/// 分离模糊按预乘透明度求平均，玻璃模式再按边缘折射采样。
pub fn process_backdrop(
    source: &SharedPixelBuffer<Rgba8Pixel>,
    radius: i32,
    glass: bool,
) -> SharedPixelBuffer<Rgba8Pixel> {
    if source.width() == 0 || source.height() == 0 {
        return source.clone();
    }
    let radius = radius.clamp(0, 32);
    let mut output = source.clone();
    if radius > 0 {
        for _ in 0..2 {
            output = blur_pass(&blur_pass(&output, radius, true), radius, false);
        }
    }
    if glass {
        let before = output.clone();
        let width = output.width();
        let height = output.height();
        for y in 0..height {
            for x in 0..width {
                let nx = (2.0 * f64::from(x) + 1.0) / f64::from(width) - 1.0;
                let ny = (2.0 * f64::from(y) + 1.0) / f64::from(height) - 1.0;
                let scale = 1.0 + 0.12 * nx.abs().max(ny.abs()).powi(4);
                let sx = (((nx / scale + 1.0) * f64::from(width) - 1.0) / 2.0)
                    .round()
                    .clamp(0.0, f64::from(width - 1)) as u32;
                let sy = (((ny / scale + 1.0) * f64::from(height) - 1.0) / 2.0)
                    .round()
                    .clamp(0.0, f64::from(height - 1)) as u32;
                output.make_mut_slice()[(y * width + x) as usize] =
                    before.as_slice()[(sy * width + sx) as usize];
            }
        }
    }
    output
}

#[macro_export]
macro_rules! bind_widget_services {
    ($ui:expr, $service:ty, $qr:ty, $line:ty) => {{
        use slint::ComponentHandle as _;
        let ui = &$ui;
        let service = ui.global::<$service>();
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
