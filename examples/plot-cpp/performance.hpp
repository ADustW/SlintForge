// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cstdlib>

static std::filesystem::path forge_plot_consumer_qa_directory() {
    if(const auto configured=std::getenv("SLINTFORGE_PLOT_QA_DIR"))return configured;
    return std::filesystem::current_path()/"build/plot-qa/cpp-consumer";
}

static std::shared_ptr<slint::VectorModel<ForgePlotPoint>> forge_plot_consumer_points(int count,int frame) {
    std::vector<ForgePlotPoint> points;points.reserve(count);
    for(int index=0;index<count;++index){ForgePlotPoint point{};point.x=index*.01f;point.y=std::sin((index+frame)*.03f)*2+5;points.push_back(point);}
    return std::make_shared<slint::VectorModel<ForgePlotPoint>>(std::move(points));
}

// 与 Rust 使用相同真实事件和截图，记录完整交互帧的分段耗时。
static void forge_plot_consumer_interaction(const slint::ComponentHandle<ProfessionalPlotWindow>& ui) {
    const auto directory=forge_plot_consumer_qa_directory();std::filesystem::create_directories(directory);
    std::ofstream csv(directory/"interaction-performance.csv");csv<<"points,operation,frame,event_ms,prepare_ms,capture_ms,total_ms\n";
    const auto elapsed=[](const auto& start){return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();};
    for(int count:{12,2000}){
        ui->set_live_points({});if(count>12)ui->set_live_points(forge_plot_consumer_points(count,0));ui->invoke_reset();
        const auto scene=ui->get_scene();if(scene.status!=0)throw std::runtime_error(std::string(scene.error));
        if(!forge_capture_window(ui->window()).ok())throw std::runtime_error("预热截图失败");
        const auto rect=*scene.rects->row_data(0);const float x=ui->get_plot_offset_x()+rect.x+rect.width*.5f,y=ui->get_plot_offset_y()+rect.y+rect.height*.5f;
        for(const std::string operation:{"hover","drag","wheel","live"}){
            std::vector<double> totals;
            if(operation=="drag")ui->window().dispatch_pointer_press_event(slint::LogicalPosition({x,y}),slint::PointerEventButton::Left);
            for(int frame=0;frame<24;++frame){const auto start=std::chrono::steady_clock::now();const slint::LogicalPosition position({x+frame*.8f,y});
                if(operation=="hover"||operation=="drag")ui->window().dispatch_pointer_move_event(position);
                else if(operation=="wheel")ui->window().dispatch_pointer_scroll_event(position,0,frame%2==0?50:-50);
                else ui->set_live_points(forge_plot_consumer_points(count,frame));
                const double event_ms=elapsed(start);const auto prepare_start=std::chrono::steady_clock::now();
                const auto current=ui->get_scene();if(current.status!=0)throw std::runtime_error(std::string(current.error));const double prepare_ms=elapsed(prepare_start);
                const auto capture_start=std::chrono::steady_clock::now();const auto capture=forge_capture_window(ui->window());if(!capture.ok())throw std::runtime_error(capture.error);
                const double capture_ms=elapsed(capture_start),total_ms=elapsed(start);totals.push_back(total_ms);
                csv<<count<<','<<operation<<','<<frame<<','<<event_ms<<','<<prepare_ms<<','<<capture_ms<<','<<total_ms<<'\n';
                if(frame==23){const auto error=forge_save_capture_png(*capture.pixels,directory/("interaction-"+std::to_string(count)+"-"+operation+".png"));if(!error.empty())throw std::runtime_error(error);}
            }
            if(operation=="drag")ui->window().dispatch_pointer_release_event(slint::LogicalPosition({x+23*.8f,y}),slint::PointerEventButton::Left);
            ui->invoke_reset();std::sort(totals.begin(),totals.end());
            std::cout<<"PLOT_INTERACTION_PERF host=cpp points="<<count<<" operation="<<operation<<" median_ms="<<totals[12]<<" p95_ms="<<totals[22]<<'\n';
        }
    }
    if(!csv.good())throw std::runtime_error("性能 CSV 写入失败");
    std::cout<<"PLOT_INTERACTION_BENCHMARK_OK host=cpp\n";
}
