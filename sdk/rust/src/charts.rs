use slint::Color;
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
use slintforge_runtime_api::models::*;
use slintforge_runtime_api::{Arena, Wire};
pub fn chart_color(index: usize) -> Color {
    let args = ArgsChartColor { index };
    crate::wire_color_from(crate::runtime::value::<u32, _>(OP_CHARTCOLOR, &args))
}
pub fn prepare_chart(
    kind: i32,
    series: &[ChartSeriesData],
    slices: &[ChartSliceData],
    categories: &[String],
    options: &ChartOptionsData,
) -> ChartSceneData {
    let mut arena = Arena::default();
    let args = ArgsChartPrepare {
        kind,
        series: slintforge_runtime_api::slice_view(series, &mut arena),
        slices: slintforge_runtime_api::slice_view(slices, &mut arena),
        categories: slintforge_runtime_api::slice_view(categories, &mut arena),
        options: options.view(&mut arena),
    };
    crate::runtime::value::<ChartSceneData, _>(OP_CHARTPREPARE, &args)
}
pub fn chart_hit_test(
    marks: &[ChartMarkData],
    x: f64,
    y: f64,
    width: f64,
    height: f64,
    inner_radius: f64,
) -> i32 {
    let mut arena = Arena::default();
    let args = ArgsChartHit {
        marks: slintforge_runtime_api::slice_view(marks, &mut arena),
        x,
        y,
        width,
        height,
        inner_radius,
    };
    crate::runtime::value::<i32, _>(OP_CHARTHIT, &args)
}
#[macro_export]
macro_rules! bind_chart_services {
    ($ui:expr, $service:ty, $scene:ty, $mark:ty, $line:ty, $tick:ty, $label:ty) => {{
        use slint::{ComponentHandle as _, Model as _};
        $crate::runtime::initialize().expect("图表运行库初始化失败");
        let ui = &$ui;
        $crate::runtime::initialize().expect("组件运行库初始化失败");
        let service = ui.global::<$service>();
        service.on_prepare(|kind, series, slices, categories, options| {
            let data = series
                .iter()
                .map(|s| $crate::charts::ChartSeriesData {
                    name: s.name.to_string(),
                    color: s.color,
                    hidden: s.hidden,
                    points: s
                        .points
                        .iter()
                        .map(|p| $crate::charts::ChartPointData {
                            x: p.x as f64,
                            y: p.y as f64,
                            size: p.size as f64,
                            label: p.label.to_string(),
                            color: p.color,
                        })
                        .collect(),
                })
                .collect::<Vec<_>>();
            let slices = slices
                .iter()
                .map(|s| $crate::charts::ChartSliceData {
                    label: s.label.to_string(),
                    value: s.value as f64,
                    color: s.color,
                    hidden: s.hidden,
                })
                .collect::<Vec<_>>();
            let categories = categories.iter().map(|v| v.to_string()).collect::<Vec<_>>();
            let options = $crate::charts::ChartOptionsData {
                auto_range: options.auto_range,
                x_min: options.x_min as f64,
                x_max: options.x_max as f64,
                y_min: options.y_min as f64,
                y_max: options.y_max as f64,
                include_zero: options.include_zero,
                stacked: options.stacked,
                smooth: options.smooth,
                fill_area: options.fill_area,
                doughnut: options.doughnut,
                point_size: options.point_size as f64,
                precision: options.precision,
            };
            let source = $crate::charts::prepare_chart(kind, &data, &slices, &categories, &options);
            let convert_mark = |p: $crate::charts::ChartMarkData| {
                let mut mark = <$mark>::default();
                mark.kind = p.kind;
                mark.x = p.x as f32;
                mark.y = p.y as f32;
                mark.width = p.width as f32;
                mark.height = p.height as f32;
                mark.radius = p.radius as f32;
                mark.x1 = p.x1 as f32;
                mark.y1 = p.y1 as f32;
                mark.x2 = p.x2 as f32;
                mark.y2 = p.y2 as f32;
                mark.x3 = p.x3 as f32;
                mark.y3 = p.y3 as f32;
                mark.start_angle = p.start_angle as f32;
                mark.sweep_angle = p.sweep_angle as f32;
                mark.color = p.color;
                mark.series_index = p.series_index;
                mark.point_index = p.point_index;
                mark.value = p.value as f32;
                mark.label = p.label.into();
                mark
            };
            let mut result = <$scene>::default();
            result.status = source.status;
            result.x_min = source.x_min as f32;
            result.x_max = source.x_max as f32;
            result.y_min = source.y_min as f32;
            result.y_max = source.y_max as f32;
            result.zero_y = source.zero_y as f32;
            result.inner_radius = source.inner_radius as f32;
            result.marks = std::rc::Rc::new(slint::VecModel::from(
                source
                    .marks
                    .into_iter()
                    .map(convert_mark)
                    .collect::<Vec<_>>(),
            ))
            .into();
            result.areas = std::rc::Rc::new(slint::VecModel::from(
                source
                    .areas
                    .into_iter()
                    .map(convert_mark)
                    .collect::<Vec<_>>(),
            ))
            .into();
            result.lines = std::rc::Rc::new(slint::VecModel::from(
                source
                    .lines
                    .into_iter()
                    .map(|p| {
                        let mut line = <$line>::default();
                        line.x = p.x as f32;
                        line.y = p.y as f32;
                        line.x1 = p.x1 as f32;
                        line.y1 = p.y1 as f32;
                        line.c1x = p.c1x as f32;
                        line.c1y = p.c1y as f32;
                        line.c2x = p.c2x as f32;
                        line.c2y = p.c2y as f32;
                        line.color = p.color;
                        line
                    })
                    .collect::<Vec<_>>(),
            ))
            .into();
            let ticks = |input: Vec<$crate::charts::ChartTickData>| -> slint::ModelRc<$tick> {
                std::rc::Rc::new(slint::VecModel::from(
                    input
                        .into_iter()
                        .map(|p| {
                            let mut item = <$tick>::default();
                            item.position = p.position as f32;
                            item.label = p.label.into();
                            item
                        })
                        .collect::<Vec<_>>(),
                ))
                .into()
            };
            result.x_ticks = ticks(source.x_ticks);
            result.y_ticks = ticks(source.y_ticks);
            result.labels = std::rc::Rc::new(slint::VecModel::from(
                source
                    .labels
                    .into_iter()
                    .map(|p| {
                        let mut item = <$label>::default();
                        item.x = p.x as f32;
                        item.y = p.y as f32;
                        item.text = p.text.into();
                        item
                    })
                    .collect::<Vec<_>>(),
            ))
            .into();
            result
        });
        service.on_hit_test(|marks, x, y, width, height, inner| {
            let values = marks
                .iter()
                .map(|p| $crate::charts::ChartMarkData {
                    kind: p.kind,
                    x: p.x as f64,
                    y: p.y as f64,
                    width: p.width as f64,
                    height: p.height as f64,
                    radius: p.radius as f64,
                    start_angle: p.start_angle as f64,
                    sweep_angle: p.sweep_angle as f64,
                    ..Default::default()
                })
                .collect::<Vec<_>>();
            $crate::charts::chart_hit_test(
                &values,
                x as f64,
                y as f64,
                width as f64,
                height as f64,
                inner as f64,
            )
        });
        service.set_connected(true);
    }};
}

#[cfg(test)]
mod tests {
    use super::*;
    fn points(values: &[(f64, f64)]) -> ChartSeriesData {
        ChartSeriesData {
            name: "测试数据".into(),
            points: values
                .iter()
                .map(|&(x, y)| ChartPointData {
                    x,
                    y,
                    size: 15.,
                    ..Default::default()
                })
                .collect(),
            ..Default::default()
        }
    }
    fn near(a: f64, b: f64) {
        assert!((a - b).abs() < 1e-8, "{} != {}", a, b);
    }
    #[test]
    fn cartesian_ranges_stacks_gaps_and_hidden_data() {
        let mut data = vec![
            points(&[(0., 10.), (1., -5.)]),
            points(&[(0., 20.), (1., -7.)]),
        ];
        let categories = vec!["甲".into(), "乙".into()];
        let options = ChartOptionsData {
            stacked: true,
            ..Default::default()
        };
        let scene = prepare_chart(0, &data, &[], &categories, &options);
        assert_eq!((scene.status, scene.marks.len()), (0, 4));
        near(scene.y_min, -20.);
        near(scene.y_max, 30.);
        near(scene.zero_y, 0.6);
        near(scene.marks[0].height, 0.2);
        near(scene.marks[2].height, 0.4);
        near(scene.marks[3].height, 0.14);
        assert_eq!(
            chart_hit_test(&scene.marks, scene.marks[0].x + 0.05, 0.5, 500., 300., 0.),
            0
        );
        data[1].hidden = true;
        assert_eq!(
            prepare_chart(0, &data, &[], &categories, &options)
                .marks
                .len(),
            2
        );
        let line = prepare_chart(
            1,
            &[points(&[(0., 0.), (1., 10.), (f64::NAN, 5.), (3., 4.)])],
            &[],
            &[],
            &ChartOptionsData {
                smooth: true,
                fill_area: true,
                ..Default::default()
            },
        );
        assert_eq!(
            (line.marks.len(), line.lines.len(), line.areas.len()),
            (3, 1, 1)
        );
        assert!(line.x_min < 0. && line.x_max > 3.);
        let invalid = prepare_chart(
            5,
            &data,
            &[],
            &[],
            &ChartOptionsData {
                auto_range: false,
                x_min: 2.,
                x_max: 2.,
                ..Default::default()
            },
        );
        assert_eq!(invalid.status, 2);
        assert_eq!(prepare_chart(1, &[], &[], &[], &options).status, 1);
        let constant = prepare_chart(
            5,
            &[points(&[(2., 4.)])],
            &[],
            &[],
            &ChartOptionsData::default(),
        );
        assert!(
            constant.x_min < 2.
                && constant.x_max > 2.
                && constant.y_min < 4.
                && constant.y_max > 4.
        );
    }
    #[test]
    fn sectors_full_circle_holes_and_polar_area() {
        let data = vec![
            ChartSliceData {
                label: "甲".into(),
                value: 1.,
                ..Default::default()
            },
            ChartSliceData {
                label: "乙".into(),
                value: 3.,
                ..Default::default()
            },
            ChartSliceData {
                value: -4.,
                ..Default::default()
            },
        ];
        let pie = prepare_chart(
            2,
            &[],
            &data,
            &[],
            &ChartOptionsData {
                doughnut: true,
                ..Default::default()
            },
        );
        assert_eq!(pie.marks.len(), 2);
        near(pie.marks[0].sweep_angle, 90.);
        near(pie.marks[1].sweep_angle, 270.);
        assert_eq!(
            chart_hit_test(&pie.marks, 0.5, 0.5, 300., 300., pie.inner_radius),
            -1
        );
        let full = prepare_chart(2, &[], &data[..1], &[], &ChartOptionsData::default());
        near(full.marks[0].sweep_angle, 360.);
        near(full.marks[0].x1, full.marks[0].x3);
        near(full.marks[0].y1, full.marks[0].y3);
        let polar = prepare_chart(
            3,
            &[],
            &[
                ChartSliceData {
                    value: 1.,
                    ..Default::default()
                },
                ChartSliceData {
                    value: 4.,
                    ..Default::default()
                },
            ],
            &[],
            &ChartOptionsData::default(),
        );
        near(polar.marks[0].sweep_angle, 180.);
        near(polar.marks[0].radius / polar.marks[1].radius, 0.5);
        assert_eq!(chart_hit_test(&polar.marks, 0.8, 0.5, 300., 300., 0.), -1);
        assert_eq!(chart_hit_test(&polar.marks, 0.2, 0.5, 300., 300., 0.), 1);
        assert_eq!(
            prepare_chart(
                2,
                &[],
                &[ChartSliceData {
                    value: 0.,
                    ..Default::default()
                }],
                &[],
                &ChartOptionsData::default()
            )
            .status,
            1
        );
    }
    #[test]
    fn bubble_pixel_hit_and_radar_closure() {
        let bubble = prepare_chart(
            4,
            &[points(&[(0., 0.), (10., 10.)])],
            &[],
            &[],
            &ChartOptionsData::default(),
        );
        let point = &bubble.marks[0];
        assert_eq!(
            chart_hit_test(&bubble.marks, point.x + 10. / 400., point.y, 400., 200., 0.),
            0
        );
        assert_eq!(
            chart_hit_test(&bubble.marks, point.x + 30. / 400., point.y, 400., 200., 0.),
            -1
        );
        let radar = prepare_chart(
            6,
            &[points(&[(0., 1.), (1., 2.), (2., 3.)])],
            &[],
            &["甲".into(), "乙".into(), "丙".into()],
            &ChartOptionsData::default(),
        );
        assert_eq!(
            (
                radar.marks.len(),
                radar.lines.len(),
                radar.areas.len(),
                radar.labels.len()
            ),
            (3, 3, 3, 3)
        );
        near(radar.lines[2].x1, radar.lines[0].x);
        near(radar.lines[2].y1, radar.lines[0].y);
        assert_eq!(chart_hit_test(&radar.marks, 0.5, 0.5, 0., 200., 0.), -1);
        assert_eq!(
            prepare_chart(
                6,
                &[points(&[(0., 1.), (1., 2.)])],
                &[],
                &[],
                &ChartOptionsData::default()
            )
            .status,
            2
        );
    }
    #[test]
    fn geometry_contract_for_both_hosts() {
        let series = vec![
            points(&[(0., 20.), (1., 60.), (2., 40.)]),
            points(&[(0., 30.), (1., 10.), (2., 50.)]),
        ];
        let slices = vec![
            ChartSliceData {
                label: "甲".into(),
                value: 1.,
                ..Default::default()
            },
            ChartSliceData {
                label: "乙".into(),
                value: 3.,
                ..Default::default()
            },
        ];
        let categories = vec!["甲".into(), "乙".into(), "丙".into()];
        let mut output = String::new();
        for kind in 0..=6 {
            let scene = prepare_chart(
                kind,
                &series,
                &slices,
                &categories,
                &ChartOptionsData {
                    stacked: true,
                    smooth: true,
                    fill_area: true,
                    ..Default::default()
                },
            );
            assert_eq!(scene.status, 0);
            output += &format!(
                "{},{},{},{},{},{:.6},{:.6},{:.6},{:.6}\n",
                kind,
                scene.status,
                scene.marks.len(),
                scene.lines.len(),
                scene.areas.len(),
                scene.x_min,
                scene.x_max,
                scene.y_min,
                scene.y_max
            );
            for mark in scene.marks {
                output += &format!(
                    "{},{},{},{:.6},{:.6},{:.6},{:.6},{:.6},{:.6},{:.6}\n",
                    mark.kind,
                    mark.series_index,
                    mark.point_index,
                    mark.x,
                    mark.y,
                    mark.width,
                    mark.height,
                    mark.radius,
                    mark.start_angle,
                    mark.sweep_angle
                );
            }
        }
        let directory =
            std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../build/charts-qa");
        std::fs::create_dir_all(&directory).unwrap();
        std::fs::write(directory.join("rust-geometry.csv"), output).unwrap();
    }
}
