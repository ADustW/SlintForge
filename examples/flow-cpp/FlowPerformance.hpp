// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "FlowQa.hpp"
#include <algorithm>

struct ForgeFlowPerformance {
    bool active=false, pending=false;
    bool node_dragged=false, viewport_panned=false, viewport_zoomed=false;
    std::chrono::steady_clock::time_point render, input, last;
    std::vector<double> render_ms, frame_ms, feedback_ms, command_ms;
    int phase=0;
    std::vector<double> phase_commands[3],phase_frames[3];
};
static double forge_flow_elapsed(std::chrono::steady_clock::time_point start) { return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count(); }
static double forge_flow_p95(std::vector<double> values) { if(values.empty())return 0;std::sort(values.begin(),values.end());return values[static_cast<std::size_t>((values.size()-1)*.95)]; }
static void forge_flow_benchmark(const slint::ComponentHandle<ForgeFlowWindow>& ui,slint::Timer& timer,bool complex,const std::shared_ptr<std::string>& failure) {
    auto metrics=std::make_shared<ForgeFlowPerformance>();
    const auto error=ui->window().set_rendering_notifier([metrics](slint::RenderingState state,slint::GraphicsAPI api){
        if(!metrics->active)return;
        if(state==slint::RenderingState::BeforeRendering)metrics->render=std::chrono::steady_clock::now();
        if(state==slint::RenderingState::AfterRendering){
            metrics->render_ms.push_back(forge_flow_elapsed(metrics->render));
            if(metrics->last.time_since_epoch().count()){metrics->frame_ms.push_back(forge_flow_elapsed(metrics->last));metrics->phase_frames[metrics->phase].push_back(forge_flow_elapsed(metrics->last));}
            metrics->last=std::chrono::steady_clock::now();
            if(metrics->pending){metrics->feedback_ms.push_back(forge_flow_elapsed(metrics->input));metrics->pending=false;}
        }
    });
    if(error)throw std::runtime_error("当前渲染器不提供帧回调，无法生成有效性能记录");
    auto frame=std::make_shared<int>(-30);
    timer.start(slint::TimerMode::Repeated,std::chrono::milliseconds(16),[=,&timer](){
        try{
            auto& w=ui->window();const int i=(*frame)++;
            if(i==-30){ui->invoke_edit(forge_flow_qa_action({{"op","demo"},{"number",complex?1:0},{"count",500}}));ui->invoke_set_view(40,40,1);return;}
            if(i<0)return;
            if(i==0){w.dispatch_pointer_move_event(slint::LogicalPosition{{100,156}});w.dispatch_pointer_press_event(slint::LogicalPosition{{100,156}},slint::PointerEventButton::Left);metrics->active=true;return;}
            if(i==101){w.dispatch_pointer_release_event(slint::LogicalPosition{{199,180}},slint::PointerEventButton::Left);const auto doc=nlohmann::json::parse(std::string(ui->invoke_document_json()));metrics->node_dragged=doc["nodes"][0]["x"].get<double>()>90;w.dispatch_pointer_press_event(slint::LogicalPosition{{10,110}},slint::PointerEventButton::Middle);return;}
            if(i==202){w.dispatch_pointer_release_event(slint::LogicalPosition{{109,140}},slint::PointerEventButton::Middle);metrics->viewport_panned=ui->get_state().viewport.x>100;return;}
            if(i>302){
                metrics->active=false;timer.stop();
                const auto p95=forge_flow_p95(metrics->frame_ms),feedback=forge_flow_p95(metrics->feedback_ms);
                const auto size=w.size();
                const bool measured=metrics->frame_ms.size()>=150&&metrics->feedback_ms.size()>=150&&metrics->node_dragged&&metrics->viewport_panned&&metrics->viewport_zoomed;
                nlohmann::json report={{"host","cpp"},{"build","Release"},{"machine",std::getenv("COMPUTERNAME")?std::getenv("COMPUTERNAME"):""},{"renderer",std::getenv("SLINT_BACKEND")?std::getenv("SLINT_BACKEND"):"winit-skia"},{"scenario",complex?"mixed-cards":"standard"},{"nodes",500},{"edges",1000},{"physical_width",size.width},{"physical_height",size.height},{"scale_factor",w.scale_factor()},{"frames",metrics->frame_ms.size()},{"render_p95_ms",measured?nlohmann::json(forge_flow_p95(metrics->render_ms)):nlohmann::json(nullptr)},{"frame_p95_ms",measured?nlohmann::json(p95):nlohmann::json(nullptr)},{"feedback_p95_ms",measured?nlohmann::json(feedback):nlohmann::json(nullptr)},{"command_p95_ms",forge_flow_p95(metrics->command_ms)},{"target_met",measured&&p95<=33&&feedback<=100},{"measurement_complete",measured}};
                report.update({{"node_dragged",metrics->node_dragged},{"viewport_panned",metrics->viewport_panned},{"viewport_zoomed",metrics->viewport_zoomed}});
                const char* phases[]={"drag","pan","zoom"};for(int phase=0;phase<3;++phase)report["phases"][phases[phase]]={{"command_p95_ms",forge_flow_p95(metrics->phase_commands[phase])},{"frame_p95_ms",forge_flow_p95(metrics->phase_frames[phase])}};
                const char* folder=std::getenv("SLINTFORGE_FLOW_QA_DIR");auto path=std::filesystem::u8path(folder?folder:"build/flow-qa/cpp");std::filesystem::create_directories(path);
                std::ofstream(path/(complex?"performance-mixed.json":"performance-standard.json"))<<report.dump(2);
                std::cout<<"FLOW_PERFORMANCE "<<report.dump()<<std::endl;forge_flow_qa_screenshot(ui,complex?"performance-mixed":"performance-standard");
                // 测量完成后，在窗口仍可绘制时释放大图的渲染资源，再结束事件循环。
                ui->invoke_edit(forge_flow_qa_action({{"op","demo"},{"number",0}}));
                timer.start(slint::TimerMode::SingleShot,std::chrono::milliseconds(80),[](){slint::quit_event_loop();});return;
            }
            if(!metrics->pending){metrics->input=std::chrono::steady_clock::now();metrics->pending=true;}
            auto started=std::chrono::steady_clock::now();
            if(i<=100)w.dispatch_pointer_move_event(slint::LogicalPosition{{100.f+i,156.f+(i%25)}});
            else if(i<=201)w.dispatch_pointer_move_event(slint::LogicalPosition{{10.f+i-102,110.f+((i-102)%30)}});
            else {const slint::LogicalPosition p{{1070,600}};w.dispatch_pointer_move_event(p);w.dispatch_pointer_scroll_event(p,0,(i%10<5)?20.f:-20.f);metrics->viewport_zoomed=metrics->viewport_zoomed||std::abs(ui->get_state().viewport.zoom-1)>0.01;}
            metrics->phase=i<=100?0:i<=201?1:2;const auto elapsed=forge_flow_elapsed(started);metrics->command_ms.push_back(elapsed);metrics->phase_commands[metrics->phase].push_back(elapsed);
        }catch(const std::exception& e){*failure=e.what();metrics->active=false;slint::quit_event_loop();}
    });
}
