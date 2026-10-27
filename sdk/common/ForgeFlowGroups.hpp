#pragma once

// 所有容器操作共用同一坐标转换；修改父级不改变节点在画布上的位置。
static void forge_flow_reparent_group_node(ForgeFlowCore& f,const std::string& id,const std::string& parent) {
    forge_flow_require(f.document["mode"]=="graph","普通分组操作不能用于思维导图");
    const auto scene=forge_flow_scene_node(f,forge_flow_node(f,id));
    double px=0,py=0;
    if(!parent.empty()) {
        const auto& p=forge_flow_node(f,parent);
        forge_flow_require(p.value("is_group",false),"父级必须是分组容器");
        forge_flow_require(parent!=id&&!forge_flow_has_ancestor(f,p,{id}),"分组不能成为自身或后代的子节点");
        const auto& world=forge_flow_scene_node(f,p);px=world["x"];py=world["y"];
    }
    auto& node=forge_flow_node(f,id);node["parent"]=parent;
    node["x"]=scene["x"].get<double>()-px;node["y"]=scene["y"].get<double>()-py;
    f.scene_nodes.clear();
}

static std::set<std::string> forge_flow_selection_roots(ForgeFlowCore& f) {
    std::set<std::string> roots;
    for(const auto& id:f.selected_nodes)if(f.indices.contains(id)&&!forge_flow_has_ancestor(f,forge_flow_node(f,id),f.selected_nodes))roots.insert(id);
    return roots;
}

static void forge_flow_group_selection(ForgeFlowCore& f,const ForgeFlowJson& action) {
    forge_flow_require(f.document["mode"]=="graph","思维导图使用主题分支");
    const auto roots=forge_flow_selection_roots(f);forge_flow_require(!roots.empty(),"请先选择要分组的节点");
    const auto parent=forge_flow_id(forge_flow_node(f,*roots.begin()),"parent");
    double left=1e100,top=1e100,right=-1e100,bottom=-1e100;
    for(const auto& id:roots) {
        const auto& node=forge_flow_node(f,id);
        forge_flow_require(forge_flow_id(node,"parent")==parent,"请先把选中节点移到同一父级");
        forge_flow_require(node.value("draggable",true),"不可移动节点不能通过分组改变父级");
        const double x=node["x"],y=node["y"];
        left=std::min(left,x);top=std::min(top,y);right=std::max(right,x+node["width"].get<double>());bottom=std::max(bottom,y+node["height"].get<double>());
    }
    const auto id=forge_flow_new_id(f,"group-");
    auto group=forge_flow_normalize_node({{"id",id},{"kind","group"},{"title",action.value("text",std::string("新分组"))},{"parent",parent},{"x",left-12},{"y",top-48},{"width",right-left+24},{"height",bottom-top+60}},false);
    f.document["nodes"].push_back(group);forge_flow_reindex(f);
    for(const auto& child:roots)forge_flow_reparent_group_node(f,child,id);
    forge_flow_validate(f);f.selected_nodes={id};f.selected_edges.clear();
}

static void forge_flow_ungroup(ForgeFlowCore& f,const std::string& id) {
    forge_flow_require(f.document["mode"]=="graph","思维导图使用主题父级关系");
    const auto group=forge_flow_node(f,id);
    forge_flow_require(group.value("is_group",false)&&group.value("deletable",true),"该节点不是可解除的分组");
    std::vector<std::string> children;
    for(const auto& node:f.document["nodes"])if(forge_flow_id(node,"parent")==id)children.push_back(forge_flow_id(node));
    for(const auto& edge:f.document["edges"])if(forge_flow_id(edge,"source_node")==id||forge_flow_id(edge,"target_node")==id)forge_flow_require(edge.value("deletable",true),"分组连接包含不可删除的连线");
    for(const auto& child:children)forge_flow_reparent_group_node(f,child,forge_flow_id(group,"parent"));
    auto& nodes=f.document["nodes"];nodes.erase(std::remove_if(nodes.begin(),nodes.end(),[&](const auto& n){return forge_flow_id(n)==id;}),nodes.end());
    auto& edges=f.document["edges"];edges.erase(std::remove_if(edges.begin(),edges.end(),[&](const auto& e){return forge_flow_id(e,"source_node")==id||forge_flow_id(e,"target_node")==id;}),edges.end());
    forge_flow_reindex(f);forge_flow_validate(f);f.selected_nodes=std::set<std::string>(children.begin(),children.end());f.selected_edges.clear();
}

// 选中组和组内节点时只移动最高层祖先；锁定子节点仍然跟随容器移动。
static void forge_flow_begin_group_drag(ForgeFlowCore& f,const std::string& primary) {
    auto candidates=f.selected_nodes;candidates.insert(primary);
    for(const auto& id:candidates) {
        const auto& node=forge_flow_node(f,id);
        if(!forge_flow_has_ancestor(f,node,candidates)&&node.value("draggable",true)&&!forge_flow_hidden(f,node))f.drag_origins[id]={node["x"],node["y"]};
    }
    std::set<std::string> roots;for(const auto& [id,point]:f.drag_origins)roots.insert(id);
    f.drag_effective=forge_flow_descendants(f,roots);
}

static void forge_flow_constrain_drag(ForgeFlowCore& f,double& dx,double& dy) {
    if(f.document["mode"]!="graph")return;
    double minx=-1e100,miny=-1e100,maxx=1e100,maxy=1e100;
    for(const auto& [id,origin]:f.drag_origins) {
        const auto& node=forge_flow_node(f,id);const auto parent=forge_flow_id(node,"parent");
        if(parent.empty()||!node.value("constrain_parent",false))continue;
        const auto& p=forge_flow_node(f,parent);const double top=p.value("header_visible",true)?p.value("header_height",36.)+12.:12.;
        minx=std::max(minx,12-origin.x);miny=std::max(miny,top-origin.y);
        maxx=std::min(maxx,p["width"].get<double>()-12-node["width"].get<double>()-origin.x);
        maxy=std::min(maxy,p["height"].get<double>()-12-node["height"].get<double>()-origin.y);
    }
    forge_flow_require(minx<=maxx&&miny<=maxy,"选中节点的父级约束冲突");dx=std::clamp(dx,minx,maxx);dy=std::clamp(dy,miny,maxy);
}

static void forge_flow_translate_drag_edges(ForgeFlowCore& f,double dx,double dy) {
    for(std::size_t i=0;i<f.document["edges"].size();++i) {
        auto& edge=f.document["edges"][i];const auto& before=f.drag_before["edges"][i];
        if(!before.contains("control_points")||!f.drag_effective.contains(forge_flow_id(edge,"source_node"))||!f.drag_effective.contains(forge_flow_id(edge,"target_node")))continue;
        edge["control_points"]=before["control_points"];
        for(auto& point:edge["control_points"]){point["x"]=point["x"].get<double>()+dx;point["y"]=point["y"].get<double>()+dy;}
    }
}

static void forge_flow_group_resize_limits(ForgeFlowCore& f,const ForgeFlowJson& node,bool right,bool bottom,double& minimum_w,double& minimum_h,double& maximum_w,double& maximum_h) {
    if(f.document["mode"]!="graph")return;
    if(node.value("is_group",false))for(const auto& child:f.document["nodes"])if(forge_flow_id(child,"parent")==forge_flow_id(node)&&child.value("constrain_parent",false)) {
        minimum_w=std::max(minimum_w,child["x"].get<double>()+child["width"].get<double>()+12);
        minimum_h=std::max(minimum_h,child["y"].get<double>()+child["height"].get<double>()+12);
    }
    const auto parent=forge_flow_id(node,"parent");
    if(!parent.empty()&&node.value("constrain_parent",false)) {
        const auto& p=forge_flow_node(f,parent);const auto origin=f.drag_origins.at(forge_flow_id(node));
        const double top=p.value("header_visible",true)?p.value("header_height",36.)+12.:12.;
        maximum_w=std::min(maximum_w,right?p["width"].get<double>()-12-origin.x:origin.x+f.resize_width-12);
        maximum_h=std::min(maximum_h,bottom?p["height"].get<double>()-12-origin.y:origin.y+f.resize_height-top);
        // UI 最小值是操作提示；父级边界优先，文档声明的最小尺寸仍须满足。
        minimum_w=std::max(node.value("min_width",64.),std::min(minimum_w,maximum_w));
        minimum_h=std::max(node.value("min_height",48.),std::min(minimum_h,maximum_h));
    }
    forge_flow_require(minimum_w<=maximum_w&&minimum_h<=maximum_h,"节点尺寸与父级边界冲突");
}

static void forge_flow_drop_groups(ForgeFlowCore& f) {
    if(f.document["mode"]!="graph")return;
    if(std::none_of(f.document["nodes"].begin(),f.document["nodes"].end(),[](const auto& node){return node.value("is_group",false);}))return;
    bool changed=false;
    const auto roots=f.drag_origins;
    for(const auto& [id,origin]:roots) {
        const auto scene=forge_flow_scene_node(f,forge_flow_node(f,id));
        const double cx=scene["x"].get<double>()+scene["width"].get<double>()/2,cy=scene["y"].get<double>()+scene["height"].get<double>()/2;
        std::string target;int best=-1;
        for(const auto& candidate:f.document["nodes"]) {
            if(!candidate.value("is_group",false)||candidate.value("collapsed",false)||forge_flow_hidden(f,candidate)||f.drag_effective.contains(forge_flow_id(candidate)))continue;
            const auto& world=forge_flow_scene_node(f,candidate);const int depth=forge_flow_depth(f,candidate);
            if(cx>=world["x"].get<double>()&&cx<=world["x"].get<double>()+world["width"].get<double>()&&cy>=world["y"].get<double>()+world.value("header_height",36.)&&cy<=world["y"].get<double>()+world["height"].get<double>()&&depth>best){target=forge_flow_id(candidate);best=depth;}
        }
        if(target!=forge_flow_id(forge_flow_node(f,id),"parent")){forge_flow_reparent_group_node(f,id,target);changed=true;}
    }
    if(changed)forge_flow_validate(f);
}

static void forge_flow_clear_gesture(ForgeFlowCore& f) {
    f.dragging=false;f.resize_id.clear();f.bend_edge.clear();f.bend_index=-1;f.drag_origins.clear();f.drag_scene_origins.clear();f.drag_effective.clear();f.guides=ForgeFlowJson::array();
}
