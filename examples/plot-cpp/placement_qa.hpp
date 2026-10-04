#pragma once
#include <sstream>

// 验证真实窗口中两个信息框的区域与位置，并保留布局元数据和截图。
static void forge_plot_placement_start(const slint::ComponentHandle<ProfessionalPlotWindow>& ui,slint::Timer& timer,const std::shared_ptr<std::string>& failure) {
    const auto directory=forge_plot_consumer_qa_directory();std::filesystem::create_directories(directory);
    const auto phase=std::make_shared<int>(0);const auto manifest=std::make_shared<std::ostringstream>();
    const auto overlay=std::make_shared<std::ostringstream>();*overlay<<"name,legend_position,measure_position,rect_x,rect_y,rect_width,rect_height,legend_x,legend_y,legend_width,legend_height,measure_x,measure_y,measure_width,measure_height,offset_x,offset_y\n";
    *manifest<<"name,legend_area,measure_area,position,panel,rect_x,rect_y,rect_width,rect_height,legend_x,legend_y,legend_width,legend_height,measure_x,measure_y,measure_width,measure_height,offset_x,offset_y\n";
    timer.start(slint::TimerMode::Repeated,std::chrono::milliseconds(150),[ui,directory,phase,manifest,overlay,failure](){try{
        if(*phase==0){ui->set_dark_mode(true);*phase=1;return;}const std::string theme=*phase==1?"dark":"light";ui->set_measure(true);
        // 左上/左下交叉放置，开关测量框时保持相同波形，验证透明底色。
        ui->set_mode(0);ui->invoke_reset();ui->set_measurement_series(0);ui->set_legend_placement_index(0);ui->set_measurement_placement_index(0);ui->set_legend_style_index(1);
        for(int position:{6,7}){const int measure_position=position==6?7:6;ui->set_legend_position_index(position);ui->set_measurement_position_index(measure_position);ui->set_measure(true);const auto scene=ui->get_scene();if(scene.status!=0)throw std::runtime_error(std::string(scene.error));
            const auto rect=*scene.rects->row_data(0);const auto l=scene.legend_rect,m=scene.measurement_rect;const auto name=theme+"-corner-"+std::to_string(position);std::ofstream(directory/(name+".svg"))<<std::string(scene.svg);
            const auto save=[&](const std::string& suffix){const auto capture=forge_capture_window(ui->window());if(!capture.ok())throw std::runtime_error(capture.error);const auto error=forge_save_capture_png(*capture.pixels,directory/(name+suffix+".png"));if(!error.empty())throw std::runtime_error(error);};
            save("-on");ui->set_measure(false);if(ui->get_scene().svg!=scene.svg)throw std::runtime_error("透明框对比必须使用同一波形");save("-off");
            *overlay<<name<<','<<position<<','<<measure_position<<','<<rect.x<<','<<rect.y<<','<<rect.width<<','<<rect.height<<','<<l.x<<','<<l.y<<','<<l.width<<','<<l.height<<','<<m.x<<','<<m.y<<','<<m.width<<','<<m.height<<','<<ui->get_plot_offset_x()<<','<<ui->get_plot_offset_y()<<'\n';
        }
        ui->set_measure(true);
        for(int mode:{0,9}){ui->set_mode(mode);ui->invoke_reset();ui->set_measurement_series(mode==9?1:0);
            for(int legend_area=0;legend_area<2;++legend_area)for(int measure_area=0;measure_area<2;++measure_area)for(int position=0;position<8;++position){
                if(mode==9&&(legend_area!=1||measure_area!=1||position<2||position>3))continue;
                ui->set_legend_placement_index(legend_area);ui->set_measurement_placement_index(measure_area);ui->set_legend_position_index(position);ui->set_measurement_position_index(position);ui->set_legend_style_index(position%3);
                const auto scene=ui->get_scene();if(scene.status!=0)throw std::runtime_error(std::string(scene.error));if(mode==0&&std::string(scene.svg).find("data:image/png")!=std::string::npos)throw std::runtime_error("默认示例仍有红色图片");
                const auto rect=*scene.rects->row_data(mode==9?1:0);const auto l=scene.legend_rect,m=scene.measurement_rect;if(m.id!=rect.id)throw std::runtime_error("测量框绘图区错误");
                const auto name=theme+"-"+std::to_string(mode)+"-"+std::to_string(legend_area)+"-"+std::to_string(measure_area)+"-"+std::to_string(position);
                std::ofstream(directory/(name+".svg"))<<std::string(scene.svg);const auto capture=forge_capture_window(ui->window());if(!capture.ok())throw std::runtime_error(capture.error);const auto error=forge_save_capture_png(*capture.pixels,directory/(name+".png"));if(!error.empty())throw std::runtime_error(error);
                *manifest<<name<<','<<legend_area<<','<<measure_area<<','<<position<<','<<rect.id<<','<<rect.x<<','<<rect.y<<','<<rect.width<<','<<rect.height<<','<<l.x<<','<<l.y<<','<<l.width<<','<<l.height<<','<<m.x<<','<<m.y<<','<<m.width<<','<<m.height<<','<<ui->get_plot_offset_x()<<','<<ui->get_plot_offset_y()<<'\n';
            }
        }
        if(*phase==1){ui->set_dark_mode(false);*phase=2;return;}
        std::ofstream(directory/"placement.csv")<<manifest->str();std::ofstream(directory/"overlay.csv")<<overlay->str();ui->set_mode(0);ui->invoke_reset();ui->set_measurement_series(0);ui->set_legend_placement_index(1);ui->set_measurement_placement_index(1);ui->set_legend_position_index(0);ui->set_measurement_position_index(0);forge_plot_streaming_performance(ui,directory);
        std::cout<<"PLOT_PLACEMENT_WINDOW_OK host=cpp themes=2 positions=8 placements=4 screenshots=76 transparent_corners=4\n";slint::quit_event_loop();
    }catch(const std::exception& error){*failure=error.what();slint::quit_event_loop();}});
}
