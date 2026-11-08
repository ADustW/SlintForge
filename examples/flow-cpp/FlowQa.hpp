// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "ForgeCapture.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
#include <deque>

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
static void forge_flow_qa_step(const slint::ComponentHandle<ForgeFlowWindow>& ui,const nlohmann::json& step,std::deque<std::pair<std::string,std::string>>& events) {
    auto& window=ui->window();const auto kind=step.at("kind").get<std::string>();
    const slint::LogicalPosition p{{step.value("x",0.f),step.value("y",0.f)}};
    const auto button=step.value("button","")=="middle"?slint::PointerEventButton::Middle:step.value("button","")=="right"?slint::PointerEventButton::Right:slint::PointerEventButton::Left;
    if(kind=="view")ui->invoke_set_view(p.x,p.y,step.at("zoom"));
    else if(kind=="edit"){if(step.value("op","")=="demo")ui->set_variant(step.value("number",0));ui->invoke_edit(forge_flow_qa_action(step));forge_flow_qa_require(ui->get_state().ok,std::string(ui->get_state().error));}
    else if(kind=="block")ui->set_blocked_target(step.value("target","").c_str());
    else if(kind=="block-edit")ui->set_blocked_operation(step.value("op","").c_str());
    else if(kind=="event")forge_flow_qa_require(std::string(ui->get_last_interaction())==step.at("value").get<std::string>()&&std::string(ui->get_last_interaction_id())==step.value("id",""),"事件不符: "+std::string(ui->get_last_interaction()));
    else if(kind=="clear-events")events.clear();
    else if(kind=="observed-event")forge_flow_qa_require(std::find(events.begin(),events.end(),std::make_pair(step.at("value").get<std::string>(),step.value("id","")))!=events.end(),"未收到事件: "+step.at("value").get<std::string>());
    else if(kind=="preview")forge_flow_qa_require(std::string(ui->get_connection_path()).find(step.at("contains").get<std::string>())!=std::string::npos,"连接预览线型不符");
    else if(kind=="error")forge_flow_qa_require(std::string(ui->get_flow_error()).find(step.at("contains").get<std::string>())!=std::string::npos,"缺少预期错误");
    else if(kind=="readonly")ui->set_read_only(step.at("value"));
    else if(kind=="animations")ui->set_animations_enabled(step.at("value"));
    else if(kind=="file-path"||kind=="image-path") {
#ifdef _WIN32
        const wchar_t* folder=_wgetenv(L"SLINTFORGE_FLOW_QA_DIR");const auto base=std::filesystem::path(folder?folder:L"build/flow-qa/cpp");
#else
        const char* folder=std::getenv("SLINTFORGE_FLOW_QA_DIR");const auto base=std::filesystem::u8path(folder?folder:"build/flow-qa/cpp");
#endif
        const auto path=(base/std::filesystem::u8path(step.at("name").get<std::string>())).u8string();
        const slint::SharedString value(reinterpret_cast<const char*>(path.c_str()));
        if(kind=="file-path")ui->set_file_path(value);else ui->set_image_path(value);
    }
    else if(kind=="save")ui->invoke_save_document();
    else if(kind=="focus-canvas")ui->invoke_focus_canvas();
    else if(kind=="dispose")ui->invoke_dispose();
    else if(kind=="reopen")ui->invoke_reopen();
    else if(kind=="controllers")forge_flow_qa_require(ui->global<ForgeFlowServices>().invoke_active_documents()==step.at("value"),"文档控制器数量不符");
    else if(kind=="lifecycle-marker")std::cout<<"FLOW_LIFECYCLE_PHASE "<<step.at("name").get<std::string>()<<std::endl;
    else if(kind=="lifecycle") {
        auto& service=ui->global<ForgeFlowServices>();const auto active=service.invoke_active_documents();const auto preserved=service.invoke_export_json(ui->get_state().token);
        for(int cycle=0;cycle<64;++cycle){const auto state=service.invoke_create("graph");service.invoke_execute(state.token,forge_flow_qa_action({{"op","demo"},{"number",cycle%4},{"count",12}}));service.invoke_release(state.token);service.invoke_release(state.token);forge_flow_qa_require(!service.invoke_execute(state.token,forge_flow_qa_action({{"op","refresh"}})).ok,"已释放句柄仍可编辑");}
        forge_flow_qa_require(service.invoke_active_documents()==active&&service.invoke_export_json(ui->get_state().token)==preserved,"多实例释放泄漏控制器或影响当前文档");
    }
    else if(kind=="export")ui->invoke_export_image(step.value("selected",false));
    else if(kind=="ui-state") {
        const nlohmann::json state={{"dirty",ui->get_state().dirty},{"save_count",ui->get_save_count()},{"export_count",ui->get_export_count()}};
        for(const auto& [key,value]:step.at("value").items())forge_flow_qa_require(state.at(key)==value,"界面状态不符: "+state.dump());
    }
    else if(kind=="press")window.dispatch_pointer_press_event(p,button);
    else if(kind=="release")window.dispatch_pointer_release_event(p,button);
    else if(kind=="move")window.dispatch_pointer_move_event(p);
    else if(kind=="click"){window.dispatch_pointer_move_event(p);window.dispatch_pointer_press_event(p,button);window.dispatch_pointer_release_event(p,button);}
    else if(kind=="double-click"){window.dispatch_pointer_move_event(p);for(int i=0;i<2;++i){window.dispatch_pointer_press_event(p,button);window.dispatch_pointer_release_event(p,button);}}
    else if(kind=="wheel"){window.dispatch_pointer_move_event(p);window.dispatch_pointer_scroll_event(p,step.value("delta_x",0.f),step.at("delta"));}
    // 固定 Slint 版本的验收入口；进入运行时手势分发，不直接调用 Canvas 的缩放函数。
    else if(kind=="pinch") {
        const auto name=step.at("phase").get<std::string>();
        const auto phase=name=="start"?slint::cbindgen_private::types::TouchPhase::Started:name=="move"?slint::cbindgen_private::types::TouchPhase::Moved:name=="end"?slint::cbindgen_private::types::TouchPhase::Ended:slint::cbindgen_private::types::TouchPhase::Cancelled;
        window.window_handle().dispatch_pointer_event(slint::cbindgen_private::BackendMouseEvent::PinchGesture({p.x,p.y},step.value("delta",0.f),phase));
    }
    else if(kind=="key"||kind=="key-down"||kind=="key-up"){const slint::SharedString text(step.at("text").get<std::string>().c_str());if(kind!="key-up")window.dispatch_key_press_event(text);if(kind!="key-down")window.dispatch_key_release_event(text);}
    else if(kind=="theme")ui->set_dark(step.at("dark"));
    // 显式转换应用缩放，避免系统 DPI 与 SLINT_SCALE_FACTOR 不同导致验收落点错位。
    else if(kind=="size")window.set_size(slint::PhysicalSize{{static_cast<uint32_t>(std::lround(step.at("width").get<float>()*window.scale_factor())),static_cast<uint32_t>(std::lround(step.at("height").get<float>()*window.scale_factor()))}});
    else if(kind=="fit")ui->invoke_fit();
    else if(kind=="snapshot")forge_flow_qa_screenshot(ui,step.at("name"));
    else if(kind=="expect-range") {
        const auto doc=nlohmann::json::parse(std::string(ui->invoke_document_json()));const double value=doc.at(nlohmann::json::json_pointer(step.at("path"))).get<double>();
        forge_flow_qa_require(value>=step.at("min").get<double>()&&value<=step.at("max").get<double>(),step.at("path").get<std::string>()+" 超出范围: "+std::to_string(value));
    }
    else if(kind=="expect"||kind=="query-expect") {
        const auto doc=nlohmann::json::parse(kind=="query-expect"?std::string(ui->global<ForgeFlowServices>().invoke_query(ui->get_state().token,step.at("request").dump().c_str())):std::string(ui->invoke_document_json()));
        if(kind=="query-expect")forge_flow_qa_require(doc.value("ok",false),doc.value("error",std::string("查询失败")));
        const auto& actual=doc.at(nlohmann::json::json_pointer(step.at("path")));
        const auto& expected=step.at("value");forge_flow_qa_require(actual.is_number()&&expected.is_number()?std::abs(actual.get<double>()-expected.get<double>())<0.02:actual==expected,step.at("path").get<std::string>()+" expected="+expected.dump()+" actual="+actual.dump());
    } else if(kind=="count")forge_flow_qa_require(ui->get_state().node_count==step.at("nodes")&&ui->get_state().edge_count==step.at("edges"),"节点或连线数量不符: "+std::to_string(ui->get_state().node_count)+"/"+std::to_string(ui->get_state().edge_count)+" "+std::string(ui->get_state().error));
    else if(kind=="save-load") {
        auto& services=ui->global<ForgeFlowServices>();const auto token=ui->get_state().token;const auto before=services.invoke_export_json(token);
        const auto saved=services.invoke_save_file(token,"build/flow-qa/cpp-roundtrip.json");forge_flow_qa_require(saved.empty(),std::string(saved));
        auto load=services.invoke_load_file(token,"build/flow-qa/cpp-roundtrip.json");forge_flow_qa_require(load.ok&&services.invoke_export_json(token)==before,"文件保存加载不一致");
    } else throw std::runtime_error("未知验收步骤: "+kind);
    if(std::getenv("SLINTFORGE_FLOW_QA_TRACE"))std::cout<<"FLOW_TRACE "<<step.dump()<<" event="<<std::string(ui->get_last_interaction())<<" error="<<std::string(ui->get_flow_error())<<'\n';
}
