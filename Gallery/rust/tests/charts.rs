use slint::platform::{Key, PointerEventButton, WindowEvent};
use slint::{ComponentHandle, LogicalPosition, Model, ModelRc, VecModel};
use std::time::Duration;

slint::slint! {
    import { ForgeChart } from "../../../ui/charts/charts.slint";
    import { ForgeChartScene, ForgeChartKind, ForgeChartSeries, ForgeChartPoint, ForgeChartSlice } from "../../../ui/types/charts.slint";
    export { ForgeChartServices, ForgeChartScene, ForgeChartMark, ForgeChartLine, ForgeChartTick, ForgeChartLabel, ForgeChartKind, ForgeChartSeries, ForgeChartPoint, ForgeChartSlice } from "../../../ui/types/charts.slint";
    export component ChartTestWindow inherits Window {
        width: 640px; height: 420px;
        in-out property<ForgeChartKind> kind: bar;
        in-out property<[ForgeChartSeries]> series: [
            {name: "A", points: [{x: 0, y: 20, size: 12}, {x: 1, y: 60, size: 18}, {x: 2, y: 40, size: 10}]},
            {name: "B", points: [{x: 0, y: 30, size: 15}, {x: 1, y: 10, size: 20}, {x: 2, y: 50, size: 9}]}
        ];
        in-out property<[ForgeChartSlice]> slices: [{label: "A", value: 1}, {label: "B", value: 3}];
        in-out property<bool> enabled: true;
        in-out property<bool> stacked: false;
        in-out property<bool> doughnut: false;
        in-out property<bool> auto-range: true;
        in-out property<float> y-max: 100;
        out property<ForgeChartScene> scene: chart.scene;
        out property<int> selection: chart.selected-index;
        out property<int> hover: chart.hovered-index;
        in-out property<int> selections: 0;
        callback hide-series(int, bool); hide-series(index, value) => { chart.hide-series(index, value); }
        callback hide-slice(int, bool); hide-slice(index, value) => { chart.hide-slice(index, value); }
        callback refresh(); refresh => { chart.refresh(); }
        chart := ForgeChart {
            x: 20px; y: 20px; width: 600px; height: 370px;
            kind: root.kind; series: root.series; slices: root.slices; categories: ["A", "B", "C"];
            enabled: root.enabled; stacked: root.stacked; doughnut: root.doughnut;
            auto-range: root.auto-range; y-min: 0; y-max: root.y-max;
            include-zero: true; smooth: true; fill-area: true;
            point-selected(a,b,c) => { root.selections += 1; }
        }
    }
}

fn tick() {
    i_slint_backend_testing::mock_elapsed_time(Duration::from_millis(1));
    slint::platform::update_timers_and_animations();
}
fn click(ui: &ChartTestWindow, x: f32, y: f32) {
    let position = LogicalPosition::new(x, y);
    ui.window().dispatch_event(WindowEvent::PointerMoved { position });
    ui.window().dispatch_event(WindowEvent::PointerPressed { position, button: PointerEventButton::Left });
    ui.window().dispatch_event(WindowEvent::PointerReleased { position, button: PointerEventButton::Left });
    tick();
}
fn key(ui: &ChartTestWindow, key: Key) {
    let text: slint::SharedString = key.into();
    ui.window().dispatch_event(WindowEvent::KeyPressed { text: text.clone() });
    ui.window().dispatch_event(WindowEvent::KeyReleased { text });
    tick();
}

#[test]
fn charts_bind_models_render_geometry_and_handle_input() {
    i_slint_backend_testing::init_no_event_loop();
    let ui = ChartTestWindow::new().unwrap();
    assert_eq!(ui.get_scene().status, 3);
    slintforge::bind_chart_services!(ui, ForgeChartServices, ForgeChartScene, ForgeChartMark, ForgeChartLine, ForgeChartTick, ForgeChartLabel);
    ui.show().unwrap();
    tick();
    assert_eq!(ui.get_scene().marks.row_count(), 6);
    tick();
    let first = ui.get_scene().marks.row_data(0).unwrap();
    let x = 72. + (first.x + first.width / 2.) * 528.;
    let y = 32. + (first.y + first.height / 2.) * 278.;
    ui.window().dispatch_event(WindowEvent::PointerMoved { position: LogicalPosition::new(x, y) });
    assert_eq!(ui.get_hover(), 0);
    click(&ui, x, y);
    assert_eq!(ui.get_hover(), 0);
    assert_eq!(ui.get_selection(), 0);
    assert_eq!(ui.get_selections(), 1);
    key(&ui, Key::RightArrow);
    assert_eq!(ui.get_selection(), 1);
    key(&ui, Key::Escape);
    key(&ui, Key::LeftArrow);
    assert_eq!(ui.get_selection(), 5);
    ui.set_enabled(false);
    click(&ui, x, y);
    key(&ui, Key::Home);
    assert_eq!(ui.get_selection(), 5);
    ui.set_enabled(true);
    ui.invoke_hide_series(1, true);
    assert_eq!(ui.get_scene().marks.row_count(), 3);
    ui.invoke_hide_series(1, false);
    ui.set_stacked(true);
    assert_eq!(ui.get_scene().y_max, 100.);
    for kind in [ForgeChartKind::Line, ForgeChartKind::Bubble, ForgeChartKind::Scatter, ForgeChartKind::Radar] {
        ui.set_kind(kind);
        assert_eq!(ui.get_scene().status, 0);
        assert_eq!(ui.get_scene().marks.row_count(), 6);
    }
    assert_eq!(ui.get_scene().lines.row_count(), 6);
    ui.set_kind(ForgeChartKind::Pie);
    assert_eq!(ui.get_scene().marks.row_count(), 2);
    ui.set_doughnut(true);
    assert!(ui.get_scene().inner_radius > 0.);
    ui.invoke_hide_slice(0, true);
    assert_eq!(ui.get_scene().marks.row_count(), 1);
    assert_eq!(ui.get_scene().marks.row_data(0).unwrap().sweep_angle, 360.);
    ui.set_kind(ForgeChartKind::Polar);
    assert_eq!(ui.get_scene().status, 0);
    ui.set_kind(ForgeChartKind::Line);
    ui.set_auto_range(false);
    ui.set_y_max(0.);
    assert_eq!(ui.get_scene().status, 2);
    ui.set_auto_range(true);
    let points = ModelRc::new(VecModel::from(vec![ForgeChartPoint { x: 1., y: 42., ..Default::default() }]));
    ui.set_series(ModelRc::new(VecModel::from(vec![ForgeChartSeries { name: "Updated".into(), points: points.clone(), ..Default::default() }])));
    assert_eq!(ui.get_scene().marks.row_count(), 1);
    assert_eq!(ui.get_scene().marks.row_data(0).unwrap().value, 42.);
    points.set_row_data(0, ForgeChartPoint { x: 1., y: 73., ..Default::default() });
    ui.invoke_refresh();
    assert_eq!(ui.get_scene().marks.row_data(0).unwrap().value, 73.);
    ui.set_series(ModelRc::new(VecModel::from(Vec::<ForgeChartSeries>::new())));
    assert_eq!(ui.get_scene().status, 1);
    // Slint 的 changed 回调在下一轮更新执行。
    tick();
    assert_eq!(ui.get_selection(), -1);
}
