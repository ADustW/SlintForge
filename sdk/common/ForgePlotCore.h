// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <stddef.h>
#include <stdint.h>

// 两端共用的纯数据 ABI；输入字符串/数组仅需在调用期间有效。
#ifdef __cplusplus
extern "C" {
#endif

typedef struct ForgePlotPointCore {
    double x, y, low, high, open, close, q1, q3;
    double error_x_minus, error_x_plus, error_y_minus, error_y_plus;
    const char* label;
    const double* outliers;
    size_t outlier_count;
} ForgePlotPointCore;

typedef struct ForgePlotTickCore { double value; const char* label; } ForgePlotTickCore;
typedef struct ForgePlotAxisCore {
    int id, panel, side, scale, ticker, auto_range, reversed, visible, grid, precision;
    double minimum, maximum, tick_step, offset, time_origin;
    const char *label, *format;
    const ForgePlotTickCore* ticks;
    size_t tick_count;
} ForgePlotAxisCore;

typedef struct ForgePlotPanelCore {
    int id;
    double x, y, width, height;
    const char* title;
    const ForgePlotTickCore* categories;
    size_t category_count;
} ForgePlotPanelCore;

typedef struct ForgePlotPointStyleCore {
    int index, hidden;
    double size;
    uint32_t color;
} ForgePlotPointStyleCore;

typedef struct ForgePlotSeriesCore {
    int id, kind, x_axis, y_axis, line_style, scatter_style, hidden, selectable;
    int stack_group, fill_target, layer, sorted;
    uint32_t color, fill_color;
    double line_width, point_size, bar_width, baseline;
    const char* name;
    double inner_radius;
    const ForgePlotPointStyleCore* point_styles;
    size_t point_style_count;
} ForgePlotSeriesCore;

typedef struct ForgePlotMapCore {
    int id, panel, x_axis, y_axis, columns, rows, palette, auto_range, logarithmic;
    int hidden, interpolate, color_scale, layer;
    double x_min, x_max, y_min, y_max, z_min, z_max;
    const char* name;
} ForgePlotMapCore;

typedef struct ForgePlotItemCore {
    int id, kind, panel, x_axis, y_axis, coordinates, anchor, anchor_part, clip, layer, selectable;
    double x, y, x2, y2, x3, y3, x4, y4, width, size;
    uint32_t color, fill_color;
    const char *text, *image;
    const uint8_t* image_pixels;
    size_t image_byte_count;
    int image_width, image_height;
} ForgePlotItemCore;

typedef struct ForgePlotOptionsCore {
    double width, height, font_size;
    uint32_t background, foreground, grid_color, selection_color;
    int legend, adaptive, antialias, max_points_per_pixel;
    int legend_position, legend_style, legend_latest_value, legend_precision;
    int legend_placement;
    int measurement_enabled, measurement_position, measurement_placement, measurement_series;
    double measurement_width, measurement_height;
    int x_window_mode, x_window_axis;
    double x_window_size;
    const char *title, *font_family;
} ForgePlotOptionsCore;

typedef struct ForgePlotRangeCore { int id, scale, reversed; double minimum, maximum; } ForgePlotRangeCore;
typedef struct ForgePlotRectCore { int id; double x, y, width, height; } ForgePlotRectCore;
typedef struct ForgePlotSelectionCore { int id, first, last; } ForgePlotSelectionCore;
typedef struct ForgePlotInfoCore {
    int status;
    uint64_t input_points, rendered_points, path_count, svg_bytes;
    const char* error;
} ForgePlotInfoCore;
typedef struct ForgePlotHitCore {
    int kind, id, index, panel;
    double x, y, value, pixel_x, pixel_y, distance;
    const char* label;
} ForgePlotHitCore;

void* forge_plot_core_new(const ForgePlotOptionsCore* options);
void forge_plot_core_free(void* plot);
void forge_plot_core_begin_view(void* plot, const ForgePlotOptionsCore* options);
void forge_plot_core_axis(void* plot, const ForgePlotAxisCore* axis);
void forge_plot_core_panel(void* plot, const ForgePlotPanelCore* panel);
void forge_plot_core_series(void* plot, const ForgePlotSeriesCore* series, const ForgePlotPointCore* points, size_t count);
int forge_plot_core_append(void* plot, int series_id, const ForgePlotPointCore* points, size_t count);
int forge_plot_core_update_series(void* plot, const ForgePlotSeriesCore* series);
void forge_plot_core_map(void* plot, const ForgePlotMapCore* map, const double* values, size_t count);
void forge_plot_core_item(void* plot, const ForgePlotItemCore* item);
void forge_plot_core_selection(void* plot, const ForgePlotSelectionCore* selection, size_t count);
size_t forge_plot_core_image_data(const char* uri, uint8_t* data, size_t capacity);
const char* forge_plot_core_render(void* plot);
ForgePlotInfoCore forge_plot_core_info(const void* plot);
size_t forge_plot_core_ranges(const void* plot, ForgePlotRangeCore* ranges, size_t capacity);
size_t forge_plot_core_domains(const void* plot, ForgePlotRangeCore* ranges, size_t capacity);
ForgePlotHitCore forge_plot_core_measure(const void* plot, int series_id, double x);
size_t forge_plot_core_rects(const void* plot, ForgePlotRectCore* rects, size_t capacity);
ForgePlotRectCore forge_plot_core_annotation_rect(const void* plot, int kind);
ForgePlotHitCore forge_plot_core_hit(const void* plot, double x, double y, double tolerance);
size_t forge_plot_core_select(const void* plot, double x, double y, double width, double height, ForgePlotHitCore* hits, size_t capacity);
double forge_plot_core_coordinate(const void* plot, int axis, double pixel, int inverse);
ForgePlotRangeCore forge_plot_core_view(const void* plot, const ForgePlotRangeCore* baseline, int mode, double position, double amount, double endpoint);
int forge_plot_core_export(const void* plot, const char* path, int format);

#ifdef __cplusplus
}
#endif
