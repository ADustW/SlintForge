// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
struct ForgePlotStreamingQaState {
    int phase=0,frames=0,previous_count=0,paused_count=0;
    std::chrono::steady_clock::time_point started=std::chrono::steady_clock::now();
    std::string csv="elapsed_s,history_points,rendered_points,prepare_ms,capture_ms\n";
};
static void forge_plot_streaming_require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
static void forge_plot_streaming_performance(const slint::ComponentHandle<ProfessionalPlotWindow>& ui,const std::filesystem::path& directory){
    ui->set_streaming(true);ui->set_window_mode(1);ui->get_scene();std::vector<ForgePlotPoint> points;points.reserve(1000000);for(int i=0;i<1000000;++i){ForgePlotPoint pt{};pt.x=i*.001f;pt.y=std::sin(i*.003f)*2+5;points.push_back(pt);}
    forge_plot_streaming_require(ui->invoke_append_points(0,std::make_shared<slint::VectorModel<ForgePlotPoint>>(std::move(points))),"百万点历史导入失败");ui->invoke_flush();forge_plot_streaming_require(ui->get_scene().input_points==1000000,"百万点历史数量错误");const auto warmup=forge_capture_window(ui->window());forge_plot_streaming_require(warmup.ok(),"预热截图失败");
    std::ofstream csv(directory/"incremental-frame-performance.csv");csv<<"history_points,append_ms,prepare_ms,capture_ms,total_ms,rendered_points\n";
    const auto elapsed=[](const auto& start){return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();};
    for(int batch=0;batch<64;++batch){std::vector<ForgePlotPoint> points;for(int i=0;i<32;++i){ForgePlotPoint pt{};pt.x=(1000000+batch*32+i)*.001f;pt.y=std::sin(pt.x*3)*2+5;points.push_back(pt);}const auto start=std::chrono::steady_clock::now();forge_plot_streaming_require(ui->invoke_append_points(0,std::make_shared<slint::VectorModel<ForgePlotPoint>>(std::move(points))),"追加帧失败");const double append_ms=elapsed(start);const auto prepared=std::chrono::steady_clock::now();ui->invoke_flush();const auto scene=ui->get_scene();const double prepare_ms=elapsed(prepared);const auto captured=std::chrono::steady_clock::now();const auto capture=forge_capture_window(ui->window());const double capture_ms=elapsed(captured);forge_plot_streaming_require(capture.ok(),"追加帧截图失败");forge_plot_streaming_require(scene.input_points==1000000+(batch+1)*32&&scene.rendered_points<5000,"增量帧数量或抽样失败");csv<<scene.input_points<<','<<append_ms<<','<<prepare_ms<<','<<capture_ms<<','<<elapsed(start)<<','<<scene.rendered_points<<'\n';}
    ui->set_streaming(false);ui->invoke_reset();ui->get_scene();
}
// 使用真实采样定时器验证连续时间、无限历史、暂停恢复和八位置图例。
static void forge_plot_streaming_start(const slint::ComponentHandle<ProfessionalPlotWindow>& ui,slint::Timer& timer,const std::shared_ptr<std::string>& failure){
    const auto state=std::make_shared<ForgePlotStreamingQaState>();const auto directory=forge_plot_consumer_qa_directory();std::filesystem::create_directories(directory);
    timer.start(slint::TimerMode::Repeated,std::chrono::milliseconds(500),[ui,state,directory,failure](){try{
        if(state->phase==0){
            ForgePlotPoint seed{};seed.x=6;seed.y=14;forge_plot_streaming_require(ui->invoke_append_points(0,std::make_shared<slint::VectorModel<ForgePlotPoint>>(std::vector<ForgePlotPoint>{seed})),"静态曲线追加失败");ui->invoke_flush();forge_plot_streaming_require(ui->get_scene().input_points==13,"静态曲线追加历史被重置");
            ui->set_mode(1);ui->invoke_reset();ui->get_scene();ui->set_mode(0);ui->invoke_reset();forge_plot_streaming_require(ui->get_scene().input_points==12,"静态模式复位失败");
            for(int position=0;position<8;++position)for(int style=0;style<3;++style){ui->set_legend_position_index(position);ui->set_legend_style_index(style);forge_plot_streaming_require(ui->get_scene().status==0,"图例场景失败");const auto capture=forge_capture_window(ui->window());forge_plot_streaming_require(capture.ok(),"图例截图失败");const auto error=forge_save_capture_png(*capture.pixels,directory/("legend-"+std::to_string(position)+"-"+std::to_string(style)+".png"));forge_plot_streaming_require(error.empty(),"图例截图保存失败");}
            for(int style=0;style<6;++style){ui->set_line_style_index(style);forge_plot_streaming_require(ui->get_scene().status==0,"线型场景失败");const auto capture=forge_capture_window(ui->window());forge_plot_streaming_require(capture.ok(),"线型截图失败");const auto error=forge_save_capture_png(*capture.pixels,directory/("line-style-"+std::to_string(style)+".png"));forge_plot_streaming_require(error.empty(),"线型截图保存失败");}ui->set_line_style_index(0);
            for(int ticker=0;ticker<5;++ticker){ui->set_axis_ticker_index(ticker);forge_plot_streaming_require(ui->get_scene().status==0,"刻度场景失败");const auto capture=forge_capture_window(ui->window());forge_plot_streaming_require(capture.ok(),"刻度截图失败");const auto error=forge_save_capture_png(*capture.pixels,directory/("axis-ticker-"+std::to_string(ticker)+".png"));forge_plot_streaming_require(error.empty(),"刻度截图保存失败");}ui->set_axis_ticker_index(0);
            forge_plot_streaming_performance(ui,directory);
            ui->set_legend_position_index(4);ui->set_legend_style_index(1);ui->set_streaming(true);ui->set_live(true);ui->set_window_mode(1);ui->get_scene();ui->invoke_sample_running(true);state->phase=1;return;
        }
        const auto start=std::chrono::steady_clock::now();ui->invoke_flush();const auto scene=ui->get_scene();const double prepare_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();forge_plot_streaming_require(scene.status<2,"实时场景失败");forge_plot_streaming_require(scene.input_points>=state->previous_count,"历史数量减少");state->previous_count=scene.input_points;
        const auto capture_start=std::chrono::steady_clock::now();const auto capture=forge_capture_window(ui->window());forge_plot_streaming_require(capture.ok(),"实时截图失败");const double capture_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-capture_start).count();
        state->csv+=std::to_string(std::chrono::duration<double>(std::chrono::steady_clock::now()-state->started).count())+','+std::to_string(scene.input_points)+','+std::to_string(scene.rendered_points)+','+std::to_string(prepare_ms)+','+std::to_string(capture_ms)+'\n';++state->frames;
        if(state->phase==1&&state->frames>=44){forge_plot_streaming_require(scene.input_points>4000&&scene.history_range.maximum>20,"采样未跨越旧缓冲边界");forge_plot_streaming_require(std::abs(scene.window_range.maximum-scene.window_range.minimum-10)<.001,"窗口跨度错误");forge_plot_streaming_require(ui->global<ForgePlotServices>().invoke_measure(scene.token,0,.01f).index==0,"无限历史丢失首点");ui->invoke_sample_running(false);ui->set_live(false);state->paused_count=scene.input_points;state->phase=2;state->frames=0;const auto error=forge_save_capture_png(*capture.pixels,directory/"live-before-pause.png");forge_plot_streaming_require(error.empty(),"采样截图保存失败");return;}
        if(state->phase==2){forge_plot_streaming_require(scene.input_points==state->paused_count,"暂停后仍追加数据");if(state->frames>=2){ui->set_live(true);ui->invoke_sample_running(true);state->phase=3;state->frames=0;}return;}
        if(state->phase==3&&state->frames>=4){forge_plot_streaming_require(scene.input_points>state->paused_count,"恢复采样失败");const int count=scene.input_points/2;const auto sample=ui->global<ForgePlotServices>().invoke_measure(scene.token,0,count*.01f);forge_plot_streaming_require(sample.index==count-1&&std::abs(sample.x-count*.01f)<.001,"恢复后时间重复或不连续");
            ui->invoke_sample_running(false);ui->set_live(false);ui->invoke_scroll_history(0);forge_plot_streaming_require(ui->get_scene().window_range.minimum<.02,"历史滚动失败");ui->set_measure(true);const auto image=forge_capture_window(ui->window());forge_plot_streaming_require(image.ok(),"测量截图失败");const auto error=forge_save_capture_png(*image.pixels,directory/"history-measure.png");forge_plot_streaming_require(error.empty(),"测量截图保存失败");ui->invoke_reset();forge_plot_streaming_require(ui->get_scene().window_range.maximum>20,"恢复跟随失败");
            std::ofstream csv(directory/"streaming-window-performance.csv");csv<<state->csv;csv.close();std::cout<<"PLOT_STREAMING_WINDOW_OK host=cpp history="<<scene.input_points<<" legends=24 pause_resume=passed\n";slint::quit_event_loop();
        }
    }catch(const std::exception& error){*failure=error.what();slint::quit_event_loop();}});
}
