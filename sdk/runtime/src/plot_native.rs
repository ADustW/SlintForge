pub use slintforge_runtime_api::plot::*;
extern "C" {
    pub fn forge_plot_core_new(options: *const ForgePlotOptionsCore) -> *mut std::ffi::c_void;
    pub fn forge_plot_core_free(plot: *mut std::ffi::c_void);
    pub fn forge_plot_core_begin_view(
        plot: *mut std::ffi::c_void,
        options: *const ForgePlotOptionsCore,
    );
    pub fn forge_plot_core_axis(plot: *mut std::ffi::c_void, axis: *const ForgePlotAxisCore);
    pub fn forge_plot_core_panel(plot: *mut std::ffi::c_void, panel: *const ForgePlotPanelCore);
    pub fn forge_plot_core_series(
        plot: *mut std::ffi::c_void,
        series: *const ForgePlotSeriesCore,
        points: *const ForgePlotPointCore,
        count: usize,
    );
    pub fn forge_plot_core_append(
        plot: *mut std::ffi::c_void,
        series_id: i32,
        points: *const ForgePlotPointCore,
        count: usize,
    ) -> i32;
    pub fn forge_plot_core_update_series(
        plot: *mut std::ffi::c_void,
        series: *const ForgePlotSeriesCore,
    ) -> i32;
    pub fn forge_plot_core_map(
        plot: *mut std::ffi::c_void,
        map: *const ForgePlotMapCore,
        values: *const f64,
        count: usize,
    );
    pub fn forge_plot_core_item(plot: *mut std::ffi::c_void, item: *const ForgePlotItemCore);
    pub fn forge_plot_core_selection(
        plot: *mut std::ffi::c_void,
        selection: *const ForgePlotSelectionCore,
        count: usize,
    );
    pub fn forge_plot_core_image_data(
        uri: *const std::ffi::c_char,
        data: *mut u8,
        capacity: usize,
    ) -> usize;
    pub fn forge_plot_core_render(plot: *mut std::ffi::c_void) -> *const std::ffi::c_char;
    pub fn forge_plot_core_info(plot: *const std::ffi::c_void) -> ForgePlotInfoCore;
    pub fn forge_plot_core_ranges(
        plot: *const std::ffi::c_void,
        ranges: *mut ForgePlotRangeCore,
        capacity: usize,
    ) -> usize;
    pub fn forge_plot_core_domains(
        plot: *const std::ffi::c_void,
        ranges: *mut ForgePlotRangeCore,
        capacity: usize,
    ) -> usize;
    pub fn forge_plot_core_measure(
        plot: *const std::ffi::c_void,
        series_id: i32,
        x: f64,
    ) -> ForgePlotHitCore;
    pub fn forge_plot_core_rects(
        plot: *const std::ffi::c_void,
        rects: *mut ForgePlotRectCore,
        capacity: usize,
    ) -> usize;
    pub fn forge_plot_core_annotation_rect(
        plot: *const std::ffi::c_void,
        kind: i32,
    ) -> ForgePlotRectCore;
    pub fn forge_plot_core_hit(
        plot: *const std::ffi::c_void,
        x: f64,
        y: f64,
        tolerance: f64,
    ) -> ForgePlotHitCore;
    pub fn forge_plot_core_select(
        plot: *const std::ffi::c_void,
        x: f64,
        y: f64,
        width: f64,
        height: f64,
        hits: *mut ForgePlotHitCore,
        capacity: usize,
    ) -> usize;
    pub fn forge_plot_core_coordinate(
        plot: *const std::ffi::c_void,
        axis: i32,
        pixel: f64,
        inverse: i32,
    ) -> f64;
    pub fn forge_plot_core_view(
        plot: *const std::ffi::c_void,
        baseline: *const ForgePlotRangeCore,
        mode: i32,
        position: f64,
        amount: f64,
        endpoint: f64,
    ) -> ForgePlotRangeCore;
    pub fn forge_plot_core_export(
        plot: *const std::ffi::c_void,
        path: *const std::ffi::c_char,
        format: i32,
    ) -> i32;
}
