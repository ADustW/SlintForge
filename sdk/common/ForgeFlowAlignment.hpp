#pragma once

static void forge_flow_align_selection(ForgeFlowCore& f,const ForgeFlowJson& action) {
    forge_flow_require(f.document["mode"]=="graph","思维导图请使用树布局");
    const auto roots=forge_flow_selection_roots(f);forge_flow_require(roots.size()>=2,"请至少选择两个节点");
    const auto mode=action.value("target",std::string("left"));
    const bool distribute=mode=="distribute-x"||mode=="distribute-y";
    forge_flow_require(distribute||mode=="left"||mode=="right"||mode=="top"||mode=="bottom"||mode=="center-x"||mode=="center-y","对齐方式无效");
    if(distribute)forge_flow_require(roots.size()>=3,"等距排列至少需要三个节点");
    std::map<std::string,ForgeFlowBox> boxes;double left=1e100,top=1e100,right=-1e100,bottom=-1e100,total=0;
    std::map<std::string,ForgeFlowPoint> before;
    for(const auto& node:f.document["nodes"]){const auto& world=forge_flow_scene_node(f,node);before[forge_flow_id(node)]={world["x"],world["y"]};}
    for(const auto& id:roots) {
        const auto& node=forge_flow_node(f,id);forge_flow_require(node.value("draggable",true)&&!forge_flow_hidden(f,node),"选择中包含不可移动节点");
        const auto box=forge_flow_box(forge_flow_scene_node(f,node));boxes[id]=box;
        left=std::min(left,box.left);top=std::min(top,box.top);right=std::max(right,box.right);bottom=std::max(bottom,box.bottom);
        total+=mode=="distribute-x"?box.right-box.left:box.bottom-box.top;
    }
    std::vector<std::string> order(roots.begin(),roots.end());
    const bool horizontal=mode=="distribute-x";
    if(distribute)std::stable_sort(order.begin(),order.end(),[&](const auto& a,const auto& b){return horizontal?boxes[a].left<boxes[b].left:boxes[a].top<boxes[b].top;});
    const double gap=distribute?((horizontal?right-left:bottom-top)-total)/(order.size()-1):0;
    if(distribute)forge_flow_require(gap>=0,"当前范围不足以无重叠地等距排列");
    double cursor=horizontal?left:top;
    for(const auto& id:order) {
        const auto box=boxes[id];double dx=0,dy=0;
        if(mode=="left")dx=left-box.left;else if(mode=="right")dx=right-box.right;
        else if(mode=="top")dy=top-box.top;else if(mode=="bottom")dy=bottom-box.bottom;
        else if(mode=="center-x")dx=(left+right-box.left-box.right)/2;
        else if(mode=="center-y")dy=(top+bottom-box.top-box.bottom)/2;
        else if(horizontal){dx=cursor-box.left;cursor+=box.right-box.left+gap;}
        else {dy=cursor-box.top;cursor+=box.bottom-box.top+gap;}
        auto& node=forge_flow_node(f,id);node["x"]=node["x"].get<double>()+dx;node["y"]=node["y"].get<double>()+dy;
    }
    f.scene_nodes.clear();
    for(auto& edge:f.document["edges"])if(edge.contains("control_points")) {
        const auto source=forge_flow_id(edge,"source_node"),target=forge_flow_id(edge,"target_node");
        const auto& a=forge_flow_scene_node(f,forge_flow_node(f,source));const auto& b=forge_flow_scene_node(f,forge_flow_node(f,target));
        const double dx=a["x"].get<double>()-before[source].x,dy=a["y"].get<double>()-before[source].y;
        if(std::abs(dx-(b["x"].get<double>()-before[target].x))<.001&&std::abs(dy-(b["y"].get<double>()-before[target].y))<.001)for(auto& point:edge["control_points"]){point["x"]=point["x"].get<double>()+dx;point["y"]=point["y"].get<double>()+dy;}
    }
    forge_flow_validate(f);
}

// 辅助线按世界坐标返回；吸附容差按屏幕像素换算，缩放不会改变手感。
static void forge_flow_drag_guides(ForgeFlowCore& f,const ForgeFlowJson& action,double& dx,double& dy) {
    f.guides=ForgeFlowJson::array();if(f.drag_origins.empty())return;
    const auto options=forge_flow_options(f);const double grid=options.value("grid_size",20.);
    const auto primary=forge_flow_id(action);const auto anchor_id=f.drag_origins.contains(primary)?primary:f.drag_origins.begin()->first;
    const auto anchor=f.drag_scene_origins.at(anchor_id);
    if(action.value("flag",false)){dx=std::round((anchor.x+dx)/grid)*grid-anchor.x;dy=std::round((anchor.y+dy)/grid)*grid-anchor.y;}
    forge_flow_constrain_drag(f,dx,dy);
    if(!options.value("guides_enabled",true))return;
    double left=1e100,top=1e100,right=-1e100,bottom=-1e100;
    std::set<std::string> ancestors;
    for(const auto& [id,origin]:f.drag_origins) {
        const auto world=f.drag_scene_origins.at(id);const auto& node=forge_flow_node(f,id);const auto& scene=forge_flow_scene_node(f,node);
        left=std::min(left,world.x+dx);top=std::min(top,world.y+dy);right=std::max(right,world.x+dx+scene["width"].get<double>());bottom=std::max(bottom,world.y+dy+scene["height"].get<double>());
        auto parent=forge_flow_id(node,"parent");while(!parent.empty()){ancestors.insert(parent);parent=forge_flow_id(forge_flow_node(f,parent),"parent");}
    }
    const double tolerance=options.value("guide_tolerance",6.)/f.document["viewport"]["zoom"].get<double>();
    double best_x=tolerance+1,best_y=tolerance+1,guide_x=0,guide_y=0;ForgeFlowBox reference_x{},reference_y{};
    for(const auto& node:f.document["nodes"]) {
        const auto id=forge_flow_id(node);if(f.drag_effective.contains(id)||ancestors.contains(id)||forge_flow_hidden(f,node))continue;
        const auto box=forge_flow_box(forge_flow_scene_node(f,node));
        for(const double a:{left,(left+right)/2,right})for(const double b:{box.left,(box.left+box.right)/2,box.right})if(std::abs(b-a)<std::abs(best_x)){best_x=b-a;guide_x=b;reference_x=box;}
        for(const double a:{top,(top+bottom)/2,bottom})for(const double b:{box.top,(box.top+box.bottom)/2,box.bottom})if(std::abs(b-a)<std::abs(best_y)){best_y=b-a;guide_y=b;reference_y=box;}
    }
    const bool snap=action.value("flag",false)&&options.value("guide_snap",true);
    const double initial_dx=dx,initial_dy=dy;
    if(snap){if(std::abs(best_x)<=tolerance)dx+=best_x;if(std::abs(best_y)<=tolerance)dy+=best_y;forge_flow_constrain_drag(f,dx,dy);}
    if(std::abs(best_x)<=tolerance&&(!snap||std::abs(dx-initial_dx-best_x)<.001))f.guides.push_back({{"x",guide_x},{"y",std::min(top,reference_x.top)-20},{"width",0},{"height",std::max(bottom,reference_x.bottom)-std::min(top,reference_x.top)+40}});
    if(std::abs(best_y)<=tolerance&&(!snap||std::abs(dy-initial_dy-best_y)<.001))f.guides.push_back({{"x",std::min(left,reference_y.left)-20},{"y",guide_y},{"width",std::max(right,reference_y.right)-std::min(left,reference_y.left)+40},{"height",0}});
}
