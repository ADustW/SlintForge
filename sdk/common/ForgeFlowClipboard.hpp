// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <mutex>

// 只有剪贴板内容在应用内共享；文档、选择和历史仍由每个控制器独立持有。
// 可导出的 JSON 片段也允许宿主接入系统剪贴板或跨进程传递。
static std::mutex forge_flow_clipboard_mutex;
static std::string forge_flow_clipboard_text;

static ForgeFlowJson forge_flow_selection_fragment(ForgeFlowCore& f) {
    const auto roots=forge_flow_selection_roots(f);
    forge_flow_require(!roots.empty(),"请先选择要复制的节点");
    const auto included=forge_flow_descendants(f,roots);
    auto nodes=ForgeFlowJson::array(),edges=ForgeFlowJson::array();
    double left=1e100,top=1e100;
    for(const auto& node:f.document["nodes"])if(included.contains(forge_flow_id(node))) {
        auto copy=node;
        if(roots.contains(forge_flow_id(node))) {
            const auto& world=forge_flow_scene_node(f,node);
            copy["parent"]="";copy["x"]=world["x"];copy["y"]=world["y"];
            left=std::min(left,world["x"].get<double>());top=std::min(top,world["y"].get<double>());
        }
        nodes.push_back(std::move(copy));
    }
    for(const auto& edge:f.document["edges"])if(included.contains(forge_flow_id(edge,"source_node"))&&included.contains(forge_flow_id(edge,"target_node")))edges.push_back(edge);
    return {{"format","forgeflow-fragment"},{"version",1},{"mode",f.document["mode"]},{"origin",{{"x",left},{"y",top}}},{"nodes",nodes},{"edges",edges}};
}

static void forge_flow_copy_selection(ForgeFlowCore& f) {
    const auto text=forge_flow_selection_fragment(f).dump();
    const std::lock_guard<std::mutex> lock(forge_flow_clipboard_mutex);
    forge_flow_clipboard_text=text;
}

static void forge_flow_delete_selection(ForgeFlowCore& f) {
    const auto removed=forge_flow_descendants(f,f.selected_nodes);
    for(const auto& node:f.document["nodes"])if(removed.contains(forge_flow_id(node)))forge_flow_require(node.value("deletable",true),"选择中包含不可删除节点");
    auto delete_edge=[&](const auto& edge){return f.selected_edges.contains(forge_flow_id(edge))||removed.contains(forge_flow_id(edge,"source_node"))||removed.contains(forge_flow_id(edge,"target_node"));};
    for(const auto& edge:f.document["edges"])if(delete_edge(edge))forge_flow_require(edge.value("deletable",true),"选择中包含不可删除连线");
    auto& nodes=f.document["nodes"];nodes.erase(std::remove_if(nodes.begin(),nodes.end(),[&](const auto& n){return removed.contains(forge_flow_id(n));}),nodes.end());
    auto& edges=f.document["edges"];edges.erase(std::remove_if(edges.begin(),edges.end(),delete_edge),edges.end());
    f.selected_nodes.clear();f.selected_edges.clear();forge_flow_reindex(f);
    if(f.document["mode"]=="mindmap")forge_flow_layout(f);
}

static void forge_flow_paste_fragment(ForgeFlowCore& f,const ForgeFlowJson& action) {
    std::string text=action.value("text",std::string());
    if(!action.contains("fragment")&&text.empty()) {
        forge_flow_require(!f.batching,"事务内粘贴必须显式提供片段");
        const std::lock_guard<std::mutex> lock(forge_flow_clipboard_mutex);text=forge_flow_clipboard_text;
    }
    forge_flow_require(action.contains("fragment")||!text.empty(),"剪贴板没有图节点");
    const auto fragment=action.contains("fragment")?action["fragment"]:ForgeFlowJson::parse(text);
    forge_flow_require(fragment.is_object()&&fragment.value("format",std::string())=="forgeflow-fragment"&&fragment.contains("version")&&fragment["version"].is_number_integer()&&fragment["version"]==1,"不支持的剪贴板片段格式");
    forge_flow_require(fragment.value("mode",std::string())==f.document.value("mode",std::string()),"图和思维导图片段不能混合粘贴");
    forge_flow_require(fragment.contains("nodes")&&fragment["nodes"].is_array()&&!fragment["nodes"].empty()&&fragment["nodes"].size()<=10000&&fragment.contains("edges")&&fragment["edges"].is_array()&&fragment["edges"].size()<=100000,"剪贴板节点或连线数量无效");
    const bool tree=f.document["mode"]=="mindmap";
    const auto parent=action.value("target",std::string());
    double parent_x=0,parent_y=0;
    if(!parent.empty()) {
        const auto& node=forge_flow_node(f,parent);
        forge_flow_require(tree||node.value("is_group",false),"粘贴目标必须是分组容器");
        forge_flow_require(!forge_flow_hidden(f,node)&&!node.value("collapsed",false),"请先展开粘贴目标");
        const auto& world=forge_flow_scene_node(f,node);parent_x=world["x"];parent_y=world["y"];
    }
    if(tree&&!f.document["nodes"].empty())forge_flow_require(!parent.empty(),"思维导图粘贴需要父主题");
    const auto origin=fragment.at("origin");const double ox=forge_flow_number(origin,"x"),oy=forge_flow_number(origin,"y");
    const double dx=action.value("flag",false)?forge_flow_number(action,"x")-ox:32;
    const double dy=action.value("flag",false)?forge_flow_number(action,"y")-oy:32;
    std::set<std::string> original_ids,edge_ids,roots;
    auto nodes=ForgeFlowJson::array();
    for(const auto& raw:fragment["nodes"]) {
        auto node=forge_flow_normalize_node(raw,tree);const auto id=forge_flow_id(node);
        forge_flow_require(original_ids.insert(id).second,"剪贴板节点 ID 重复");
        if(forge_flow_id(node,"parent").empty())roots.insert(id);
        nodes.push_back(std::move(node));
    }
    forge_flow_require(!roots.empty(),"剪贴板缺少根节点");
    if(tree&&f.document["nodes"].empty())forge_flow_require(roots.size()==1,"空思维导图只能粘贴一个根分支");
    for(const auto& node:nodes)forge_flow_require(forge_flow_id(node,"parent").empty()||original_ids.contains(forge_flow_id(node,"parent")),"剪贴板父级引用不完整");
    for(const auto& edge:fragment["edges"]) {
        const auto id=forge_flow_id(edge);
        forge_flow_require(!id.empty()&&edge_ids.insert(id).second,"剪贴板连线 ID 为空或重复");
        forge_flow_require(original_ids.contains(forge_flow_id(edge,"source_node"))&&original_ids.contains(forge_flow_id(edge,"target_node")),"剪贴板包含外部连线引用");
    }
    std::map<std::string,std::string> remap;
    auto fresh_id=[&](const char* prefix){std::string id;do{id=forge_flow_new_id(f,prefix);}while(original_ids.contains(id)||edge_ids.contains(id));return id;};
    for(const auto& id:original_ids)remap[id]=fresh_id("node-");
    f.selected_nodes.clear();f.selected_edges.clear();
    for(auto node:nodes) {
        const auto old=forge_flow_id(node),old_parent=forge_flow_id(node,"parent");node["id"]=remap.at(old);
        node["parent"]=old_parent.empty()?parent:remap.at(old_parent);
        if(tree||old_parent.empty()){node["x"]=node["x"].get<double>()+dx-(tree?0:parent_x);node["y"]=node["y"].get<double>()+dy-(tree?0:parent_y);}
        if(roots.contains(old))f.selected_nodes.insert(remap.at(old));
        f.document["nodes"].push_back(std::move(node));
    }
    forge_flow_reindex(f);
    for(auto edge:fragment["edges"]) {
        edge["id"]=fresh_id("edge-");edge["source_node"]=remap.at(forge_flow_id(edge,"source_node"));edge["target_node"]=remap.at(forge_flow_id(edge,"target_node"));
        if(edge.contains("control_points"))for(auto& point:edge["control_points"]){point["x"]=forge_flow_number(point,"x")+dx;point["y"]=forge_flow_number(point,"y")+dy;}
        f.document["edges"].push_back(std::move(edge));
    }
    // 先验证合并后的所有约束，再进行树布局；外层事务负责完整回滚。
    if(tree)forge_flow_tree_edges(f);
    forge_flow_validate(f);
    if(tree)forge_flow_layout(f);
}

static void forge_flow_duplicate_selection(ForgeFlowCore& f) {
    const auto fragment=forge_flow_selection_fragment(f);const auto roots=forge_flow_selection_roots(f);
    std::string parent=forge_flow_id(forge_flow_node(f,*roots.begin()),"parent");
    for(const auto& id:roots)if(forge_flow_id(forge_flow_node(f,id),"parent")!=parent){parent.clear();break;}
    forge_flow_paste_fragment(f,{{"fragment",fragment},{"target",parent}});
}
