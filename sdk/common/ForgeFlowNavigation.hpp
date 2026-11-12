// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

// 查询使用世界坐标；选区边界包含选中连线，避免只适应节点导致曲线被裁掉。
static ForgeFlowJson forge_flow_query_bounds(ForgeFlowCore& f,const ForgeFlowJson& a) {
    const auto id=forge_flow_id(a);const bool selected=a.value("flag",false);
    bool first=true;double left=0,top=0,right=0,bottom=0;
    const auto include=[&](double x,double y,double r,double b){if(first){left=x;top=y;right=r;bottom=b;first=false;}else{left=std::min(left,x);top=std::min(top,y);right=std::max(right,r);bottom=std::max(bottom,b);}};
    if(!id.empty())forge_flow_node(f,id);
    for(const auto& n:f.document["nodes"])if(!forge_flow_hidden(f,n)&&(id.empty()?(!selected||f.selected_nodes.contains(forge_flow_id(n))):forge_flow_id(n)==id)) {
        const auto& row=forge_flow_scene_node(f,n);const double x=row["x"],y=row["y"];
        include(x,y,x+row["width"].get<double>(),y+row["height"].get<double>());
    }
    if(id.empty())for(const auto& e:f.document["edges"])if((!selected||f.selected_edges.contains(forge_flow_id(e)))&&!e.value("hidden",false)&&!forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(e,"source_node")))&&!forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(e,"target_node")))) {
        const auto& v=forge_flow_edge_geometry(f,e).view;include(v["min_x"],v["min_y"],v["max_x"],v["max_y"]);
    }
    return {{"x",left},{"y",top},{"width",right-left},{"height",bottom-top}};
}
static std::string forge_flow_search_key(std::string text) {
    // ASCII 不区分大小写；中文等 UTF-8 内容按原文匹配，不改变多字节编码。
    for(auto& c:text)if(c>='A'&&c<='Z')c=static_cast<char>(c-'A'+'a');return text;
}
static bool forge_flow_business_hidden(ForgeFlowCore& f,const ForgeFlowJson& node) {
    const ForgeFlowJson* current=&node;
    for(;;){if(current->value("hidden",false))return true;const auto parent=forge_flow_id(*current,"parent");if(parent.empty())return false;current=&forge_flow_node(f,parent);}
}
static ForgeFlowJson forge_flow_search(ForgeFlowCore& f,const std::string& text) {
    auto result=ForgeFlowJson::array();const auto key=forge_flow_search_key(text);if(key.empty())return result;
    for(const auto& n:f.document["nodes"])if(!forge_flow_business_hidden(f,n)) {
        const auto haystack=forge_flow_search_key(forge_flow_id(n)+"\n"+n.value("title",std::string())+"\n"+n.value("kind",std::string()));
        if(haystack.find(key)!=std::string::npos)result.push_back(forge_flow_id(n));
    }
    return result;
}
static void forge_flow_reveal(ForgeFlowCore& f,const std::string& id) {
    const auto& node=forge_flow_node(f,id);forge_flow_require(!forge_flow_business_hidden(f,node),"业务隐藏的节点不能通过搜索显示");
    auto parent=forge_flow_id(node,"parent");
    while(!parent.empty()){auto& n=forge_flow_node(f,parent);n["collapsed"]=false;parent=forge_flow_id(n,"parent");}
    f.scene_nodes.clear();if(f.document["mode"]=="mindmap")forge_flow_layout(f);
    f.selected_nodes.clear();f.selected_edges.clear();if(node.value("selectable",true))f.selected_nodes.insert(id);
}
