// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#include "professional-plot.h"
#include "ForgePlot.hpp"
#include "ForgeCapture.hpp"
#include "ForgeRendering.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <fstream>
#include "performance.hpp"
#include "streaming_qa.hpp"
#include "appearance_qa.hpp"
#include "placement_qa.hpp"
#include "usability_qa.hpp"
#include "drag_qa.hpp"

static void forge_plot_consumer_benchmark(const slint::ComponentHandle<ProfessionalPlotWindow>& ui){
    const auto directory=forge_plot_consumer_qa_directory();std::filesystem::create_directories(directory);std::ofstream csv(directory/"window-performance.csv");csv<<"points,model_ms,prepare_ms,capture_ms,pan_prepare_ms,zoom_prepare_ms,zoom_capture_ms,rendered,svg_bytes\n";
    const auto elapsed=[](const auto& start){return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();};
    for(int count:{100000,1000000}){auto start=std::chrono::steady_clock::now();std::vector<ForgePlotPoint> points;points.reserve(count);for(int index=0;index<count;++index){ForgePlotPoint p{};p.x=static_cast<float>(index);p.y=index==count/3?1000:std::sin(static_cast<float>(index)*.003f);points.push_back(p);}ui->set_benchmark_points(std::make_shared<slint::VectorModel<ForgePlotPoint>>(std::move(points)));const double model_ms=elapsed(start);
        start=std::chrono::steady_clock::now();ui->invoke_refresh();const auto scene=ui->get_scene();const double prepare_ms=elapsed(start);if(scene.status!=0||scene.input_points!=count||scene.rendered_points>5000)throw std::runtime_error("大数据场景失败");
        start=std::chrono::steady_clock::now();const auto capture=forge_capture_window(ui->window());const double capture_ms=elapsed(start);if(!capture.ok())throw std::runtime_error(capture.error);const auto error=forge_save_capture_png(*capture.pixels,directory/("benchmark-"+std::to_string(count)+".png"));if(!error.empty())throw std::runtime_error(error);
        start=std::chrono::steady_clock::now();ui->invoke_benchmark_pan(12,0);ui->get_scene();const double pan_ms=elapsed(start);
        start=std::chrono::steady_clock::now();ui->invoke_benchmark_zoom();ui->get_scene();const double zoom_ms=elapsed(start);
        start=std::chrono::steady_clock::now();const auto zoom_capture=forge_capture_window(ui->window());const double zoom_capture_ms=elapsed(start);if(!zoom_capture.ok())throw std::runtime_error(zoom_capture.error);
        csv<<count<<','<<model_ms<<','<<prepare_ms<<','<<capture_ms<<','<<pan_ms<<','<<zoom_ms<<','<<zoom_capture_ms<<','<<scene.rendered_points<<','<<scene.svg_bytes<<'\n';
        std::cout<<"PLOT_WINDOW_PERF host=cpp points="<<count<<" prepare_ms="<<prepare_ms<<" capture_ms="<<capture_ms<<" pan_ms="<<pan_ms<<" zoom_ms="<<zoom_ms<<'\n';ui->set_benchmark_points({});ui->invoke_reset();ui->get_scene();
    }std::cout<<"PLOT_WINDOW_BENCHMARK_OK host=cpp\n";
}

int main(int argc,char** argv) {
    const auto rendering = forge_configure_rendering();
    try {
        auto ui=ProfessionalPlotWindow::create();
        forge_bind_plot_services<ForgePlotServices,ForgePlotScene,ForgePlotAxis,ForgePlotRange,ForgePlotRect,ForgePlotHit,ForgePlotSelection,ForgePlotExportResult>(ui);
        const auto sampler=std::make_shared<slint::Timer>();const slint::ComponentWeakHandle<ProfessionalPlotWindow> weak(ui);const auto clock=std::make_shared<double>(0);
        ui->on_sample_running([sampler,weak,clock](bool running){if(!running){sampler->stop();return;}const auto started=std::chrono::steady_clock::now();const double base=*clock;
            sampler->start(slint::TimerMode::Repeated,std::chrono::milliseconds(33),[clock,started,base,weak](){if(const auto locked=weak.lock()){
                const double target=base+std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();std::vector<ForgePlotPoint> measured,reference;
                while(*clock+.01<=target){*clock+=.01;ForgePlotPoint pt{};pt.x=static_cast<float>(*clock);pt.y=static_cast<float>(std::sin(*clock*3)*2+5);measured.push_back(pt);pt.y=static_cast<float>(std::sin(*clock*1.5)+2);reference.push_back(pt);}
                if(!measured.empty()){(*locked)->invoke_append_points(0,std::make_shared<slint::VectorModel<ForgePlotPoint>>(std::move(measured)));(*locked)->invoke_append_points(1,std::make_shared<slint::VectorModel<ForgePlotPoint>>(std::move(reference)));}
            }});
        });
        slint::Timer smoke;
        auto failure=std::make_shared<std::string>();
        if(argc>1&&std::string(argv[1])=="--cursor-drag-smoke")forge_plot_drag_start(ui,smoke,failure);
        if(argc>1&&std::string(argv[1])=="--usability-smoke")forge_plot_usability_start(ui,smoke,failure);
        if(argc>1&&std::string(argv[1])=="--appearance-smoke")forge_plot_appearance_start(ui,smoke,failure);
        if(argc>1&&std::string(argv[1])=="--placement-smoke")forge_plot_placement_start(ui,smoke,failure);
        if(argc>1&&std::string(argv[1])=="--streaming-smoke")forge_plot_streaming_start(ui,smoke,failure);
        if(argc>1&&std::string(argv[1])=="--benchmark"){smoke.start(slint::TimerMode::SingleShot,std::chrono::milliseconds(100),[ui,failure](){try{forge_plot_consumer_benchmark(ui);}catch(const std::exception& error){*failure=error.what();}slint::quit_event_loop();});}
        if(argc>1&&std::string(argv[1])=="--interaction-benchmark"){smoke.start(slint::TimerMode::SingleShot,std::chrono::milliseconds(100),[ui,failure](){try{forge_plot_consumer_interaction(ui);}catch(const std::exception& error){*failure=error.what();}slint::quit_event_loop();});}
        if(argc>1&&std::string(argv[1])=="--smoke") {
            const auto directory=forge_plot_consumer_qa_directory();std::filesystem::create_directories(directory);const auto mode=std::make_shared<int>(0);
            smoke.start(slint::TimerMode::Repeated,std::chrono::milliseconds(200),[ui,directory,mode,failure](){try{const auto scene=ui->get_scene();if(scene.status!=0)throw std::runtime_error(std::string(scene.error));
                const auto capture=forge_capture_window(ui->window());if(!capture.ok())throw std::runtime_error(capture.error);const auto error=forge_save_capture_png(*capture.pixels,directory/("mode-"+std::to_string(*mode)+".png"));if(!error.empty())throw std::runtime_error(error);
                if(*mode==0)for(const std::string format:{"png","jpeg","bmp","svg","pdf"}){const auto path=directory/(std::u8string(u8"专业图表.")+std::u8string(format.begin(),format.end()));const auto encoded=path.u8string();const auto result=ui->invoke_export_file(slint::SharedString(std::string(encoded.begin(),encoded.end())),format.c_str());if(!result.ok)throw std::runtime_error(std::string(result.error));}
                for(const std::string format:{"svg","pdf"}){const auto path=directory/("mode-"+std::to_string(*mode)+"."+format);const auto result=ui->invoke_export_file(path.string().c_str(),format.c_str());if(!result.ok)throw std::runtime_error(std::string(result.error));}
                std::cout<<"PLOT_CONSUMER_FRAME host=cpp mode="<<*mode<<" points="<<scene.input_points<<" rendered="<<scene.rendered_points<<'\n';
                ui->invoke_benchmark_pan(12,0);ui->invoke_benchmark_zoom();
                if(++*mode==10){std::cout<<"PLOT_CONSUMER_OK host=cpp\n";slint::quit_event_loop();}else ui->set_mode(*mode);
                }catch(const std::exception& error){*failure=error.what();slint::quit_event_loop();}
            });
        }
        ui->run();if(!failure->empty())throw std::runtime_error(*failure);return 0;
    } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
