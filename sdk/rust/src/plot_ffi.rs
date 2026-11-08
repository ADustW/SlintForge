// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

//! 专业绘图公开接口的 DLL 转发；数据布局与共用头文件一致。
pub use slintforge_runtime_api::plot::*;

pub unsafe extern "C" fn forge_plot_core_new(
    options: *const ForgePlotOptionsCore,
) -> *mut std::ffi::c_void {
    (crate::runtime::api().forge_plot_core_new)(options)
}

pub unsafe extern "C" fn forge_plot_core_free(plot: *mut std::ffi::c_void) {
    (crate::runtime::api().forge_plot_core_free)(plot)
}

pub unsafe extern "C" fn forge_plot_core_begin_view(
    plot: *mut std::ffi::c_void,
    options: *const ForgePlotOptionsCore,
) {
    (crate::runtime::api().forge_plot_core_begin_view)(plot, options)
}

pub unsafe extern "C" fn forge_plot_core_axis(
    plot: *mut std::ffi::c_void,
    axis: *const ForgePlotAxisCore,
) {
    (crate::runtime::api().forge_plot_core_axis)(plot, axis)
}

pub unsafe extern "C" fn forge_plot_core_panel(
    plot: *mut std::ffi::c_void,
    panel: *const ForgePlotPanelCore,
) {
    (crate::runtime::api().forge_plot_core_panel)(plot, panel)
}

pub unsafe extern "C" fn forge_plot_core_series(
    plot: *mut std::ffi::c_void,
    series: *const ForgePlotSeriesCore,
    points: *const ForgePlotPointCore,
    count: usize,
) {
    (crate::runtime::api().forge_plot_core_series)(plot, series, points, count)
}

pub unsafe extern "C" fn forge_plot_core_append(
    plot: *mut std::ffi::c_void,
    series_id: i32,
    points: *const ForgePlotPointCore,
    count: usize,
) -> i32 {
    (crate::runtime::api().forge_plot_core_append)(plot, series_id, points, count)
}

pub unsafe extern "C" fn forge_plot_core_update_series(
    plot: *mut std::ffi::c_void,
    series: *const ForgePlotSeriesCore,
) -> i32 {
    (crate::runtime::api().forge_plot_core_update_series)(plot, series)
}

pub unsafe extern "C" fn forge_plot_core_map(
    plot: *mut std::ffi::c_void,
    map: *const ForgePlotMapCore,
    values: *const f64,
    count: usize,
) {
    (crate::runtime::api().forge_plot_core_map)(plot, map, values, count)
}

pub unsafe extern "C" fn forge_plot_core_item(
    plot: *mut std::ffi::c_void,
    item: *const ForgePlotItemCore,
) {
    (crate::runtime::api().forge_plot_core_item)(plot, item)
}

pub unsafe extern "C" fn forge_plot_core_selection(
    plot: *mut std::ffi::c_void,
    selection: *const ForgePlotSelectionCore,
    count: usize,
) {
    (crate::runtime::api().forge_plot_core_selection)(plot, selection, count)
}

pub unsafe extern "C" fn forge_plot_core_image_data(
    uri: *const std::ffi::c_char,
    data: *mut u8,
    capacity: usize,
) -> usize {
    (crate::runtime::api().forge_plot_core_image_data)(uri, data, capacity)
}

pub unsafe extern "C" fn forge_plot_core_render(
    plot: *mut std::ffi::c_void,
) -> *const std::ffi::c_char {
    (crate::runtime::api().forge_plot_core_render)(plot)
}

pub unsafe extern "C" fn forge_plot_core_info(plot: *const std::ffi::c_void) -> ForgePlotInfoCore {
    (crate::runtime::api().forge_plot_core_info)(plot)
}

pub unsafe extern "C" fn forge_plot_core_ranges(
    plot: *const std::ffi::c_void,
    ranges: *mut ForgePlotRangeCore,
    capacity: usize,
) -> usize {
    (crate::runtime::api().forge_plot_core_ranges)(plot, ranges, capacity)
}

pub unsafe extern "C" fn forge_plot_core_domains(
    plot: *const std::ffi::c_void,
    ranges: *mut ForgePlotRangeCore,
    capacity: usize,
) -> usize {
    (crate::runtime::api().forge_plot_core_domains)(plot, ranges, capacity)
}

pub unsafe extern "C" fn forge_plot_core_measure(
    plot: *const std::ffi::c_void,
    series_id: i32,
    x: f64,
) -> ForgePlotHitCore {
    (crate::runtime::api().forge_plot_core_measure)(plot, series_id, x)
}

pub unsafe extern "C" fn forge_plot_core_rects(
    plot: *const std::ffi::c_void,
    rects: *mut ForgePlotRectCore,
    capacity: usize,
) -> usize {
    (crate::runtime::api().forge_plot_core_rects)(plot, rects, capacity)
}

pub unsafe extern "C" fn forge_plot_core_annotation_rect(
    plot: *const std::ffi::c_void,
    kind: i32,
) -> ForgePlotRectCore {
    (crate::runtime::api().forge_plot_core_annotation_rect)(plot, kind)
}

pub unsafe extern "C" fn forge_plot_core_hit(
    plot: *const std::ffi::c_void,
    x: f64,
    y: f64,
    tolerance: f64,
) -> ForgePlotHitCore {
    (crate::runtime::api().forge_plot_core_hit)(plot, x, y, tolerance)
}

pub unsafe extern "C" fn forge_plot_core_select(
    plot: *const std::ffi::c_void,
    x: f64,
    y: f64,
    width: f64,
    height: f64,
    hits: *mut ForgePlotHitCore,
    capacity: usize,
) -> usize {
    (crate::runtime::api().forge_plot_core_select)(plot, x, y, width, height, hits, capacity)
}

pub unsafe extern "C" fn forge_plot_core_coordinate(
    plot: *const std::ffi::c_void,
    axis: i32,
    pixel: f64,
    inverse: i32,
) -> f64 {
    (crate::runtime::api().forge_plot_core_coordinate)(plot, axis, pixel, inverse)
}

pub unsafe extern "C" fn forge_plot_core_view(
    plot: *const std::ffi::c_void,
    baseline: *const ForgePlotRangeCore,
    mode: i32,
    position: f64,
    amount: f64,
    endpoint: f64,
) -> ForgePlotRangeCore {
    (crate::runtime::api().forge_plot_core_view)(plot, baseline, mode, position, amount, endpoint)
}

pub unsafe extern "C" fn forge_plot_core_export(
    plot: *const std::ffi::c_void,
    path: *const std::ffi::c_char,
    format: i32,
) -> i32 {
    (crate::runtime::api().forge_plot_core_export)(plot, path, format)
}
