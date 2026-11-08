// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

//! 截图坐标采用窗口逻辑像素，像素数据采用不预乘的 RGBA8。
use crate::pixels::{Rgba8Pixel, SharedPixelBuffer};
use std::path::Path;

#[derive(Clone, Copy, Debug)]
pub struct CaptureRect {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

#[derive(Debug)]
pub struct CaptureError(pub String);

impl std::fmt::Display for CaptureError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(&self.0)
    }
}
impl std::error::Error for CaptureError {}

/// 必须在所属 UI 线程、窗口显示并完成布局后调用。

/// 获取当前窗口画面中的矩形区域，保留滚动位置及画面覆盖层。

/// 对已有截图裁剪，也用于不依赖显示器 DPI 的坐标换算测试。
pub fn crop_capture(
    pixels: &SharedPixelBuffer<Rgba8Pixel>,
    rect: CaptureRect,
    scale: f32,
) -> Result<SharedPixelBuffer<Rgba8Pixel>, CaptureError> {
    if ![rect.x, rect.y, rect.width, rect.height, scale]
        .iter()
        .all(|v| v.is_finite())
        || rect.width <= 0.0
        || rect.height <= 0.0
        || scale <= 0.0
    {
        return Err(CaptureError("截图区域或缩放系数无效".into()));
    }
    let scale = f64::from(scale);
    let left = (f64::from(rect.x) * scale)
        .floor()
        .clamp(0.0, f64::from(pixels.width())) as u32;
    let top = (f64::from(rect.y) * scale)
        .floor()
        .clamp(0.0, f64::from(pixels.height())) as u32;
    let right = ((f64::from(rect.x) + f64::from(rect.width)) * scale)
        .ceil()
        .clamp(0.0, f64::from(pixels.width())) as u32;
    let bottom = ((f64::from(rect.y) + f64::from(rect.height)) * scale)
        .ceil()
        .clamp(0.0, f64::from(pixels.height())) as u32;
    if right <= left || bottom <= top {
        return Err(CaptureError("截图区域与窗口没有交集".into()));
    }
    let mut cropped = SharedPixelBuffer::new(right - left, bottom - top);
    let source = pixels.as_slice();
    for (row, target) in cropped
        .make_mut_slice()
        .chunks_mut((right - left) as usize)
        .enumerate()
    {
        let start = (top as usize + row) * pixels.width() as usize + left as usize;
        target.copy_from_slice(&source[start..start + target.len()]);
    }
    Ok(cropped)
}

/// 调用方决定输出路径；不自动创建父目录，保留明确的保存错误。
pub fn save_capture_png(
    pixels: &SharedPixelBuffer<Rgba8Pixel>,
    path: impl AsRef<Path>,
) -> Result<(), CaptureError> {
    if pixels.width() == 0 || pixels.height() == 0 {
        return Err(CaptureError("不能保存空截图".into()));
    }
    let file = std::fs::File::create(path.as_ref())
        .map_err(|e| CaptureError(format!("PNG 保存失败: {e}")))?;
    let mut encoder = png::Encoder::new(
        std::io::BufWriter::new(file),
        pixels.width(),
        pixels.height(),
    );
    encoder.set_color(png::ColorType::Rgba);
    encoder.set_depth(png::BitDepth::Eight);
    let mut writer = encoder
        .write_header()
        .map_err(|e| CaptureError(format!("PNG 编码失败: {e}")))?;
    writer
        .write_image_data(pixels.as_bytes())
        .map_err(|e| CaptureError(format!("PNG 编码失败: {e}")))?;
    writer
        .finish()
        .map_err(|e| CaptureError(format!("PNG 保存失败: {e}")))
}
