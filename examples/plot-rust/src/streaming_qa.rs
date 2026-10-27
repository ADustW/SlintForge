use crate::{ForgePlotPoint, ForgePlotServices, ProfessionalPlotWindow};
use slint::ComponentHandle;
use std::time::{Duration, Instant};

// 实际窗口验证八种图例、真实定时采样跨越旧缓冲边界、暂停恢复和历史滚动。
pub fn start(ui: &ProfessionalPlotWindow, timer: &slint::Timer) {
    let directory = crate::performance::directory();
    std::fs::create_dir_all(&directory).unwrap();
    let weak = ui.as_weak();
    let mut phase = 0;
    let mut frames = 0;
    let mut previous_count = 0;
    let mut paused_count = 0;
    let started = Instant::now();
    let mut csv = String::from("elapsed_s,history_points,rendered_points,prepare_ms,capture_ms\n");
    timer.start(slint::TimerMode::Repeated,Duration::from_millis(500),move ||{
        let ui=weak.upgrade().unwrap();
        if phase==0{
            for position in 0..8{for style in 0..3{ui.set_legend_position_index(position);ui.set_legend_style_index(style);let scene=ui.get_scene();assert_eq!(scene.status,0,"{}",scene.error);let image=slintforge::capture_window(ui.window()).unwrap();slintforge::save_capture_png(&image,&directory.join(format!("legend-{position}-{style}.png"))).unwrap();}}
            for style in 0..6{ui.set_line_style_index(style);assert_eq!(ui.get_scene().status,0);let image=slintforge::capture_window(ui.window()).unwrap();slintforge::save_capture_png(&image,&directory.join(format!("line-style-{style}.png"))).unwrap();}ui.set_line_style_index(0);
            for ticker in 0..5{ui.set_axis_ticker_index(ticker);assert_eq!(ui.get_scene().status,0);let image=slintforge::capture_window(ui.window()).unwrap();slintforge::save_capture_png(&image,&directory.join(format!("axis-ticker-{ticker}.png"))).unwrap();}ui.set_axis_ticker_index(0);
            performance(&ui,&directory);
            ui.set_legend_position_index(4);ui.set_legend_style_index(1);ui.set_streaming(true);ui.set_live(true);ui.set_window_mode(1);let _=ui.get_scene();ui.invoke_sample_running(true);phase=1;return;
        }
        let start=Instant::now();ui.invoke_flush();let scene=ui.get_scene();let prepare_ms=start.elapsed().as_secs_f64()*1000.;assert!(scene.status<2,"{}",scene.error);assert!(scene.input_points>=previous_count);previous_count=scene.input_points;
        let capture=Instant::now();let image=slintforge::capture_window(ui.window()).unwrap();let capture_ms=capture.elapsed().as_secs_f64()*1000.;
        csv+=&format!("{:.3},{},{},{prepare_ms:.3},{capture_ms:.3}\n",started.elapsed().as_secs_f64(),scene.input_points,scene.rendered_points);
        frames+=1;
        if phase==1&&frames>=44{
            assert!(scene.input_points>4000,"实际采样必须跨越每曲线 2000 点边界");assert!(scene.history_range.maximum>20.);assert!((scene.window_range.maximum-scene.window_range.minimum-10.).abs()<0.001);
            let oldest=ui.global::<ForgePlotServices>().invoke_measure(scene.token,0,0.01);assert_eq!(oldest.index,0,"无限历史必须保留首点");
            ui.invoke_sample_running(false);ui.set_live(false);paused_count=scene.input_points;phase=2;frames=0;slintforge::save_capture_png(&image,&directory.join("live-before-pause.png")).unwrap();return;
        }
        if phase==2{assert_eq!(scene.input_points,paused_count,"暂停后数据仍在追加");if frames>=2{ui.set_live(true);ui.invoke_sample_running(true);phase=3;frames=0;}return;}
        if phase==3&&frames>=4{
            assert!(scene.input_points>paused_count);let count=scene.input_points/2;
            // 每点的 X 为原始索引加一乘 0.01；恢复后也不能重置或重复时间。
            let sample=ui.global::<ForgePlotServices>().invoke_measure(scene.token,0,count as f32*0.01);assert_eq!(sample.index,count-1);assert!((sample.x-count as f32*0.01).abs()<0.001);
            ui.invoke_sample_running(false);ui.set_live(false);ui.invoke_scroll_history(0.);let scene=ui.get_scene();assert!(scene.window_range.minimum<0.02);ui.set_measure(true);
            let image=slintforge::capture_window(ui.window()).unwrap();slintforge::save_capture_png(&image,&directory.join("history-measure.png")).unwrap();
            ui.invoke_reset();let scene=ui.get_scene();assert!(scene.window_range.maximum>20.);
            std::fs::write(directory.join("streaming-window-performance.csv"),&csv).unwrap();
            println!("PLOT_STREAMING_WINDOW_OK host=rust history={} frames={} legends=24 pause_resume=passed",scene.input_points,csv.lines().count()-1);slint::quit_event_loop().unwrap();
        }
    });
}
pub(crate) fn performance(ui: &ProfessionalPlotWindow, directory: &std::path::Path) {
    ui.set_streaming(true);
    ui.set_window_mode(1);
    let _ = ui.get_scene();
    let points = (0..1000000)
        .map(|i| ForgePlotPoint {
            x: i as f32 * 0.001,
            y: (i as f32 * 0.003).sin() * 2. + 5.,
            ..Default::default()
        })
        .collect::<Vec<_>>();
    assert!(ui.invoke_append_points(0, slint::ModelRc::new(slint::VecModel::from(points))));
    ui.invoke_flush();
    let scene = ui.get_scene();
    assert_eq!(scene.input_points, 1000000);
    let _ = slintforge::capture_window(ui.window()).unwrap();
    let mut csv =
        String::from("history_points,append_ms,prepare_ms,capture_ms,total_ms,rendered_points\n");
    for batch in 0..64 {
        let points = (0..32)
            .map(|i| {
                let x = (1000000 + batch * 32 + i) as f32 * 0.001;
                ForgePlotPoint {
                    x,
                    y: (x * 3.).sin() * 2. + 5.,
                    ..Default::default()
                }
            })
            .collect::<Vec<_>>();
        let start = Instant::now();
        assert!(ui.invoke_append_points(0, slint::ModelRc::new(slint::VecModel::from(points))));
        let append_ms = start.elapsed().as_secs_f64() * 1000.;
        let prepared = Instant::now();
        ui.invoke_flush();
        let scene = ui.get_scene();
        let prepare_ms = prepared.elapsed().as_secs_f64() * 1000.;
        let captured = Instant::now();
        let _ = slintforge::capture_window(ui.window()).unwrap();
        let capture_ms = captured.elapsed().as_secs_f64() * 1000.;
        assert!(scene.rendered_points < 5000);
        assert_eq!(scene.input_points, 1000000 + (batch + 1) * 32);
        csv += &format!(
            "{},{append_ms:.3},{prepare_ms:.3},{capture_ms:.3},{:.3},{}\n",
            scene.input_points,
            start.elapsed().as_secs_f64() * 1000.,
            scene.rendered_points
        );
    }
    std::fs::write(directory.join("incremental-frame-performance.csv"), csv).unwrap();
    ui.set_streaming(false);
    ui.invoke_reset();
    let _ = ui.get_scene();
}
