// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

static const ForgeFlowJson& forge_flow_edge(ForgeFlowCore& f,const std::string& id) {
    for(const auto& edge:f.document["edges"])if(forge_flow_id(edge)==id)return edge;
    throw std::runtime_error("连线不存在");
}

// 按距离逐一校验候选，不让最近的非法端口遮挡稍远的合法端口。
static ForgeFlowJson forge_flow_resolve(ForgeFlowCore& f,const ForgeFlowJson& action) {
    auto result=action;result["op"]="";
    const bool reconnect=action.value("target",std::string())=="reconnect";
    const bool start=reconnect&&action.value("number",1)==0;
    ForgeFlowJson original;
    if(reconnect){original=forge_flow_edge(f,forge_flow_id(action));if(!original.value("reconnectable",true)||original.value("hidden",false))return result;}
    const auto options=forge_flow_options(f);
    const double radius=options.value("connection_radius",16.)/f.document["viewport"]["zoom"].get<double>();
    const double x=forge_flow_number(action,"x"),y=forge_flow_number(action,"y");
    std::vector<std::tuple<double,std::string,std::string>> candidates;
    for(const auto& node:f.document["nodes"]) {
        if(forge_flow_hidden(f,node)||!node.value("connectable",true))continue;
        for(const auto& port:node["ports"])if(port.value("connectable",true)) {
            const auto anchor=forge_flow_anchor(forge_flow_scene_node(f,node),port);const auto distance=std::hypot(x-anchor.x,y-anchor.y);
            if(distance<=radius)candidates.emplace_back(distance,forge_flow_id(node),forge_flow_id(port));
        }
    }
    std::sort(candidates.begin(),candidates.end());
    for(const auto& [distance,node,port]:candidates) {
        auto edge=reconnect?original:ForgeFlowJson{{"source_node",forge_flow_id(action)},{"source_port",forge_flow_id(action,"port")}};
        edge[start?"source_node":"target_node"]=node;edge[start?"source_port":"target_port"]=port;
        try{forge_flow_validate_connection(f,edge,reconnect?forge_flow_id(original):"",true,true);}catch(const std::exception&){continue;}
        result.update({{"op",reconnect?"reconnect":"connect"},{"id",edge["source_node"]},{"port",edge["source_port"]},{"target",edge["target_node"]},{"target_port",edge["target_port"]}});
        if(reconnect)result["text"]=original["id"];
        const auto& n=forge_flow_scene_node(f,forge_flow_node(f,node));const auto anchor=forge_flow_anchor(n,forge_flow_port(n,port));
        result["x"]=anchor.x;result["y"]=anchor.y;break;
    }
    return result;
}

static ForgeFlowJson forge_flow_preview(ForgeFlowCore& f,const ForgeFlowJson& action) {
    forge_flow_sync_obstacles(f);
    const auto proposal=forge_flow_resolve(f,action);
    const bool reconnect=action.value("target",std::string())=="reconnect",start=reconnect&&action.value("number",1)==0;
    auto edge=reconnect?forge_flow_edge(f,forge_flow_id(action)):ForgeFlowJson{{"id",""},{"source_node",forge_flow_id(action)},{"source_port",forge_flow_id(action,"port")},{"target_node",""},{"target_port","preview"},{"style",forge_flow_integer(action,"number",1)},{"arrow",action.value("flag",true)}};
    auto source=forge_flow_scene_node(f,forge_flow_node(f,forge_flow_id(edge,"source_node")));
    auto target=reconnect?forge_flow_scene_node(f,forge_flow_node(f,forge_flow_id(edge,"target_node"))):source;
    if(proposal["op"]!="") {
        for(const auto& [key,value]:std::vector<std::pair<std::string,std::string>>{{"source_node","id"},{"source_port","port"},{"target_node","target"},{"target_port","target_port"}})edge[key]=proposal[value];
        source=forge_flow_scene_node(f,forge_flow_node(f,forge_flow_id(edge,"source_node")));target=forge_flow_scene_node(f,forge_flow_node(f,forge_flow_id(edge,"target_node")));
    }else {
        const auto& fixed=start?target:source;const auto& p=forge_flow_port(fixed,forge_flow_id(edge,start?"target_port":"source_port"));
        auto& moving=start?source:target;
        moving={{"x",forge_flow_number(action,"x")},{"y",forge_flow_number(action,"y")},{"width",0.},{"height",0.},{"ports",ForgeFlowJson::array({{{"id",forge_flow_id(edge,start?"source_port":"target_port")},{"side",p["side"].get<int>()^1},{"offset",0.}}})}};
    }
    auto view=forge_flow_edge_geometry(f,edge,&source,&target).view;view["selected"]=false;view["hidden"]=false;
    return view;
}

static ForgeFlowJson forge_flow_query(ForgeFlowCore& f,const ForgeFlowJson& a) {
    const auto kind=a.value("target",std::string()),id=forge_flow_id(a);
    auto result=ForgeFlowJson::array();
    if(kind=="nodes"||kind=="edges")result=f.document[kind];
    else if(kind=="export-nodes")result=forge_flow_export_nodes(f,a.value("flag",false));
    else if(kind=="export-svg")result=forge_flow_export_svg(f,a);
    else if(kind=="selection-fragment")result=forge_flow_selection_fragment(f);
    else if(kind=="history")result=f.history.status();
    else if(kind=="document-status") {result=f.history.status();result.update({{"dirty",f.dirty},{"schema_version",2},{"loaded_version",f.loaded_version},{"save_serial",f.save_serial},{"editing",f.dragging}});}
    else if(kind=="guides")result=f.guides;
    else if(kind=="bounds")result=forge_flow_query_bounds(f,a);
    else if(kind=="search")result=forge_flow_search(f,a.value("text",std::string()));
    else if(kind=="node")result=forge_flow_node(f,id);
    else if(kind=="node-world")result=forge_flow_scene_node(f,forge_flow_node(f,id));
    else if(kind=="edge")result=forge_flow_edge(f,id);
    else if(kind=="selection")result={{"nodes",f.selected_nodes},{"edges",f.selected_edges}};
    else if(kind=="ports") {
        const auto& n=forge_flow_node(f,id);
        for(const auto& p:n["ports"]){auto port=p;const int count=forge_flow_port_count(f,id,forge_flow_id(p));port["connection_count"]=count;port["available"]=n.value("connectable",true)&&p.value("connectable",true)&&!forge_flow_hidden(f,n)&&(p.value("max_connections",0)==0||count<p.value("max_connections",0));result.push_back(port);}
    }else if(kind=="connected-edges") {
        forge_flow_node(f,id);const auto port=forge_flow_id(a,"port");
        for(const auto& e:f.document["edges"])if((forge_flow_id(e,"source_node")==id&&(port.empty()||forge_flow_id(e,"source_port")==port))||(forge_flow_id(e,"target_node")==id&&(port.empty()||forge_flow_id(e,"target_port")==port)))result.push_back(e);
    }else if(kind=="upstream"||kind=="downstream"||kind=="neighbors") {
        forge_flow_node(f,id);std::set<std::string> seen{id};std::vector<std::string> pending{id};
        for(std::size_t i=0;i<pending.size();++i){
            for(const auto& e:f.document["edges"]){const auto source=forge_flow_id(e,"source_node"),target=forge_flow_id(e,"target_node");std::string next;
                if(kind!="upstream"&&source==pending[i])next=target;
                if(kind!="downstream"&&target==pending[i])next=source;
                if(!next.empty()&&seen.insert(next).second){result.push_back(forge_flow_node(f,next));pending.push_back(next);}
            }
            if(!a.value("flag",false))break;
        }
    }else if(kind=="intersections") {
        const double x=forge_flow_number(a,"x"),y=forge_flow_number(a,"y"),w=forge_flow_number(a,"width"),h=forge_flow_number(a,"height");
        const ForgeFlowBox box{std::min(x,x+w),std::min(y,y+h),std::max(x,x+w),std::max(y,y+h)};
        for(const auto& n:f.document["nodes"])if(!forge_flow_hidden(f,n)){const auto b=forge_flow_box(forge_flow_scene_node(f,n));if(a.value("flag",true)?forge_flow_boxes_intersect(b,box):b.left>=box.left&&b.top>=box.top&&b.right<=box.right&&b.bottom<=box.bottom)result.push_back(n);}
    }else if(kind=="edge-at") {
        forge_flow_sync_obstacles(f);result=forge_flow_hit_edge(f,forge_flow_number(a,"x"),forge_flow_number(a,"y"),a.value("width",8.)/f.document["viewport"]["zoom"].get<double>());
    }else if(kind=="options")result=forge_flow_options(f);
    else throw std::runtime_error("未知图查询");
    return {{"ok",true},{"error",""},{"result",result}};
}
