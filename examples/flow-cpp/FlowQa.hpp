#pragma once
#include "ForgeCapture.hpp"
#include <fstream>
#include <iostream>
#include <chrono>

// 两端读取同一场景；鼠标和键盘事件进入真实窗口的命中、焦点和渲染流程。
static ForgeFlowAction forge_flow_qa_action(const nlohmann::json& value) {
    ForgeFlowAction a{};
    a.op=value.value("op","").c_str(); a.id=value.value("id","").c_str(); a.target=value.value("target","").c_str();
    a.port=value.value("port","").c_str();a.target_port=value.value("target_port","").c_str();a.text=value.value("text","").c_str();
    a.x=value.value("x",0.f);a.y=value.value("y",0.f);a.width=value.value("width",0.f);a.height=value.value("height",0.f);a.zoom=value.value("zoom",1.f);
    a.number=value.value("number",0);a.count=value.value("count",0);a.flag=value.value("flag",false);a.additive=value.value("additive",false);return a;
}
static void forge_flow_qa_require(bool ok,const std::string& message) { if(!ok)throw std::runtime_error(message); }
static void forge_flow_qa_screenshot(const slint::ComponentHandle<ForgeFlowWindow>& ui,const std::string& name) {
    const auto capture=forge_capture_window(ui->window());forge_flow_qa_require(capture.ok(),capture.error);
#ifdef _WIN32
    const wchar_t* folder=_wgetenv(L"SLINTFORGE_FLOW_QA_DIR");auto path=std::filesystem::path(folder?folder:L"build/flow-qa/cpp")/(name+".png");
#else
    const char* folder=std::getenv("SLINTFORGE_FLOW_QA_DIR");auto path=std::filesystem::u8path(folder?folder:"build/flow-qa/cpp")/(name+".png");
#endif
    std::filesystem::create_directories(path.parent_path());const auto error=forge_save_capture_png(*capture.pixels,path);forge_flow_qa_require(error.empty(),error);
}
static void forge_flow_qa_step(const slint::ComponentHandle<ForgeFlowWindow>& ui,const nlohmann::json& step) {
    auto& window=ui->window();const auto kind=step.at("kind").get<std::string>();
    const slint::LogicalPosition p{{step.value("x",0.f),step.value("y",0.f)}};
    const auto button=step.value("button","")=="middle"?slint::PointerEventButton::Middle:slint::PointerEventButton::Left;
    if(kind=="view")ui->invoke_set_view(p.x,p.y,step.at("zoom"));
    else if(kind=="edit"){if(step.value("op","")=="demo")ui->set_variant(step.value("number",0));ui->invoke_edit(forge_flow_qa_action(step));forge_flow_qa_require(ui->get_state().ok,std::string(ui->get_state().error));}
    else if(kind=="block")ui->set_blocked_target(step.value("target","").c_str());
    else if(kind=="readonly")ui->set_read_only(step.at("value"));
    else if(kind=="press")window.dispatch_pointer_press_event(p,button);
    else if(kind=="release")window.dispatch_pointer_release_event(p,button);
    else if(kind=="move")window.dispatch_pointer_move_event(p);
    else if(kind=="click"){window.dispatch_pointer_move_event(p);window.dispatch_pointer_press_event(p,button);window.dispatch_pointer_release_event(p,button);}
    else if(kind=="wheel"){window.dispatch_pointer_move_event(p);window.dispatch_pointer_scroll_event(p,0,step.at("delta"));}
    else if(kind=="key"||kind=="key-down"||kind=="key-up"){const slint::SharedString text(step.at("text").get<std::string>().c_str());if(kind!="key-up")window.dispatch_key_press_event(text);if(kind!="key-down")window.dispatch_key_release_event(text);}
    else if(kind=="theme")ui->set_dark(step.at("dark"));
    // 显式转换应用缩放，避免系统 DPI 与 SLINT_SCALE_FACTOR 不同导致验收落点错位。
    else if(kind=="size")window.set_size(slint::PhysicalSize{{static_cast<uint32_t>(std::lround(step.at("width").get<float>()*window.scale_factor())),static_cast<uint32_t>(std::lround(step.at("height").get<float>()*window.scale_factor()))}});
    else if(kind=="fit")ui->invoke_fit();
    else if(kind=="snapshot")forge_flow_qa_screenshot(ui,step.at("name"));
    else if(kind=="expect") {
        const auto doc=nlohmann::json::parse(std::string(ui->invoke_document_json()));const auto& actual=doc.at(nlohmann::json::json_pointer(step.at("path")));
        const auto& expected=step.at("value");forge_flow_qa_require(actual.is_number()&&expected.is_number()?std::abs(actual.get<double>()-expected.get<double>())<0.02:actual==expected,step.at("path").get<std::string>()+" expected="+expected.dump()+" actual="+actual.dump());
    } else if(kind=="count")forge_flow_qa_require(ui->get_state().node_count==step.at("nodes")&&ui->get_state().edge_count==step.at("edges"),"节点或连线数量不符: "+std::to_string(ui->get_state().node_count)+"/"+std::to_string(ui->get_state().edge_count)+" "+std::string(ui->get_state().error));
    else if(kind=="save-load") {
        auto& services=ui->global<ForgeFlowServices>();const auto token=ui->get_state().token;const auto before=services.invoke_export_json(token);
        const auto saved=services.invoke_save_file(token,"build/flow-qa/cpp-roundtrip.json");forge_flow_qa_require(saved.empty(),std::string(saved));
        auto load=services.invoke_load_file(token,"build/flow-qa/cpp-roundtrip.json");forge_flow_qa_require(load.ok&&services.invoke_export_json(token)==before,"文件保存加载不一致");
    }
}
