// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <sstream>

// 实际展开并点击菜单；用累计采样验证超过 130 秒后的光标位置。
static void forge_plot_usability_start(const slint::ComponentHandle<ProfessionalPlotWindow>& ui,slint::Timer& timer,const std::shared_ptr<std::string>& failure) {
    const auto directory=forge_plot_consumer_qa_directory();std::filesystem::create_directories(directory);
    const auto phase=std::make_shared<int>(0);const auto fraction=std::make_shared<float>(0.f);const auto csv=std::make_shared<std::ostringstream>();
    *csv<<"time,minimum,maximum,cursor_a,cursor_b,pixel_a,pixel_b,rect_x,rect_width,measure_width,measure_height,input_points\n";
    timer.start(slint::TimerMode::Repeated,std::chrono::milliseconds(150),[ui,directory,phase,fraction,csv,failure](){try {
        const auto save=[&](const std::string& name){const auto capture=forge_capture_window(ui->window());if(!capture.ok())throw std::runtime_error(capture.error);const auto error=forge_save_capture_png(*capture.pixels,directory/(name+".png"));if(!error.empty())throw std::runtime_error(error);};
        const auto require=[](bool value){if(!value)throw std::runtime_error("菜单或实时光标验证失败");};
        const auto click=[&](bool legend,int index){require(legend?ui->get_legend_menu_open():ui->get_measurement_menu_open());const float x=legend?ui->get_legend_picker_x():ui->get_measurement_picker_x(),y=legend?ui->get_legend_picker_y():ui->get_measurement_picker_y();const slint::LogicalPosition target({x+30,y+42+index*32+16});ui->window().dispatch_pointer_move_event(target);ui->window().dispatch_pointer_press_event(target,slint::PointerEventButton::Left);ui->window().dispatch_pointer_release_event(target,slint::PointerEventButton::Left);};
        const auto append=[&](int first,int last){for(int id=0;id<2;++id){std::vector<ForgePlotPoint> points;for(int i=first;i<=last;++i){ForgePlotPoint p{};p.x=i*.01f;p.y=id==0?std::sin(p.x*3)*2+5:std::sin(p.x*1.5f)+2;points.push_back(p);}require(ui->invoke_append_points(id,std::make_shared<slint::VectorModel<ForgePlotPoint>>(std::move(points))));}ui->invoke_flush();};
        switch(*phase) {
        case 0:ui->set_dark_mode(true);ui->set_measure(true);break;
        case 1:ui->invoke_show_legend_positions();break;
        case 2:save("legend-menu-left-top");click(true,6);break;
        case 3:require(ui->get_legend_position_index()==6);save("legend-left-top");ui->invoke_show_legend_positions();break;
        case 4:save("legend-menu-left-bottom");click(true,7);break;
        case 5:require(ui->get_legend_position_index()==7);save("legend-left-bottom");ui->invoke_show_measurement_positions();break;
        case 6:save("measure-menu-left-top");click(false,6);break;
        case 7:require(ui->get_measurement_position_index()==6);save("measure-left-top");ui->invoke_show_measurement_positions();break;
        case 8:save("measure-menu-left-bottom");click(false,7);break;
        case 9:require(ui->get_measurement_position_index()==7);require(!ui->get_legend_menu_open()&&!ui->get_measurement_menu_open());save("measure-left-bottom");ui->set_measure(false);ui->set_mode(0);ui->invoke_reset();ui->set_streaming(true);ui->set_window_mode(2);ui->set_legend_placement_index(1);ui->set_legend_position_index(1);ui->set_measurement_placement_index(0);ui->set_measurement_position_index(5);break;
        case 10:append(0,13000);ui->set_measure(true);break;
        case 11:case 12:case 13:{const float times[]={130,140,170};const float time=times[*phase-11];const auto scene=ui->get_scene();const auto rect=scene.rects->row_data(0).value();const auto range=scene.measurement_range;
            require(scene.status==0&&std::abs(range.maximum-time)<.001f&&std::abs(range.maximum-range.minimum-10)<.001f);
            for(const auto entry:{std::pair(ui->get_cursor_a(),ui->get_cursor_a_pixel()),std::pair(ui->get_cursor_b(),ui->get_cursor_b_pixel())})require(entry.first>range.minimum&&entry.first<range.maximum&&entry.second>rect.x&&entry.second<rect.x+rect.width);
            const float current=(ui->get_cursor_a()-range.minimum)/(range.maximum-range.minimum);if(*phase==11)*fraction=current;else require(std::abs(current-*fraction)<.001f);
            require(scene.measurement_rect.width<=260&&scene.measurement_rect.height<=80&&scene.input_points==(static_cast<int>(time)*100+1)*2);
            *csv<<time<<','<<range.minimum<<','<<range.maximum<<','<<ui->get_cursor_a()<<','<<ui->get_cursor_b()<<','<<ui->get_cursor_a_pixel()<<','<<ui->get_cursor_b_pixel()<<','<<rect.x<<','<<rect.width<<','<<scene.measurement_rect.width<<','<<scene.measurement_rect.height<<','<<scene.input_points<<'\n';save("live-"+std::to_string(static_cast<int>(time)));
            if(*phase==11)append(13001,14000);else if(*phase==12)append(14001,17000);else{std::ofstream(directory/"usability.csv")<<csv->str();std::cout<<"PLOT_USABILITY_OK host=cpp menu_clicks=4 time=170 points="<<scene.input_points<<'\n';slint::quit_event_loop();}break;}
        default:throw std::runtime_error("无效验证阶段");
        }
        ++*phase;
    }catch(const std::exception& error){*failure=error.what();slint::quit_event_loop();}});
}
