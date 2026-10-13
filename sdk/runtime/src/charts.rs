//! 图表的数值与几何服务；颜色、布局、绘制、交互状态仍由共享 Slint 控件负责。
use crate::pixels::Color;
use std::f64::consts::PI;

#[derive(Clone, Default, Debug)]
pub struct ChartPointData {
    pub x: f64,
    pub y: f64,
    pub size: f64,
    pub label: String,
    pub color: Color,
}
#[derive(Clone, Default, Debug)]
pub struct ChartSeriesData {
    pub name: String,
    pub color: Color,
    pub points: Vec<ChartPointData>,
    pub hidden: bool,
}
#[derive(Clone, Default, Debug)]
pub struct ChartSliceData {
    pub label: String,
    pub value: f64,
    pub color: Color,
    pub hidden: bool,
}
#[derive(Clone, Debug)]
pub struct ChartOptionsData {
    pub auto_range: bool,
    pub x_min: f64,
    pub x_max: f64,
    pub y_min: f64,
    pub y_max: f64,
    pub include_zero: bool,
    pub stacked: bool,
    pub smooth: bool,
    pub fill_area: bool,
    pub doughnut: bool,
    pub point_size: f64,
    pub precision: i32,
}
impl Default for ChartOptionsData {
    fn default() -> Self {
        Self {
            auto_range: true,
            x_min: 0.,
            x_max: 10.,
            y_min: 0.,
            y_max: 100.,
            include_zero: false,
            stacked: false,
            smooth: false,
            fill_area: false,
            doughnut: false,
            point_size: 5.,
            precision: 2,
        }
    }
}
#[derive(Clone, Default, Debug)]
pub struct ChartMarkData {
    pub kind: i32,
    pub x: f64,
    pub y: f64,
    pub width: f64,
    pub height: f64,
    pub radius: f64,
    pub x1: f64,
    pub y1: f64,
    pub x2: f64,
    pub y2: f64,
    pub x3: f64,
    pub y3: f64,
    pub start_angle: f64,
    pub sweep_angle: f64,
    pub color: Color,
    pub series_index: i32,
    pub point_index: i32,
    pub value: f64,
    pub label: String,
}
#[derive(Clone, Default, Debug)]
pub struct ChartLineData {
    pub x: f64,
    pub y: f64,
    pub x1: f64,
    pub y1: f64,
    pub c1x: f64,
    pub c1y: f64,
    pub c2x: f64,
    pub c2y: f64,
    pub color: Color,
}
#[derive(Clone, Default, Debug)]
pub struct ChartTickData {
    pub position: f64,
    pub label: String,
}
#[derive(Clone, Default, Debug)]
pub struct ChartLabelData {
    pub x: f64,
    pub y: f64,
    pub text: String,
}
#[derive(Clone, Debug)]
pub struct ChartSceneData {
    pub status: i32,
    pub x_min: f64,
    pub x_max: f64,
    pub y_min: f64,
    pub y_max: f64,
    pub zero_y: f64,
    pub inner_radius: f64,
    pub marks: Vec<ChartMarkData>,
    pub areas: Vec<ChartMarkData>,
    pub lines: Vec<ChartLineData>,
    pub x_ticks: Vec<ChartTickData>,
    pub y_ticks: Vec<ChartTickData>,
    pub labels: Vec<ChartLabelData>,
}
impl Default for ChartSceneData {
    fn default() -> Self {
        Self {
            status: 1,
            x_min: 0.,
            x_max: 1.,
            y_min: 0.,
            y_max: 1.,
            zero_y: 1.,
            inner_radius: 0.,
            marks: vec![],
            areas: vec![],
            lines: vec![],
            x_ticks: vec![],
            y_ticks: vec![],
            labels: vec![],
        }
    }
}
pub fn chart_color(index: usize) -> Color {
    let colors = [
        (54, 162, 235),
        (255, 99, 132),
        (75, 192, 192),
        (255, 205, 86),
        (153, 102, 255),
        (255, 159, 64),
        (120, 134, 148),
    ];
    let (r, g, b) = colors[index % colors.len()];
    Color::from_rgb_u8(r, g, b)
}
fn chart_number(value: f64, precision: i32) -> String {
    let value = if value.abs() < 1e-10 { 0. } else { value };
    let mut text = format!("{:.*}", precision.clamp(0, 6) as usize, value);
    if text.contains('.') {
        while text.ends_with('0') {
            text.pop();
        }
        if text.ends_with('.') {
            text.pop();
        }
    }
    text
}
fn chart_nice_range(mut low: f64, mut high: f64, include_zero: bool, padding: bool) -> (f64, f64) {
    if include_zero {
        low = low.min(0.);
        high = high.max(0.);
    }
    if low == high {
        if low == 0. {
            high = 1.;
        } else {
            let delta = low.abs().max(1.) * 0.1;
            low -= delta;
            high += delta;
        }
    } else if padding {
        let delta = (high - low) * 0.05;
        low -= delta;
        high += delta;
    }
    let raw_step = (high - low) / 5.;
    let power = 10f64.powf(raw_step.log10().floor());
    let fraction = raw_step / power;
    let step = if fraction <= 1. {
        1.
    } else if fraction <= 2. {
        2.
    } else if fraction <= 2.5 {
        2.5
    } else if fraction <= 5. {
        5.
    } else {
        10.
    } * power;
    ((low / step).floor() * step, (high / step).ceil() * step)
}
fn chart_ticks(low: f64, high: f64, precision: i32) -> Vec<ChartTickData> {
    (0..=5)
        .map(|index| {
            let position = index as f64 / 5.;
            ChartTickData {
                position,
                label: chart_number(low + (high - low) * position, precision),
            }
        })
        .collect()
}
fn chart_resolve_color(value: Color, index: usize) -> Color {
    if value.alpha() == 0 {
        chart_color(index)
    } else {
        value
    }
}
fn chart_polar_point(angle: f64, radius: f64) -> (f64, f64) {
    (
        0.5 + radius * (angle * PI / 180.).cos(),
        0.5 + radius * (angle * PI / 180.).sin(),
    )
}
fn chart_line(
    a: (f64, f64),
    b: (f64, f64),
    previous: (f64, f64),
    next: (f64, f64),
    smooth: bool,
    color: Color,
) -> ChartLineData {
    let (c1x, c1y, c2x, c2y) = if smooth {
        (
            a.0 + (b.0 - previous.0) / 6.,
            a.1 + (b.1 - previous.1) / 6.,
            b.0 - (next.0 - a.0) / 6.,
            b.1 - (next.1 - a.1) / 6.,
        )
    } else {
        (
            a.0 + (b.0 - a.0) / 3.,
            a.1 + (b.1 - a.1) / 3.,
            a.0 + 2. * (b.0 - a.0) / 3.,
            a.1 + 2. * (b.1 - a.1) / 3.,
        )
    };
    ChartLineData {
        x: a.0,
        y: a.1,
        x1: b.0,
        y1: b.1,
        c1x,
        c1y,
        c2x,
        c2y,
        color,
    }
}
/// 0～6 依次为柱、线、饼、极坐标面积、气泡、散点、雷达；几何位置为 0～1。
pub fn prepare_chart(
    kind: i32,
    series: &[ChartSeriesData],
    slices: &[ChartSliceData],
    categories: &[String],
    options: &ChartOptionsData,
) -> ChartSceneData {
    let mut scene = ChartSceneData::default();
    if !(0..=6).contains(&kind) {
        scene.status = 2;
        return scene;
    }
    if kind == 2 || kind == 3 {
        let values = slices
            .iter()
            .enumerate()
            .filter(|(_, s)| !s.hidden && s.value.is_finite() && s.value > 0.)
            .collect::<Vec<_>>();
        let total: f64 = values.iter().map(|(_, s)| s.value).sum();
        let maximum = values.iter().map(|(_, s)| s.value).fold(0f64, f64::max);
        if total <= 0. || !total.is_finite() {
            return scene;
        }
        scene.status = 0;
        scene.y_max = maximum;
        scene.inner_radius = if kind == 2 && options.doughnut {
            0.23
        } else {
            0.
        };
        let mut angle = -90.;
        for (index, slice) in values.iter().copied() {
            let sweep = if kind == 3 {
                360. / values.len() as f64
            } else {
                slice.value / total * 360.
            };
            let radius = if kind == 3 {
                0.45 * (slice.value / maximum).sqrt()
            } else {
                0.45
            };
            let a = chart_polar_point(angle, radius);
            let b = chart_polar_point(angle + sweep / 2., radius);
            let c = chart_polar_point(angle + sweep, radius);
            scene.marks.push(ChartMarkData {
                kind: 2,
                x: 0.5,
                y: 0.5,
                radius,
                x1: a.0,
                y1: a.1,
                x2: b.0,
                y2: b.1,
                x3: c.0,
                y3: c.1,
                start_angle: angle,
                sweep_angle: sweep,
                color: chart_resolve_color(slice.color, index),
                series_index: -1,
                point_index: index as i32,
                value: slice.value,
                label: format!(
                    "{}: {} ({}%)",
                    slice.label,
                    chart_number(slice.value, options.precision),
                    chart_number(slice.value / total * 100., 1)
                ),
                ..Default::default()
            });
            angle += sweep;
        }
        if kind == 3 {
            scene.y_ticks = chart_ticks(0., maximum, options.precision);
        }
        return scene;
    }
    let visible = series
        .iter()
        .enumerate()
        .filter(|(_, s)| !s.hidden)
        .collect::<Vec<_>>();
    let mut all = visible
        .iter()
        .flat_map(|(_, s)| s.points.iter())
        .filter(|p| {
            p.x.is_finite() && p.y.is_finite() && (kind != 4 || p.size.is_finite() && p.size > 0.)
        })
        .collect::<Vec<_>>();
    if all.is_empty() {
        return scene;
    }
    let categorical = kind == 0 || kind == 6 || kind == 1 && !categories.is_empty();
    let inferred = all
        .iter()
        .filter(|p| p.x >= 0. && p.x < 10000.)
        .map(|p| p.x.floor() as usize + 1)
        .max()
        .unwrap_or(0);
    let count = if categories.is_empty() {
        inferred
    } else {
        categories.len()
    };
    if categorical && (count == 0 || count > 10000 || kind == 6 && count < 3) {
        scene.status = 2;
        return scene;
    }
    // 自动范围与实际绘制采用同一类别域，越界点不能压缩有效数据的高度。
    if categorical {
        all.retain(|p| p.x >= 0. && p.x < count as f64);
        if all.is_empty() {
            return scene;
        }
    }
    let mut xmin = all.iter().map(|p| p.x).fold(f64::INFINITY, f64::min);
    let mut xmax = all.iter().map(|p| p.x).fold(f64::NEG_INFINITY, f64::max);
    let mut ymin = all.iter().map(|p| p.y).fold(f64::INFINITY, f64::min);
    let mut ymax = all.iter().map(|p| p.y).fold(f64::NEG_INFINITY, f64::max);
    if kind == 0 && options.stacked {
        let mut positive = vec![0.; count];
        let mut negative = vec![0.; count];
        for point in &all {
            if point.x < 0. || point.x >= count as f64 {
                continue;
            }
            let index = point.x.floor() as usize;
            if point.y >= 0. {
                positive[index] += point.y;
            } else {
                negative[index] += point.y;
            }
        }
        ymin = negative.into_iter().fold(0f64, f64::min);
        ymax = positive.into_iter().fold(0f64, f64::max);
    }
    if kind == 6 {
        ymin = 0.;
        ymax = ymax.max(0.);
    }
    if options.auto_range {
        (xmin, xmax) = if categorical {
            (0., count as f64)
        } else {
            chart_nice_range(xmin, xmax, false, true)
        };
        (ymin, ymax) = chart_nice_range(
            ymin,
            ymax,
            options.include_zero || kind == 0 || kind == 6,
            kind == 4 || kind == 5,
        );
    } else {
        (xmin, xmax) = if categorical {
            (0., count as f64)
        } else {
            (options.x_min, options.x_max)
        };
        (ymin, ymax) = (options.y_min, options.y_max);
    }
    if ![xmin, xmax, ymin, ymax].iter().all(|v| v.is_finite())
        || xmin >= xmax
        || ymin >= ymax
        || !(xmax - xmin).is_finite()
        || !(ymax - ymin).is_finite()
        || kind == 6 && ymin < 0.
    {
        scene.status = 2;
        return scene;
    }
    scene.status = 0;
    scene.x_min = xmin;
    scene.x_max = xmax;
    scene.y_min = ymin;
    scene.y_max = ymax;
    scene.zero_y = 1. - (0. - ymin) / (ymax - ymin);
    let xn = |x: f64| {
        if categorical {
            (x + 0.5) / count as f64
        } else {
            (x - xmin) / (xmax - xmin)
        }
    };
    let yn = |y: f64| 1. - (y - ymin) / (ymax - ymin);
    scene.y_ticks = chart_ticks(ymin, ymax, options.precision);
    if categorical {
        let stride = count.div_ceil(8);
        scene.x_ticks = (0..count)
            .filter(|i| i % stride == 0)
            .map(|i| ChartTickData {
                position: (i as f64 + 0.5) / count as f64,
                label: categories
                    .get(i)
                    .cloned()
                    .unwrap_or_else(|| (i + 1).to_string()),
            })
            .collect();
    } else {
        scene.x_ticks = chart_ticks(xmin, xmax, options.precision);
    }
    if kind == 6 {
        scene.x_ticks.clear();
        scene.labels = (0..count)
            .map(|index| {
                let (x, y) = chart_polar_point(-90. + 360. * index as f64 / count as f64, 0.51);
                ChartLabelData {
                    x,
                    y,
                    text: categories
                        .get(index)
                        .cloned()
                        .unwrap_or_else(|| (index + 1).to_string()),
                }
            })
            .collect();
    }
    let mut positive = vec![0.; if categorical { count } else { 0 }];
    let mut negative = positive.clone();
    for (order, (si, data)) in visible.iter().copied().enumerate() {
        let color = chart_resolve_color(data.color, si);
        let name = if data.name.is_empty() {
            format!("数据集 {}", si + 1)
        } else {
            data.name.clone()
        };
        let projected = data
            .points
            .iter()
            .enumerate()
            .map(|(pi, p)| {
                if !p.x.is_finite()
                    || !p.y.is_finite()
                    || categorical && (p.x < 0. || p.x >= count as f64)
                    || kind == 4 && (!p.size.is_finite() || p.size <= 0.)
                {
                    return None;
                }
                let label = if !p.label.is_empty() {
                    p.label.clone()
                } else if categorical {
                    categories
                        .get(p.x.floor() as usize)
                        .cloned()
                        .unwrap_or_else(|| (p.x.floor() as usize + 1).to_string())
                } else {
                    chart_number(p.x, options.precision)
                };
                let point_color = if p.color.alpha() == 0 { color } else { p.color };
                let mut mark = ChartMarkData {
                    kind: 1,
                    x: xn(p.x),
                    y: yn(p.y),
                    radius: if kind == 4 {
                        p.size
                    } else {
                        options.point_size.max(0.)
                    },
                    color: point_color,
                    series_index: si as i32,
                    point_index: pi as i32,
                    value: p.y,
                    label: format!(
                        "{} · {}: {}",
                        name,
                        label,
                        chart_number(p.y, options.precision)
                    ),
                    ..Default::default()
                };
                if kind == 0 {
                    let category = p.x.floor() as usize;
                    let base = if options.stacked {
                        if p.y >= 0. {
                            positive[category]
                        } else {
                            negative[category]
                        }
                    } else {
                        0.
                    };
                    let end = base + p.y;
                    if options.stacked {
                        if p.y >= 0. {
                            positive[category] = end;
                        } else {
                            negative[category] = end;
                        }
                    }
                    let parts = if options.stacked { 1 } else { visible.len() };
                    let part = if options.stacked { 0 } else { order };
                    mark.kind = 0;
                    mark.x =
                        (category as f64 + 0.12 + 0.76 * part as f64 / parts as f64) / count as f64;
                    mark.width = 0.76 / parts as f64 / count as f64;
                    mark.y = yn(base).min(yn(end));
                    mark.height = (yn(base) - yn(end)).abs();
                } else if kind == 6 {
                    let angle = -90. + 360. * p.x / count as f64;
                    let value = ((p.y - ymin) / (ymax - ymin)).clamp(0., 1.);
                    (mark.x, mark.y) = chart_polar_point(angle, 0.45 * value);
                }
                Some(mark)
            })
            .collect::<Vec<_>>();
        let original = data.points.len();
        if kind == 1 || kind == 6 {
            let pairs = if kind == 6 {
                original
            } else {
                original.saturating_sub(1)
            };
            for index in 0..pairs {
                let next = (index + 1) % original;
                let (Some(a), Some(b)) = (&projected[index], &projected[next]) else {
                    continue;
                };
                let previous = if index > 0 {
                    projected[index - 1].as_ref().unwrap_or(a)
                } else {
                    a
                };
                let following = projected
                    .get(next + 1)
                    .and_then(|p| p.as_ref())
                    .unwrap_or(b);
                scene.lines.push(chart_line(
                    (a.x, a.y),
                    (b.x, b.y),
                    (previous.x, previous.y),
                    (following.x, following.y),
                    options.smooth && kind == 1,
                    color,
                ));
                if options.fill_area || kind == 6 {
                    let (center_x, center_y) = if kind == 6 {
                        (0.5, 0.5)
                    } else {
                        (b.x, scene.zero_y.clamp(0., 1.))
                    };
                    scene.areas.push(ChartMarkData {
                        kind: 3,
                        x: a.x,
                        y: a.y,
                        x1: b.x,
                        y1: b.y,
                        x2: center_x,
                        y2: center_y,
                        x3: if kind == 6 { 0.5 } else { a.x },
                        y3: center_y,
                        color,
                        ..Default::default()
                    });
                }
            }
        }
        scene.marks.extend(projected.into_iter().flatten());
    }
    if scene.marks.is_empty() {
        scene.status = 1;
    }
    scene
}
/// 使用绘制区域的逻辑像素进行命中测试；气泡半径以逻辑像素计。
pub fn chart_hit_test(
    marks: &[ChartMarkData],
    x: f64,
    y: f64,
    width: f64,
    height: f64,
    inner_radius: f64,
) -> i32 {
    if ![x, y, width, height].iter().all(|v| v.is_finite()) || width <= 0. || height <= 0. {
        return -1;
    }
    for (index, mark) in marks.iter().enumerate().rev() {
        let hit = match mark.kind {
            0 => {
                x >= mark.x && x <= mark.x + mark.width && y >= mark.y && y <= mark.y + mark.height
            }
            1 => ((x - mark.x) * width).hypot((y - mark.y) * height) <= mark.radius.max(4.) + 4.,
            2 => {
                let distance = (x - 0.5).hypot(y - 0.5);
                let mut angle = ((y - 0.5).atan2(x - 0.5) * 180. / PI - mark.start_angle) % 360.;
                if angle < 0. {
                    angle += 360.;
                }
                distance >= inner_radius
                    && distance <= mark.radius
                    && angle <= mark.sweep_angle + 1e-7
            }
            _ => false,
        };
        if hit {
            return index as i32;
        }
    }
    -1
}

// 输出类型由当前窗口生成；无需重复实现 UI，也无需跨语言 FFI。
