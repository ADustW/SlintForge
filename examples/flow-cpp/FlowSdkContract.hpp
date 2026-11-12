// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "ForgeFlow.hpp"
#include <iostream>

// 只引用公共 SDK，在安装后的独立消费项目中运行相同契约检查。
static int forge_flow_sdk_contract() {
    try {
        ForgeFlowDocument document;
        document.apply_checked({{"op","batch"},{"commands",nlohmann::json::array({{{"op","insert"},{"node",{{"id","device-a"},{"kind","external-card"},{"data",{{"name","外部设备"}}}}}},{{"op","insert"},{"node",{{"id","device-b"},{"x",400}}}},{{"op","connect"},{"id","device-a"},{"port","out"},{"target","device-b"},{"target_port","in"}}})}});
        if(document.downstream("device-a").size()!=1||document.ports("device-b")[0]["connection_count"]!=1)throw std::runtime_error("图查询失败");
        const auto saved=document.export_json();document.undo();if(!document.query("nodes").empty())throw std::runtime_error("事务历史失败");document.redo();
        if(saved!=document.export_json())throw std::runtime_error("重做内容不一致");
        document.set_before_edit([](const auto& a){return a.value("op",std::string())=="delete"?std::string("设备受保护"):std::string();});
        document.select("device-a");if(document.remove_selected()["ok"].get<bool>()||saved!=document.export_json())throw std::runtime_error("业务审核失败");
        ForgeFlowDocument other;other.import_json(saved);if(saved!=other.export_json())throw std::runtime_error("序列化失败");
        document.select("device-b",true);if(!document.group_selection("公共分组")["ok"].get<bool>())throw std::runtime_error("分组接口失败");
        const auto group=document.query("selection")["nodes"][0].get<std::string>();
        if(document.node_world("device-b")["x"]!=400)throw std::runtime_error("分组改变了世界坐标");
        document.copy_selection();const auto fragment=document.selection_fragment();
        ForgeFlowDocument pasted;if(!pasted.paste_clipboard(100,200)["ok"].get<bool>()||pasted.query("nodes").size()!=3||pasted.query("edges").size()!=1)throw std::runtime_error("跨文档剪贴板失败");
        const auto saved_paste=pasted.export_json();pasted.undo();if(!pasted.query("nodes").empty())throw std::runtime_error("粘贴撤销失败");pasted.redo();if(pasted.export_json()!=saved_paste)throw std::runtime_error("粘贴重做失败");
        if(fragment["nodes"].size()!=3||!document.toggle_collapse(group)["ok"].get<bool>()||!document.ungroup(group)["ok"].get<bool>())throw std::runtime_error("分组组合接口失败");
        document.select("device-a");document.select("device-b",true);
        if(!document.align_selected("top")["ok"].get<bool>()||!document.layout_graph({{"algorithm","layered"},{"direction","TB"}})["ok"].get<bool>())throw std::runtime_error("布局或对齐公共接口失败");
        if(document.search("DEVICE-A").size()!=1||document.bounds(false,"device-a")["width"].get<double>()<=0||!document.reveal("device-a")["ok"].get<bool>())throw std::runtime_error("导航公共接口失败");
        document.select("device-b",true);const auto before_properties=document.export_json();
        if(!document.patch_selected_nodes({{"layout_fixed",true}})["ok"].get<bool>()||document.query("node","device-b")["layout_fixed"]!=true)throw std::runtime_error("批量属性公共接口失败");
        document.undo();if(document.export_json()!=before_properties)throw std::runtime_error("批量属性撤销失败");
        const auto edge=document.query("edges")[0]["id"].get<std::string>();
        const auto before_style=document.export_json();
        if(!document.update_edge_appearance(edge,{{"marker_start","circle"},{"marker_end","diamond"},{"animated",true},{"animation_speed",90.},{"curvature",.65},{"layer",8}})["ok"].get<bool>())throw std::runtime_error("高级连线公共接口失败");
        auto styled=nlohmann::json::parse(document.snapshot())["edges"][0];
        if(styled["start_marker"]!="circle"||styled["motion_points"].size()!=65||styled["layer"]!=8)throw std::runtime_error("高级连线视图契约不一致");
        document.undo();if(document.export_json()!=before_style)throw std::runtime_error("高级连线撤销失败");
        document.mark_saved();document.patch_node("device-a",{{"title","导出 <卡片>"}});
        if(document.document_status()["dirty"]!=true)throw std::runtime_error("未保存状态接口失败");
        document.undo();if(document.document_status()["dirty"]!=false)throw std::runtime_error("保存点撤销接口失败");
        const auto exported=document.export_svg(false,{{"theme","dark"}},[](const nlohmann::json& node){return nlohmann::json::array({{{"type","text"},{"text",node["id"].get<std::string>()+" <SDK>"}}});});
        if(exported.find("&lt;SDK&gt;")==std::string::npos||document.document_status()["dirty"]!=false)throw std::runtime_error("图片导出公共接口失败");
        document.select("device-a");const auto keyboard_before=document.export_json();
        if(!document.nudge_selected(1,0)["ok"].get<bool>())throw std::runtime_error("键盘移动公共接口失败");
        document.undo();if(document.export_json()!=keyboard_before)throw std::runtime_error("键盘移动公共撤销失败");
        if(!document.select_next()["ok"].get<bool>()||document.query("selection")["nodes"][0]!="device-b")throw std::runtime_error("键盘导航公共接口失败");
        other.configure_history(2,4096);
        for(int i=0;i<4;++i)other.patch_node("device-a",{{"title",std::to_string(i)}});
        auto history=other.history_status();
        if(history["undo_count"]!=2||history["history_bytes"].get<std::size_t>()>4096)throw std::runtime_error("历史预算公共接口失败");
        other.undo();other.undo();other.undo();
        if(other.query("node","device-a")["title"]!="1")throw std::runtime_error("预算裁剪后撤销越界");
        const auto history_bytes=other.history_status()["history_bytes"];other.redo();other.redo();
        if(other.query("node","device-a")["title"]!="3"||other.history_status()["history_bytes"]!=history_bytes)throw std::runtime_error("重做未复用历史存储");
        other.configure_history(100,1024);other.update_data("device-a",{{"blob",std::string(8192,'x')}});
        if(other.history_status()["history_barriers"]!=1||other.history_status()["history_bytes"]!=0)throw std::runtime_error("超大编辑历史边界失败");
        std::cout<<"FLOW_HISTORY_SDK_OK host=cpp budget=public undo_redo=bounded oversized=barrier\n";
        std::cout<<"FLOW_SDK_CONTRACT_OK host=cpp batch=atomic history=one_step queries=public callbacks=veto documents=isolated groups=world_coordinates clipboard=cross_document layout=layered alignment=top navigation=search_bounds_reveal edge_presentation=markers_motion_curvature_layer\n";
        return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
