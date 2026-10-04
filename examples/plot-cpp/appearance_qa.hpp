#pragma once
#include <sstream>

// 明暗主题下遍历全部图例位置和样式，捕获真实窗口与测量覆盖层。
static void forge_plot_appearance_start(const slint::ComponentHandle<ProfessionalPlotWindow>& ui,slint::Timer& timer,const std::shared_ptr<std::string>& failure) {
    const auto directory=forge_plot_consumer_qa_directory();std::filesystem::create_directories(directory);
    const auto phase=std::make_shared<int>(0);const auto manifest=std::make_shared<std::ostringstream>();
    *manifest<<"theme,position,style,rect_x,rect_y,rect_width,rect_height,offset_x,offset_y\n";
    timer.start(slint::TimerMode::Repeated,std::chrono::milliseconds(150),[ui,directory,phase,manifest,failure](){try{
        if(*phase==0){ui->set_dark_mode(true);*phase=1;return;}
        const std::string theme=*phase==1?"dark":"light";
        const auto save=[&](const std::string& name){const auto capture=forge_capture_window(ui->window());if(!capture.ok())throw std::runtime_error(capture.error);const auto error=forge_save_capture_png(*capture.pixels,directory/(name+".png"));if(!error.empty())throw std::runtime_error(error);};
        for(int position=0;position<8;++position)for(int style=0;style<3;++style){
            ui->set_measure(false);ui->set_legend_position_index(position);ui->set_legend_style_index(style);const auto scene=ui->get_scene();if(scene.status!=0)throw std::runtime_error(std::string(scene.error));const auto rect=*scene.rects->row_data(0);
            const auto name=theme+"-legend-"+std::to_string(position)+"-"+std::to_string(style);std::ofstream(directory/(name+".svg"))<<std::string(scene.svg);save(name);
            *manifest<<theme<<','<<position<<','<<style<<','<<rect.x<<','<<rect.y<<','<<rect.width<<','<<rect.height<<','<<ui->get_plot_offset_x()<<','<<ui->get_plot_offset_y()<<'\n';
        }
        ui->set_legend_position_index(4);ui->set_legend_style_index(2);save("measure-"+theme+"-off");ui->set_measure(true);save("measure-"+theme);ui->set_legend_position_index(0);save("measure-"+theme+"-bottom");
        if(*phase==1){ui->set_dark_mode(false);*phase=2;return;}
        std::ofstream(directory/"appearance.csv")<<manifest->str();std::cout<<"PLOT_APPEARANCE_OK host=cpp themes=2 positions=8 styles=3 screenshots=54\n";slint::quit_event_loop();
    }catch(const std::exception& error){*failure=error.what();slint::quit_event_loop();}});
}
