// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

//! 共用绘图引擎的生命周期、空间查询和导出。Slint 绑定由生成的宏完成。
#[path = "plot_ffi.rs"]
pub mod raw;
use std::collections::HashMap;
use std::ffi::{CStr, CString};
use std::ptr::NonNull;

pub fn plot_cstring(value: &str) -> CString {
    CString::new(value.replace('\0', "")).expect("已移除内嵌零字节")
}
pub fn plot_color(value: slint::Color) -> u32 {
    ((value.alpha() as u32) << 24)
        | ((value.red() as u32) << 16)
        | ((value.green() as u32) << 8)
        | value.blue() as u32
}
// 原始指针只在引擎对象存活期间读取，返回字符串立即复制。
fn plot_string(value: *const std::ffi::c_char) -> String {
    if value.is_null() {
        String::new()
    } else {
        unsafe { CStr::from_ptr(value).to_string_lossy().into_owned() }
    }
}

#[derive(Clone, Debug, Default)]
pub struct PlotStats {
    pub status: i32,
    pub input_points: u64,
    pub rendered_points: u64,
    pub path_count: u64,
    pub svg_bytes: u64,
    pub error: String,
}
#[derive(Clone, Debug, Default)]
pub struct PlotHit {
    pub kind: i32,
    pub id: i32,
    pub index: i32,
    pub panel: i32,
    pub x: f64,
    pub y: f64,
    pub value: f64,
    pub pixel_x: f64,
    pub pixel_y: f64,
    pub distance: f64,
    pub label: String,
}
impl From<raw::ForgePlotHitCore> for PlotHit {
    fn from(v: raw::ForgePlotHitCore) -> Self {
        Self {
            kind: v.kind,
            id: v.id,
            index: v.index,
            panel: v.panel,
            x: v.x,
            y: v.y,
            value: v.value,
            pixel_x: v.pixel_x,
            pixel_y: v.pixel_y,
            distance: v.distance,
            label: plot_string(v.label),
        }
    }
}

pub struct Plot {
    handle: NonNull<std::ffi::c_void>,
    svg: String,
    display: std::cell::RefCell<Option<(u32, slint::Image)>>,
}
impl Drop for Plot {
    fn drop(&mut self) {
        unsafe { raw::forge_plot_core_free(self.handle.as_ptr()) }
    }
}
impl Plot {
    /// # Safety
    /// 契约中的字符串须有效且以零字节结束；引擎在调用期间复制。
    pub unsafe fn from_core(options: &raw::ForgePlotOptionsCore) -> Result<Self, String> {
        crate::runtime::initialize().map_err(|e| e.message())?;
        NonNull::new(raw::forge_plot_core_new(options))
            .map(|handle| Self {
                handle,
                svg: String::new(),
                display: Default::default(),
            })
            .ok_or_else(|| "绘图引擎分配失败".into())
    }
    /// # Safety
    /// 轴契约中的字符串和刻度数组须在本次调用期间有效。
    pub unsafe fn add_axis_core(&mut self, axis: &raw::ForgePlotAxisCore) {
        raw::forge_plot_core_axis(self.handle.as_ptr(), axis);
    }
    /// # Safety
    /// 字符串须在本次调用期间有效；保留原始数据，随后需重新提供坐标轴。
    pub unsafe fn begin_view_core(&mut self, options: &raw::ForgePlotOptionsCore) {
        raw::forge_plot_core_begin_view(self.handle.as_ptr(), options);
    }
    /// # Safety
    /// 标题字符串须在本次调用期间有效。
    pub unsafe fn add_panel_core(&mut self, panel: &raw::ForgePlotPanelCore) {
        raw::forge_plot_core_panel(self.handle.as_ptr(), panel);
    }
    /// # Safety
    /// 点的标签和离群数组须在本次调用期间有效。
    pub unsafe fn add_series_core(
        &mut self,
        series: &raw::ForgePlotSeriesCore,
        points: &[raw::ForgePlotPointCore],
    ) {
        raw::forge_plot_core_series(self.handle.as_ptr(), series, points.as_ptr(), points.len());
    }
    /// # Safety
    /// 点的标签和离群数组须在本次调用期间有效；历史由引擎保留。
    pub unsafe fn append_core(&mut self, series: i32, points: &[raw::ForgePlotPointCore]) -> bool {
        raw::forge_plot_core_append(self.handle.as_ptr(), series, points.as_ptr(), points.len())
            != 0
    }
    /// 追加一批双精度 XY 数据，不复制既有历史；同一曲线的 X 建议保持递增。
    pub fn append_xy(&mut self, series: i32, points: &[(f64, f64)]) -> bool {
        let points = points
            .iter()
            .map(|&(x, y)| raw::ForgePlotPointCore {
                x,
                y,
                ..Default::default()
            })
            .collect::<Vec<_>>();
        unsafe { self.append_core(series, &points) }
    }
    /// # Safety
    /// 系列名称须在调用期间有效；改变外观时保留追加数据。
    pub unsafe fn update_series_core(&mut self, series: &raw::ForgePlotSeriesCore) -> bool {
        raw::forge_plot_core_update_series(self.handle.as_ptr(), series) != 0
    }
    pub fn domains(&self) -> Vec<raw::ForgePlotRangeCore> {
        let mut out = vec![
            raw::ForgePlotRangeCore::default();
            unsafe {
                raw::forge_plot_core_domains(self.handle.as_ptr(), std::ptr::null_mut(), 0)
            }
        ];
        unsafe {
            raw::forge_plot_core_domains(self.handle.as_ptr(), out.as_mut_ptr(), out.len());
        }
        out
    }
    /// 光标读取最邻近的原始采样点；返回索引不受显示抽样影响。
    pub fn measure(&self, series: i32, x: f64) -> PlotHit {
        unsafe { raw::forge_plot_core_measure(self.handle.as_ptr(), series, x).into() }
    }
    /// # Safety
    /// 名称字符串须在本次调用期间有效。
    pub unsafe fn add_map_core(&mut self, map: &raw::ForgePlotMapCore, values: &[f64]) {
        raw::forge_plot_core_map(self.handle.as_ptr(), map, values.as_ptr(), values.len());
    }
    /// # Safety
    /// 文字和图片 URI 须在本次调用期间有效。
    pub unsafe fn add_item_core(&mut self, item: &raw::ForgePlotItemCore) {
        // 宿主先解码，向共用引擎传递 RGBA，使图片图元不依赖平台解码器。
        if item.kind == 8 && item.image_pixels.is_null() && !item.image.is_null() {
            let count = raw::forge_plot_core_image_data(item.image, std::ptr::null_mut(), 0);
            let mut bytes = vec![0u8; count];
            if count > 0 {
                raw::forge_plot_core_image_data(item.image, bytes.as_mut_ptr(), count);
                if let Ok(pixels) = crate::decode_image_pixels(&bytes) {
                    let mut core = *item;
                    core.image_width = pixels.width() as i32;
                    core.image_height = pixels.height() as i32;
                    core.image_byte_count = pixels.as_bytes().len();
                    core.image_pixels = pixels.as_bytes().as_ptr();
                    raw::forge_plot_core_item(self.handle.as_ptr(), &core);
                    return;
                }
            }
        }
        raw::forge_plot_core_item(self.handle.as_ptr(), item);
    }
    pub fn graph(points: &[(f64, f64)], width: f64, height: f64) -> Result<Self, String> {
        let font = plot_cstring("Segoe UI Variable");
        let options = raw::ForgePlotOptionsCore {
            width,
            height,
            font_size: 12.,
            background: 0xffffffff,
            foreground: 0xff202020,
            grid_color: 0xffdddddd,
            adaptive: 1,
            antialias: 1,
            font_family: font.as_ptr(),
            ..Default::default()
        };
        let mut result = unsafe { Self::from_core(&options)? };
        let input = points
            .iter()
            .map(|&(x, y)| raw::ForgePlotPointCore {
                x,
                y,
                ..Default::default()
            })
            .collect::<Vec<_>>();
        let series = raw::ForgePlotSeriesCore {
            y_axis: 1,
            line_style: 1,
            selectable: 1,
            stack_group: -1,
            fill_target: -1,
            line_width: 2.,
            ..Default::default()
        };
        unsafe {
            result.add_series_core(&series, &input);
        }
        result.render()?;
        Ok(result)
    }
    pub fn render(&mut self) -> Result<&str, String> {
        let svg = plot_string(unsafe { raw::forge_plot_core_render(self.handle.as_ptr()) });
        if self.svg != svg {
            self.display.get_mut().take();
            self.svg = svg;
        }
        let stats = self.stats();
        if stats.status == 2 {
            Err(stats.error)
        } else {
            Ok(&self.svg)
        }
    }
    pub fn svg(&self) -> &str {
        &self.svg
    }
    pub fn stats(&self) -> PlotStats {
        let s = unsafe { raw::forge_plot_core_info(self.handle.as_ptr()) };
        PlotStats {
            status: s.status,
            input_points: s.input_points,
            rendered_points: s.rendered_points,
            path_count: s.path_count,
            svg_bytes: s.svg_bytes,
            error: plot_string(s.error),
        }
    }
    pub fn image(&self) -> Result<slint::Image, String> {
        slint::Image::load_from_svg_data(self.svg.as_bytes())
            .map_err(|e| format!("SVG 加载失败：{e}"))
    }
    /// 仅在场景或 DPI 变化后栅格化一次，避免悬停覆盖层触发整幅 SVG 重绘。
    /// SVG/PDF 和其他导出仍使用原始逻辑尺寸及矢量内容。
    pub fn display_image(&self, scale: f32) -> Result<slint::Image, String> {
        let scale = if scale.is_finite() && scale > 0. {
            scale
        } else {
            1.
        };
        if let Some((cached_scale, image)) = &*self.display.borrow() {
            if *cached_scale == scale.to_bits() {
                return Ok(image.clone());
            }
        }
        let mut svg = self.svg.clone();
        for attribute in [" width=\"", " height=\""] {
            let start = svg.find(attribute).ok_or("SVG 根尺寸缺失")? + attribute.len();
            let end = start + svg[start..].find('"').ok_or("SVG 根尺寸无效")?;
            let logical = svg[start..end].parse::<f64>().map_err(|e| e.to_string())?;
            svg.replace_range(
                start..end,
                &((logical * scale as f64).ceil().max(1.) as u32).to_string(),
            );
        }
        let vector = slint::Image::load_from_svg_data(svg.as_bytes())
            .map_err(|e| format!("SVG 加载失败：{e}"))?;
        let pixels = vector.to_rgba8_premultiplied().ok_or("图表栅格化失败")?;
        let image = slint::Image::from_rgba8_premultiplied(pixels);
        *self.display.borrow_mut() = Some((scale.to_bits(), image.clone()));
        Ok(image)
    }
    pub fn ranges(&self) -> Vec<raw::ForgePlotRangeCore> {
        let count =
            unsafe { raw::forge_plot_core_ranges(self.handle.as_ptr(), std::ptr::null_mut(), 0) };
        let mut result = vec![raw::ForgePlotRangeCore::default(); count];
        unsafe {
            raw::forge_plot_core_ranges(self.handle.as_ptr(), result.as_mut_ptr(), result.len());
        }
        result
    }
    pub fn annotation_rect(&self, kind: i32) -> raw::ForgePlotRectCore {
        unsafe { raw::forge_plot_core_annotation_rect(self.handle.as_ptr(), kind) }
    }
    pub fn rects(&self) -> Vec<raw::ForgePlotRectCore> {
        let count =
            unsafe { raw::forge_plot_core_rects(self.handle.as_ptr(), std::ptr::null_mut(), 0) };
        let mut result = vec![raw::ForgePlotRectCore::default(); count];
        unsafe {
            raw::forge_plot_core_rects(self.handle.as_ptr(), result.as_mut_ptr(), result.len());
        }
        result
    }
    pub fn coordinate(&self, axis: i32, pixel: f64, inverse: bool) -> f64 {
        unsafe {
            raw::forge_plot_core_coordinate(self.handle.as_ptr(), axis, pixel, inverse as i32)
        }
    }
    pub fn set_selection(&mut self, selection: &[raw::ForgePlotSelectionCore]) {
        unsafe {
            raw::forge_plot_core_selection(
                self.handle.as_ptr(),
                selection.as_ptr(),
                selection.len(),
            );
        }
    }
    pub fn view(
        &self,
        range: &raw::ForgePlotRangeCore,
        mode: i32,
        position: f32,
        amount: f32,
    ) -> raw::ForgePlotRangeCore {
        unsafe {
            raw::forge_plot_core_view(
                self.handle.as_ptr(),
                range,
                mode,
                position as f64,
                amount as f64,
                amount as f64,
            )
        }
    }
    pub fn hit(&self, x: f64, y: f64, tolerance: f64) -> PlotHit {
        unsafe { raw::forge_plot_core_hit(self.handle.as_ptr(), x, y, tolerance) }.into()
    }
    pub fn select_rect(&self, x: f64, y: f64, width: f64, height: f64) -> Vec<(i32, i32, i32)> {
        let count = unsafe {
            raw::forge_plot_core_select(
                self.handle.as_ptr(),
                x,
                y,
                width,
                height,
                std::ptr::null_mut(),
                0,
            )
        };
        let mut hits = vec![raw::ForgePlotHitCore::default(); count];
        unsafe {
            raw::forge_plot_core_select(
                self.handle.as_ptr(),
                x,
                y,
                width,
                height,
                hits.as_mut_ptr(),
                count,
            );
        }
        let mut result: Vec<(i32, i32, i32)> = Vec::new();
        for hit in hits {
            if let Some(last) = result.last_mut() {
                if last.0 == hit.id && last.2 + 1 == hit.index {
                    last.2 = hit.index;
                    continue;
                }
            }
            result.push((hit.id, hit.index, hit.index));
        }
        result
    }
    pub fn export(&self, path: &std::path::Path, format: &str) -> Result<(), String> {
        match format.to_lowercase().as_str() {
            "svg" => std::fs::write(path, &self.svg).map_err(|e| e.to_string()),
            "pdf" => {
                let path = plot_cstring(&path.to_string_lossy());
                if unsafe { raw::forge_plot_core_export(self.handle.as_ptr(), path.as_ptr(), 1) }
                    == 1
                {
                    Ok(())
                } else {
                    Err("PDF 写入失败".into())
                }
            }
            "png" => {
                let image = self.image()?;
                let pixels = image.to_rgba8().ok_or("SVG 无法读取像素")?;
                crate::save_capture_png(&pixels, path).map_err(|e| e.to_string())
            }
            "jpg" | "jpeg" | "bmp" => {
                let image = self.image()?;
                let pixels = image.to_rgba8().ok_or("SVG 无法读取像素")?;
                crate::save_image_pixels(
                    &pixels,
                    path,
                    if format.eq_ignore_ascii_case("bmp") {
                        2
                    } else {
                        1
                    },
                )
            }
            _ => Err("支持 PNG、JPEG、BMP、SVG、PDF".into()),
        }
    }
}

#[derive(Default)]
pub struct PlotServiceState {
    next: i32,
    plots: HashMap<i32, Plot>,
}
impl PlotServiceState {
    pub fn allocate(&mut self) -> i32 {
        self.next = self.next.checked_add(1).expect("图表实例计数溢出");
        self.next
    }
    pub fn store(&mut self, id: i32, plot: Plot) {
        self.plots.insert(id, plot);
    }
    pub fn release(&mut self, id: i32) {
        self.plots.remove(&id);
    }
    pub fn get(&self, id: i32) -> Option<&Plot> {
        self.plots.get(&id)
    }
    pub fn get_mut(&mut self, id: i32) -> Option<&mut Plot> {
        self.plots.get_mut(&id)
    }
    pub fn take(&mut self, id: i32) -> Option<Plot> {
        self.plots.remove(&id)
    }
}
// 闭区间选择，合并后保持顺序；mode=0 替换，1 合并，2 对称差。
pub fn merge_plot_selection(
    current: &[(i32, i32, i32)],
    next: &[(i32, i32, i32)],
    mode: i32,
) -> Vec<(i32, i32, i32)> {
    use slintforge_runtime_api::{models::*, Arena, Wire};
    let mut arena = Arena::default();
    let current = current.to_vec();
    let incoming = next.to_vec();
    let args = ArgsMergeSelection {
        current: current.view(&mut arena),
        incoming: incoming.view(&mut arena),
        mode,
    };
    crate::runtime::value_or::<Vec<(i32, i32, i32)>, _>(OP_MERGESELECTION, &args, || current.clone())
}

// 有界实时缓冲：追加 O(1)，旧点按容量淘汰；批量同步到 UI 由应用节流。
pub struct PlotBuffer {
    capacity: usize,
    points: std::collections::VecDeque<(f64, f64)>,
}
impl PlotBuffer {
    pub fn new(capacity: usize) -> Self {
        Self {
            capacity: capacity.max(1),
            points: std::collections::VecDeque::new(),
        }
    }
    pub fn push(&mut self, x: f64, y: f64) {
        if self.points.len() == self.capacity {
            self.points.pop_front();
        }
        self.points.push_back((x, y));
    }
    pub fn extend(&mut self, points: impl IntoIterator<Item = (f64, f64)>) {
        for (x, y) in points {
            self.push(x, y);
        }
    }
    pub fn snapshot(&self) -> Vec<(f64, f64)> {
        self.points.iter().copied().collect()
    }
    pub fn len(&self) -> usize {
        self.points.len()
    }
    pub fn is_empty(&self) -> bool {
        self.points.is_empty()
    }
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn selection_intervals_merge_and_toggle() {
        assert_eq!(
            merge_plot_selection(&[(0, 0, 4)], &[(0, 2, 3)], 2),
            vec![(0, 0, 1), (0, 4, 4)]
        );
        assert_eq!(
            merge_plot_selection(&[(0, 0, 4)], &[(0, 4, 8), (1, 1, 2)], 1),
            vec![(0, 0, 8), (1, 1, 2)]
        );
        assert_eq!(
            merge_plot_selection(&[(0, 0, 4), (0, 2, 8)], &[(0, 3, 5)], 2),
            vec![(0, 0, 2), (0, 6, 8)]
        );
    }
    #[test]
    fn native_engine_maps_and_keeps_peaks() {
        let points = (0..100000)
            .map(|i| {
                (
                    i as f64,
                    if i == 33333 {
                        1000.
                    } else {
                        (i as f64 * 0.01).sin()
                    },
                )
            })
            .collect::<Vec<_>>();
        let plot = Plot::graph(&points, 900., 540.).unwrap();
        assert!(plot.stats().rendered_points < 5000);
        let x = plot.coordinate(0, 33333., false);
        let y = plot.coordinate(1, 1000., false);
        assert_eq!(plot.hit(x, y, 2.).index, 33333);
        assert!((plot.coordinate(0, x, true) - 33333.).abs() < 1e-7);
        let mut buffer = PlotBuffer::new(2);
        buffer.extend([(1., 1.), (2., 2.), (3., 3.)]);
        assert_eq!(buffer.snapshot(), vec![(2., 2.), (3., 3.)]);
    }
}
