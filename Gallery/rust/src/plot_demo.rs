use crate::{ForgePlotPoint, GalleryPlotDemoModels, GalleryWindow};
use slint::{ComponentHandle, ModelRc, VecModel};
use std::rc::Rc;
use std::time::{Duration, Instant};

pub fn connect(ui: &GalleryWindow) {
    slintforge::bind_plot_services!(ui, crate::ForgePlotServices, crate::ForgePlotScene,
        crate::ForgePlotAxis, crate::ForgePlotRange, crate::ForgePlotRect,
        crate::ForgePlotHit, crate::ForgePlotSelection, crate::ForgePlotExportResult);
    let sampler = Rc::new(slint::Timer::default());
    let clock = Rc::new(std::cell::Cell::new(0f64));
    let weak = ui.as_weak();
    ui.global::<GalleryPlotDemoModels>().on_sample_running(move |running| {
        if !running { sampler.stop(); return; }
        let clock=clock.clone();let base=clock.get();let started=Instant::now();
        let weak = weak.clone();
        // 固定采样间隔，以实际运行时间批量追加；暂停恢复不重置时间和历史。
        sampler.start(slint::TimerMode::Repeated, Duration::from_millis(33), move || {
            if let Some(ui) = weak.upgrade() {
                let target=base+started.elapsed().as_secs_f64();let mut time=clock.get();let mut measured=Vec::new();let mut reference=Vec::new();
                while time+0.01<=target {time+=0.01;measured.push(ForgePlotPoint{x:time as f32,y:((time*3.).sin()*2.+5.) as f32,..Default::default()});reference.push(ForgePlotPoint{x:time as f32,y:((time*1.5).sin()+2.) as f32,..Default::default()});}
                if !measured.is_empty(){ui.invoke_append_plot_points(0,ModelRc::new(VecModel::from(measured)));ui.invoke_append_plot_points(1,ModelRc::new(VecModel::from(reference)));clock.set(time);}
            }
        });
    });
}
