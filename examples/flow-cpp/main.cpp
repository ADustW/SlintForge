#include "flow.h"
#include "ForgeFlowBindings.hpp"
#include "ForgeCapture.hpp"
#include <cstdlib>
#include <iostream>
#include "FlowQa.hpp"
#include "FlowPerformance.hpp"
#include "FlowSdkContract.hpp"
#include "FlowCardExport.hpp"

int main(int argc, char** argv) {
    if(argc>1&&std::string(argv[1])=="--sdk-contract")return forge_flow_sdk_contract();
    if (!std::getenv("SLINT_BACKEND")) _putenv_s("SLINT_BACKEND", "winit-skia");
    if (!std::getenv("WGPU_BACKEND")) _putenv_s("WGPU_BACKEND", "dx12");
    auto ui = ForgeFlowWindow::create();
    forge_bind_flow_services<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState, ForgeFlowServices>(ui);
    ui->global<ForgeFlowServices>().on_export_card([](slint::SharedString node){return slint::SharedString(forge_flow_demo_export_card(std::string(node)).c_str());});
    slint::Timer timer;
    auto failure=std::make_shared<std::string>();
    if(argc > 1 && std::string(argv[1]) == "--benchmark") forge_flow_benchmark(ui,timer,argc>2&&std::string(argv[2])=="--complex",failure);
    if(argc > 1 && std::string(argv[1]) == "--smoke") {
        auto events=std::make_shared<std::deque<std::pair<std::string,std::string>>>();
        ui->on_interaction([events](slint::SharedString name,ForgeFlowAction a){if(events->size()==64)events->pop_front();events->emplace_back(std::string(name),std::string(a.id));});
        auto path=std::filesystem::path(argv[0]).parent_path().parent_path()/"flow-qa.json";
        if(!std::filesystem::exists(path))path=std::filesystem::path(argv[0]).parent_path()/"flow-qa.json";
        // Windows 环境变量采用宽字符，避免中文路径被当前 ANSI 代码页误解为 UTF-8。
#ifdef _WIN32
        if(const wchar_t* scenario=_wgetenv(L"SLINTFORGE_FLOW_QA_SCENARIO"))path=std::filesystem::path(scenario);
#else
        if(const char* scenario=std::getenv("SLINTFORGE_FLOW_QA_SCENARIO"))path=std::filesystem::u8path(scenario);
#endif
        std::ifstream file(path);auto steps=std::make_shared<nlohmann::json>();
        if(!file){std::cerr<<"无法打开 Flow 验收场景文件\n";return 1;}
        try{file>>*steps;}catch(const std::exception& e){std::cerr<<"Flow 验收场景无效："<<e.what()<<'\n';return 1;}
        auto index=std::make_shared<int>(-3);
        timer.start(slint::TimerMode::Repeated,std::chrono::milliseconds(100),[=](){
            if((*index)++<0)return;
            try{
                const auto& step=steps->at(*index-1);
                if(step.at("kind")=="finish"){std::cout<<"FLOW_WINDOW_OK host=cpp steps="<<steps->size()<<'\n';slint::quit_event_loop();return;}
                forge_flow_qa_step(ui,step,*events);
            }catch(const std::exception& e){*failure="步骤 "+std::to_string(*index-1)+": "+e.what();forge_flow_qa_screenshot(ui,"failure");slint::quit_event_loop();}
        });
    }
    if(argc > 1 && std::string(argv[1]) == "--capture") {
        timer.start(slint::TimerMode::SingleShot, std::chrono::milliseconds(700), [=]() {
            const auto capture=forge_capture_window(ui->window());
            std::filesystem::create_directories("build/flow-qa");
            if(capture.ok()) { const auto error=forge_save_capture_png(*capture.pixels,"build/flow-qa/editor-cpp.png"); if(!error.empty())std::cerr<<error<<'\n'; }
            std::cout << std::string(ui->invoke_document_json()) << '\n';
            slint::quit_event_loop();
        });
    }
    if(argc > 1 && std::string(argv[1]) == "--benchmark") {
        // OpenGL 测量保持窗口上下文到组件析构，避免先 hide 后释放绘制缓存。
        ui->show();
        slint::run_event_loop();
    } else {
        ui->run();
    }
    if(!failure->empty()){std::cerr<<*failure<<'\n';return 1;}
}
