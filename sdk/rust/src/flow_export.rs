// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

use crate::flow::FlowError;
use std::path::Path;

/// SVG 由共用核心生成；PNG 不依赖当前画布的缩放、裁剪或选区覆盖层。
pub fn save_flow_image(svg: &str, path: impl AsRef<Path>) -> Result<(), FlowError> {
    let path=path.as_ref();
    let extension=path.extension().and_then(|s|s.to_str()).unwrap_or_default().to_ascii_lowercase();
    if extension!="svg" && extension!="png" {return Err(FlowError::Rejected("图像格式必须为 .svg 或 .png".into()));}
    if let Some(parent)=path.parent().filter(|p|!p.as_os_str().is_empty()) {std::fs::create_dir_all(parent).map_err(|e|FlowError::Rejected(e.to_string()))?;}
    if extension=="svg" {return std::fs::write(path,svg).map_err(|e|FlowError::Rejected(e.to_string()));}
    let image=slint::Image::load_from_svg_data(svg.as_bytes()).map_err(|e|FlowError::Rejected(format!("SVG 解码失败: {e}")))?;
    let size=image.size();
    if size.width==0 || size.height==0 || u64::from(size.width)*u64::from(size.height)>64000000 {return Err(FlowError::Rejected("SVG 尺寸无效或超过 6400 万像素".into()));}
    let pixels=image.to_rgba8().ok_or_else(||FlowError::Rejected("SVG 像素转换失败".into()))?;
    crate::save_capture_png(&pixels,path).map_err(|e|FlowError::Rejected(e.to_string()))
}
