#[repr(C)]
#[derive(Clone, Copy)]
pub struct RuntimeApiV1 {
    pub abi_version: u32,
    pub struct_size: u32,
    pub schema: [u8; 32],
    pub call: unsafe extern "C" fn(u32, *const std::ffi::c_void, *mut CallResult) -> i32,
    pub release: unsafe extern "C" fn(u64) -> i32,
    pub forge_plot_core_new:
        unsafe extern "C" fn(options: *const plot::ForgePlotOptionsCore) -> *mut std::ffi::c_void,
    pub forge_plot_core_free: unsafe extern "C" fn(plot: *mut std::ffi::c_void),
    pub forge_plot_core_begin_view: unsafe extern "C" fn(
        plot: *mut std::ffi::c_void,
        options: *const plot::ForgePlotOptionsCore,
    ),
    pub forge_plot_core_axis:
        unsafe extern "C" fn(plot: *mut std::ffi::c_void, axis: *const plot::ForgePlotAxisCore),
    pub forge_plot_core_panel:
        unsafe extern "C" fn(plot: *mut std::ffi::c_void, panel: *const plot::ForgePlotPanelCore),
    pub forge_plot_core_series: unsafe extern "C" fn(
        plot: *mut std::ffi::c_void,
        series: *const plot::ForgePlotSeriesCore,
        points: *const plot::ForgePlotPointCore,
        count: usize,
    ),
    pub forge_plot_core_append: unsafe extern "C" fn(
        plot: *mut std::ffi::c_void,
        series_id: i32,
        points: *const plot::ForgePlotPointCore,
        count: usize,
    ) -> i32,
    pub forge_plot_core_update_series: unsafe extern "C" fn(
        plot: *mut std::ffi::c_void,
        series: *const plot::ForgePlotSeriesCore,
    ) -> i32,
    pub forge_plot_core_map: unsafe extern "C" fn(
        plot: *mut std::ffi::c_void,
        map: *const plot::ForgePlotMapCore,
        values: *const f64,
        count: usize,
    ),
    pub forge_plot_core_item:
        unsafe extern "C" fn(plot: *mut std::ffi::c_void, item: *const plot::ForgePlotItemCore),
    pub forge_plot_core_selection: unsafe extern "C" fn(
        plot: *mut std::ffi::c_void,
        selection: *const plot::ForgePlotSelectionCore,
        count: usize,
    ),
    pub forge_plot_core_image_data:
        unsafe extern "C" fn(uri: *const std::ffi::c_char, data: *mut u8, capacity: usize) -> usize,
    pub forge_plot_core_render:
        unsafe extern "C" fn(plot: *mut std::ffi::c_void) -> *const std::ffi::c_char,
    pub forge_plot_core_info:
        unsafe extern "C" fn(plot: *const std::ffi::c_void) -> plot::ForgePlotInfoCore,
    pub forge_plot_core_ranges: unsafe extern "C" fn(
        plot: *const std::ffi::c_void,
        ranges: *mut plot::ForgePlotRangeCore,
        capacity: usize,
    ) -> usize,
    pub forge_plot_core_domains: unsafe extern "C" fn(
        plot: *const std::ffi::c_void,
        ranges: *mut plot::ForgePlotRangeCore,
        capacity: usize,
    ) -> usize,
    pub forge_plot_core_measure: unsafe extern "C" fn(
        plot: *const std::ffi::c_void,
        series_id: i32,
        x: f64,
    ) -> plot::ForgePlotHitCore,
    pub forge_plot_core_rects: unsafe extern "C" fn(
        plot: *const std::ffi::c_void,
        rects: *mut plot::ForgePlotRectCore,
        capacity: usize,
    ) -> usize,
    pub forge_plot_core_annotation_rect:
        unsafe extern "C" fn(plot: *const std::ffi::c_void, kind: i32) -> plot::ForgePlotRectCore,
    pub forge_plot_core_hit: unsafe extern "C" fn(
        plot: *const std::ffi::c_void,
        x: f64,
        y: f64,
        tolerance: f64,
    ) -> plot::ForgePlotHitCore,
    pub forge_plot_core_select: unsafe extern "C" fn(
        plot: *const std::ffi::c_void,
        x: f64,
        y: f64,
        width: f64,
        height: f64,
        hits: *mut plot::ForgePlotHitCore,
        capacity: usize,
    ) -> usize,
    pub forge_plot_core_coordinate: unsafe extern "C" fn(
        plot: *const std::ffi::c_void,
        axis: i32,
        pixel: f64,
        inverse: i32,
    ) -> f64,
    pub forge_plot_core_view: unsafe extern "C" fn(
        plot: *const std::ffi::c_void,
        baseline: *const plot::ForgePlotRangeCore,
        mode: i32,
        position: f64,
        amount: f64,
        endpoint: f64,
    ) -> plot::ForgePlotRangeCore,
    pub forge_plot_core_export: unsafe extern "C" fn(
        plot: *const std::ffi::c_void,
        path: *const std::ffi::c_char,
        format: i32,
    ) -> i32,
}

pub const ABI_SCHEMA: [u8; 32] = [
    28, 205, 219, 63, 216, 216, 51, 181, 130, 109, 153, 122, 186, 45, 54, 198, 115, 42, 155, 57,
    50, 139, 107, 215, 185, 0, 180, 46, 114, 42, 242, 123,
];
