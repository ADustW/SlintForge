// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

// 标记采用端点局部坐标：尖端为原点，负 X 方向沿连线向内。
// 自定义轮廓只包含坐标，不接受脚本或任意路径命令。
static ForgeFlowJson forge_flow_marker(const ForgeFlowJson& edge, bool start) {
    const auto key=start?"marker_start":"marker_end";
    auto marker=edge.value(key,ForgeFlowJson(start?"none":edge.value("arrow",true)?"arrow":"none"));
    if(marker.is_string())marker=ForgeFlowJson{{"shape",marker}};
    forge_flow_require(marker.is_object(),"端点标记必须是名称或对象");
    const auto shape=marker.value("shape",std::string("none"));
    forge_flow_require(shape=="none"||shape=="arrow"||shape=="open-arrow"||shape=="circle"||shape=="diamond"||shape=="custom","端点标记类型无效");
    const double size=forge_flow_number(marker,"size",11.);
    forge_flow_require(size>=4&&size<=64,"端点标记尺寸必须位于 4 至 64");
    forge_flow_require(!marker.contains("filled")||marker["filled"].is_boolean(),"标记填充必须为布尔值");
    const bool filled=marker.value("filled",shape!="open-arrow");
    auto points=marker.value("points",ForgeFlowJson::array());
    if(shape=="custom") {
        forge_flow_require(points.is_array()&&points.size()>=(filled?3u:2u)&&points.size()<=64,"自定义标记需要 2 至 64 个点，填充轮廓至少需要 3 个点");
        for(const auto& point:points) {
            forge_flow_require(point.is_object()&&point.contains("x")&&point.contains("y"),"标记轮廓缺少坐标");
            const double x=forge_flow_number(point,"x"),y=forge_flow_number(point,"y");
            forge_flow_require(x>=-2&&x<=0&&std::abs(y)<=1,"标记轮廓的 X 必须位于 -2 至 0，Y 必须位于 -1 至 1");
        }
    }
    return {{"shape",shape},{"size",size},{"filled",filled},{"points",points}};
}

static void forge_flow_validate_appearance(const ForgeFlowJson& edge) {
    forge_flow_marker(edge,true);forge_flow_marker(edge,false);
    forge_flow_require(!edge.contains("animated")||edge["animated"].is_boolean(),"连线动画开关必须为布尔值");
    const double speed=forge_flow_number(edge,"animation_speed",60.),curvature=forge_flow_number(edge,"curvature",.4),offset=forge_flow_number(edge,"parallel_offset");
    const int layer=forge_flow_integer(edge,"layer");
    forge_flow_require(speed>=5&&speed<=600,"连线流动速度必须位于 5 至 600 像素每秒");
    forge_flow_require(curvature>=0&&curvature<=2,"连线曲率必须位于 0 至 2");
    forge_flow_require(std::abs(offset)<=512,"并行连线偏移不能超过 512");
    forge_flow_require(layer>=-900&&layer<=1500,"连线层级必须位于 -900 至 1500");
}

static ForgeFlowMarkerGeometry forge_flow_marker_geometry(const ForgeFlowJson& marker,ForgeFlowPoint anchor,double angle) {
    ForgeFlowMarkerGeometry result;result.filled=marker["filled"];
    const std::string shape=marker["shape"];
    if(shape=="none")return result;
    std::vector<ForgeFlowPoint> points;
    if(shape=="arrow")points={{0,0},{-.9,.435},{-.9,-.435}};
    else if(shape=="open-arrow"){points={{-.9,.435},{0,0},{-.9,-.435}};result.filled=false;}
    else if(shape=="diamond")points={{0,0},{-.5,.4},{-1,0},{-.5,-.4}};
    else if(shape=="circle")for(int i=0;i<32;++i){const double t=i*6.283185307179586/32;points.push_back({-.5+.5*std::cos(t),.5*std::sin(t)});}
    else for(const auto& point:marker["points"])points.push_back({point["x"],point["y"]});
    result.closed=shape!="open-arrow"&&(shape!="custom"||result.filled);
    const double size=marker["size"],c=std::cos(angle),s=std::sin(angle);
    for(const auto p:points)result.points.push_back({anchor.x+(p.x*c-p.y*s)*size,anchor.y+(p.x*s+p.y*c)*size});
    return result;
}

static std::string forge_flow_marker_path(const ForgeFlowMarkerGeometry& marker) {
    if(marker.points.empty())return {};
    std::ostringstream path;path.imbue(std::locale::classic());path<<std::fixed<<std::setprecision(3);
    for(std::size_t i=0;i<marker.points.size();++i)path<<(i?" L ":" M ")<<marker.points[i].x<<' '<<marker.points[i].y;
    if(marker.closed)path<<" Z";
    return path.str();
}

// 同一对节点按稳定 ID 排列通道；反向边也使用相同的世界方向。
static void forge_flow_sync_parallel(ForgeFlowCore& f) {
    if(!f.parallel_dirty)return;
    f.parallel_dirty=false;f.parallel_offsets.clear();f.terminal_spacing.clear();
    std::map<std::pair<std::string,std::string>,std::vector<const ForgeFlowJson*>> groups;
    std::map<std::pair<std::string,std::string>,std::vector<std::pair<const ForgeFlowJson*,bool>>> terminals;
    for(const auto& edge:f.document["edges"]) {
        if(edge.value("hidden",false)||forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(edge,"source_node")))||forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(edge,"target_node"))))continue;
        auto a=forge_flow_id(edge,"source_node"),b=forge_flow_id(edge,"target_node");
        if(a>b)std::swap(a,b);
        if(edge.value("path_kind",std::string("auto"))=="auto") {
            groups[{a,b}].push_back(&edge);
            // 显式偏移由调用方控制；自动分支只对共用同一端口的可见连线分配。
            if(!edge.contains("parallel_offset")) {
                terminals[{forge_flow_id(edge,"source_node"),forge_flow_id(edge,"source_port")}].push_back({&edge,true});
                terminals[{forge_flow_id(edge,"target_node"),forge_flow_id(edge,"target_port")}].push_back({&edge,false});
            }
        }
    }
    const double spacing=forge_flow_options(f).value("parallel_spacing",24.);
    for(auto& [key,edges]:groups) {
        std::sort(edges.begin(),edges.end(),[](const auto* a,const auto* b){return forge_flow_id(*a)<forge_flow_id(*b);});
        for(std::size_t i=0;i<edges.size();++i) {
            const auto& edge=*edges[i];const double direction=forge_flow_id(edge,"source_node")==key.first?1.:-1.;
            f.parallel_offsets[forge_flow_id(edge)]=(double(i)-(double(edges.size())-1)/2)*spacing*direction;
        }
    }
    for(auto& [key,edges]:terminals)if(edges.size()>1) {
        std::sort(edges.begin(),edges.end(),[](const auto& a,const auto& b){return forge_flow_id(*a.first)<forge_flow_id(*b.first);});
        const double step=std::min(12.,48./(edges.size()-1));
        for(std::size_t i=0;i<edges.size();++i) {
            const auto& [edge,source]=edges[i];
            auto [entry,inserted]=f.terminal_spacing.try_emplace(forge_flow_id(*edge),24.,24.);
            (source?entry->second.first:entry->second.second)=36.+i*step;
        }
    }
}

static ForgeFlowPoint forge_flow_path_point(const std::vector<ForgeFlowPoint>& points,double distance) {
    if(points.empty())return {};
    for(std::size_t i=1;i<points.size();++i) {
        const auto a=points[i-1],b=points[i];const double length=std::hypot(b.x-a.x,b.y-a.y);
        if(distance<=length&&length>1e-9){const double t=std::clamp(distance/length,0.,1.);return {a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t};}
        distance-=length;
    }
    return points.back();
}
