// 由 tools/generate_plot_ffi.py 生成；修改共用头文件后重新生成。
#![allow(non_snake_case)]

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ForgePlotPointCore {
    pub x: f64,
    pub y: f64,
    pub low: f64,
    pub high: f64,
    pub open: f64,
    pub close: f64,
    pub q1: f64,
    pub q3: f64,
    pub error_x_minus: f64,
    pub error_x_plus: f64,
    pub error_y_minus: f64,
    pub error_y_plus: f64,
    pub label: *const std::ffi::c_char,
    pub outliers: *const f64,
    pub outlier_count: usize,
}
impl Default for ForgePlotPointCore {
    fn default() -> Self {
        // 契约只包含数字和裸指针，零位模式均有效。
        unsafe { std::mem::zeroed() }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ForgePlotTickCore {
    pub value: f64,
    pub label: *const std::ffi::c_char,
}
impl Default for ForgePlotTickCore {
    fn default() -> Self {
        // 契约只包含数字和裸指针，零位模式均有效。
        unsafe { std::mem::zeroed() }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ForgePlotAxisCore {
    pub id: i32,
    pub panel: i32,
    pub side: i32,
    pub scale: i32,
    pub ticker: i32,
    pub auto_range: i32,
    pub reversed: i32,
    pub visible: i32,
    pub grid: i32,
    pub precision: i32,
    pub minimum: f64,
    pub maximum: f64,
    pub tick_step: f64,
    pub offset: f64,
    pub time_origin: f64,
    pub label: *const std::ffi::c_char,
    pub format: *const std::ffi::c_char,
    pub ticks: *const ForgePlotTickCore,
    pub tick_count: usize,
}
impl Default for ForgePlotAxisCore {
    fn default() -> Self {
        // 契约只包含数字和裸指针，零位模式均有效。
        unsafe { std::mem::zeroed() }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ForgePlotPanelCore {
    pub id: i32,
    pub x: f64,
    pub y: f64,
    pub width: f64,
    pub height: f64,
    pub title: *const std::ffi::c_char,
}
impl Default for ForgePlotPanelCore {
    fn default() -> Self {
        // 契约只包含数字和裸指针，零位模式均有效。
        unsafe { std::mem::zeroed() }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ForgePlotSeriesCore {
    pub id: i32,
    pub kind: i32,
    pub x_axis: i32,
    pub y_axis: i32,
    pub line_style: i32,
    pub scatter_style: i32,
    pub hidden: i32,
    pub selectable: i32,
    pub stack_group: i32,
    pub fill_target: i32,
    pub layer: i32,
    pub sorted: i32,
    pub color: u32,
    pub fill_color: u32,
    pub line_width: f64,
    pub point_size: f64,
    pub bar_width: f64,
    pub baseline: f64,
    pub name: *const std::ffi::c_char,
}
impl Default for ForgePlotSeriesCore {
    fn default() -> Self {
        // 契约只包含数字和裸指针，零位模式均有效。
        unsafe { std::mem::zeroed() }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ForgePlotMapCore {
    pub id: i32,
    pub panel: i32,
    pub x_axis: i32,
    pub y_axis: i32,
    pub columns: i32,
    pub rows: i32,
    pub palette: i32,
    pub auto_range: i32,
    pub logarithmic: i32,
    pub hidden: i32,
    pub interpolate: i32,
    pub color_scale: i32,
    pub layer: i32,
    pub x_min: f64,
    pub x_max: f64,
    pub y_min: f64,
    pub y_max: f64,
    pub z_min: f64,
    pub z_max: f64,
    pub name: *const std::ffi::c_char,
}
impl Default for ForgePlotMapCore {
    fn default() -> Self {
        // 契约只包含数字和裸指针，零位模式均有效。
        unsafe { std::mem::zeroed() }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ForgePlotItemCore {
    pub id: i32,
    pub kind: i32,
    pub panel: i32,
    pub x_axis: i32,
    pub y_axis: i32,
    pub coordinates: i32,
    pub anchor: i32,
    pub anchor_part: i32,
    pub clip: i32,
    pub layer: i32,
    pub selectable: i32,
    pub x: f64,
    pub y: f64,
    pub x2: f64,
    pub y2: f64,
    pub x3: f64,
    pub y3: f64,
    pub x4: f64,
    pub y4: f64,
    pub width: f64,
    pub size: f64,
    pub color: u32,
    pub fill_color: u32,
    pub text: *const std::ffi::c_char,
    pub image: *const std::ffi::c_char,
    pub image_pixels: *const u8,
    pub image_byte_count: usize,
    pub image_width: i32,
    pub image_height: i32,
}
impl Default for ForgePlotItemCore {
    fn default() -> Self {
        // 契约只包含数字和裸指针，零位模式均有效。
        unsafe { std::mem::zeroed() }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ForgePlotOptionsCore {
    pub width: f64,
    pub height: f64,
    pub font_size: f64,
    pub background: u32,
    pub foreground: u32,
    pub grid_color: u32,
    pub selection_color: u32,
    pub legend: i32,
    pub adaptive: i32,
    pub antialias: i32,
    pub max_points_per_pixel: i32,
    pub legend_position: i32,
    pub legend_style: i32,
    pub legend_latest_value: i32,
    pub legend_precision: i32,
    pub legend_placement: i32,
    pub measurement_enabled: i32,
    pub measurement_position: i32,
    pub measurement_placement: i32,
    pub measurement_series: i32,
    pub measurement_width: f64,
    pub measurement_height: f64,
    pub x_window_mode: i32,
    pub x_window_axis: i32,
    pub x_window_size: f64,
    pub title: *const std::ffi::c_char,
    pub font_family: *const std::ffi::c_char,
}
impl Default for ForgePlotOptionsCore {
    fn default() -> Self {
        // 契约只包含数字和裸指针，零位模式均有效。
        unsafe { std::mem::zeroed() }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ForgePlotRangeCore {
    pub id: i32,
    pub scale: i32,
    pub reversed: i32,
    pub minimum: f64,
    pub maximum: f64,
}
impl Default for ForgePlotRangeCore {
    fn default() -> Self {
        // 契约只包含数字和裸指针，零位模式均有效。
        unsafe { std::mem::zeroed() }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ForgePlotRectCore {
    pub id: i32,
    pub x: f64,
    pub y: f64,
    pub width: f64,
    pub height: f64,
}
impl Default for ForgePlotRectCore {
    fn default() -> Self {
        // 契约只包含数字和裸指针，零位模式均有效。
        unsafe { std::mem::zeroed() }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ForgePlotSelectionCore {
    pub id: i32,
    pub first: i32,
    pub last: i32,
}
impl Default for ForgePlotSelectionCore {
    fn default() -> Self {
        // 契约只包含数字和裸指针，零位模式均有效。
        unsafe { std::mem::zeroed() }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ForgePlotInfoCore {
    pub status: i32,
    pub input_points: u64,
    pub rendered_points: u64,
    pub path_count: u64,
    pub svg_bytes: u64,
    pub error: *const std::ffi::c_char,
}
impl Default for ForgePlotInfoCore {
    fn default() -> Self {
        // 契约只包含数字和裸指针，零位模式均有效。
        unsafe { std::mem::zeroed() }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ForgePlotHitCore {
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
    pub label: *const std::ffi::c_char,
}
impl Default for ForgePlotHitCore {
    fn default() -> Self {
        // 契约只包含数字和裸指针，零位模式均有效。
        unsafe { std::mem::zeroed() }
    }
}
