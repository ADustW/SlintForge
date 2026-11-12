// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <sstream>

// 每个拖动步骤之间追加数据，覆盖场景重建、顶部标签与两条光标交叉。
static void forge_plot_drag_start(const slint::ComponentHandle<ProfessionalPlotWindow>& ui,slint::Timer& timer,const std::shared_ptr<std::string>& failure){
    const auto directory=forge_plot_consumer_qa_directory();std::filesystem::create_directories(directory);const auto phase=std::make_shared<int>(0);const auto start_x=std::make_shared<float>(0.f),y=std::make_shared<float>(0.f);const auto csv=std::make_shared<std::ostringstream>();*csv<<"cursor,step,maximum,target_pixel,actual_pixel,actual_value,other_pixel\n";
    timer.start(slint::TimerMode::Repeated,std::chrono::milliseconds(100),[ui,directory,phase,start_x,y,csv,failure](){try{
        const auto require=[](bool value){if(!value)throw std::runtime_error("实时光标拖动验证失败");};const auto save=[&](const std::string& name){const auto image=forge_capture_window(ui->window());if(!image.ok())throw std::runtime_error(image.error);const auto error=forge_save_capture_png(*image.pixels,directory/(name+".png"));if(!error.empty())throw std::runtime_error(error);};
        if(*phase==0){ui->set_dark_mode(true);ui->set_mode(0);ui->set_streaming(true);ui->set_window_mode(2);ui->set_measure(false);ui->set_measurement_placement_index(0);ui->set_measurement_position_index(5);}
        else if(*phase==1){for(int id=0;id<2;++id){std::vector<ForgePlotPoint> points;for(int i=0;i<=13000;++i){ForgePlotPoint p{};p.x=i*.01f;p.y=id==0?std::sin(p.x*3)*2+5:std::sin(p.x*1.5f)+2;points.push_back(p);}require(ui->invoke_append_points(id,std::make_shared<slint::VectorModel<ForgePlotPoint>>(std::move(points))));}ui->invoke_flush();ui->set_measure(true);}
        else{const int cursor=(*phase-2)/11,stage=(*phase-2)%11;
            if(stage==0){const auto scene=ui->get_scene();const auto rect=scene.rects->row_data(0).value();*start_x=cursor==0?ui->get_cursor_a_pixel():ui->get_cursor_b_pixel();*y=rect.y+(cursor==0?rect.height*.4f:9);save("cursor-"+std::to_string(cursor)+"-before");const slint::LogicalPosition target({ui->get_plot_offset_x()+*start_x,ui->get_plot_offset_y()+*y});ui->window().dispatch_pointer_move_event(target);ui->window().dispatch_pointer_press_event(target,slint::PointerEventButton::Left);}
            else if(stage%2==1){const float time=ui->get_scene().window_range.maximum+.1f;for(int id=0;id<2;++id){ForgePlotPoint p{};p.x=time;p.y=id==0?std::sin(time*3)*2+5:std::sin(time*1.5f)+2;require(ui->invoke_append_points(id,std::make_shared<slint::VectorModel<ForgePlotPoint>>(std::vector<ForgePlotPoint>{p})));}ui->invoke_flush();}
            else{const int step=stage/2;const float pixel_target=*start_x+step*(cursor==0?80.f:-70.f);const slint::LogicalPosition target({ui->get_plot_offset_x()+pixel_target,ui->get_plot_offset_y()+*y});ui->window().dispatch_pointer_move_event(target);const auto scene=ui->get_scene();const float actual=cursor==0?ui->get_cursor_a():ui->get_cursor_b(),pixel=cursor==0?ui->get_cursor_a_pixel():ui->get_cursor_b_pixel(),other=cursor==0?ui->get_cursor_b_pixel():ui->get_cursor_a_pixel();const auto expected=ui->global<ForgePlotServices>().invoke_coordinate(scene.token,scene.measurement_axis,pixel_target,true);
                require(std::abs(actual-expected)<.002f&&std::abs(pixel-pixel_target)<.02f&&std::abs(scene.window_range.maximum-(130+cursor*.5f+step*.1f))<.002f);*csv<<cursor<<','<<step<<','<<scene.window_range.maximum<<','<<pixel_target<<','<<pixel<<','<<actual<<','<<other<<'\n';
                if(stage==10){ui->window().dispatch_pointer_release_event(target,slint::PointerEventButton::Left);save("cursor-"+std::to_string(cursor)+"-after");if(cursor==1){std::ofstream(directory/"drag.csv")<<csv->str();std::cout<<"PLOT_CURSOR_DRAG_OK host=cpp cursors=2 steps=10 updates=10\n";slint::quit_event_loop();}}
            }
        }++*phase;
    }catch(const std::exception& error){*failure=error.what();slint::quit_event_loop();}});
}
