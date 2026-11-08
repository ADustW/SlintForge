// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

include!(concat!(env!("OUT_DIR"), "/slintforge_style.rs"));
use slint::{ComponentHandle, Model};
slintforge_ui! {
    import {Palette} from "std-widgets.slint";
    import {ForgePlot} from "../../../ui/charts/plot.slint";
    import {ForgePlotKind,ForgePlotSeries,ForgePlotScene} from "../../../ui/types/plot.slint";
    export {ForgePlotServices,ForgePlotScene,ForgePlotAxis,ForgePlotRange,ForgePlotRect,ForgePlotHit,ForgePlotSelection,ForgePlotExportResult,ForgePlotPointStyle} from "../../../ui/types/plot.slint";
    export component ChartTestWindow inherits Window {
        width:720px;height:480px;
        in-out property<ForgePlotKind> kind: graph;
        in-out property<bool> dark:false;
        in-out property<float> inner:0;
        changed dark => {Palette.color-scheme=root.dark ? ColorScheme.dark : ColorScheme.light;}
        in-out property<[ForgePlotSeries]> series: [{id:17,kind:root.kind,name:"Series",inner-radius:root.inner,color:Palette.accent-background,fill-color:Palette.accent-background.with-alpha(0.2),point-size:16,points:[{x:0,y:2},{x:1,y:4},{x:2,y:3}]}];
        out property<ForgePlotScene> scene: plot.scene;
        plot:=ForgePlot {width:720px;height:480px;series<=>root.series;panels:[{id:0,width:1,height:1,categories:["A","B","C"]}];legend-placement:outside;}
    }
}
#[test]
fn migrated_charts_share_hit_legend_theme_and_exports() {
    i_slint_backend_testing::init_no_event_loop();
    let ui=ChartTestWindow::new().unwrap();
    slintforge::bind_plot_services!(ui,ForgePlotServices,ForgePlotScene,ForgePlotAxis,ForgePlotRange,ForgePlotRect,ForgePlotHit,ForgePlotSelection,ForgePlotExportResult);
    slint::platform::update_timers_and_animations();
    let directory=std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../build/winui3/rust-chart-exports");std::fs::create_dir_all(&directory).unwrap();
    for (index,kind) in [ForgePlotKind::Graph,ForgePlotKind::Bars,ForgePlotKind::Bubble,ForgePlotKind::Pie,ForgePlotKind::PolarArea,ForgePlotKind::Radar].into_iter().enumerate(){
        ui.set_kind(kind);let scene=ui.get_scene();assert_eq!(scene.status,0,"{}",scene.error);assert_eq!(scene.input_points,3);
        for format in ["svg","png","jpeg","bmp","pdf"] {let path=directory.join(format!("{index}.{format}"));let result=ui.global::<ForgePlotServices>().invoke_export(scene.token,path.to_string_lossy().as_ref().into(),format.into());assert!(result.ok,"{}",result.error);assert!(std::fs::metadata(path).unwrap().len()>100);}
    }
    ui.set_kind(ForgePlotKind::Pie);ui.set_inner(0.6);let before=ui.get_scene();let rect=before.rects.row_data(0).unwrap();
    let service=ui.global::<ForgePlotServices>();let center=service.invoke_hit_test(before.token,rect.x+rect.width/2.,rect.y+rect.height/2.,8.);assert!(center.kind<0,"环形中心应穿透");
    let hidden=service.invoke_toggle_point(ui.get_series(),17,0);ui.set_series(hidden);let scene=ui.get_scene();assert_eq!(scene.rendered_points,2);
    let styles=ui.get_series().row_data(0).unwrap().point_styles;assert!(styles.row_data(0).unwrap().hidden);
    ui.set_dark(true);slint::platform::update_timers_and_animations();let dark=ui.get_scene();assert_eq!(dark.token,scene.token);assert_eq!(dark.rendered_points,2);assert_ne!(dark.svg,scene.svg,"主题须更新缓存");
    let restored=service.invoke_toggle_point(ui.get_series(),17,0);ui.set_series(restored);assert_eq!(ui.get_scene().rendered_points,3);
}
