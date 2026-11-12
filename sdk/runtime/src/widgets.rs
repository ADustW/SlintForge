// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

use crate::pixels::{Rgba8Pixel, SharedPixelBuffer};

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
