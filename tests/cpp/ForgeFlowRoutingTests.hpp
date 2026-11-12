// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <sstream>

// 直接检查最终 SVG 路径，覆盖绘制、标签和命中所使用的实际几何。
static std::vector<std::pair<double,double>> forge_flow_test_path(const std::string& svg) {
    std::istringstream input(svg);char command;double x=0,y=0;
    std::vector<std::pair<double,double>> points;
    while(input>>command){
        if(command=='M'){input>>x>>y;points.emplace_back(x,y);}
        else if(command=='L'){
            double nx,ny;input>>nx>>ny;
            for(int i=1;i<=128;++i){const double t=i/128.;points.emplace_back(x+(nx-x)*t,y+(ny-y)*t);}x=nx;y=ny;
        }else if(command=='C'){
            double cx,cy,dx,dy,nx,ny;input>>cx>>cy>>dx>>dy>>nx>>ny;
            for(int i=1;i<=128;++i){const double t=i/128.,u=1-t;points.emplace_back(u*u*u*x+3*u*u*t*cx+3*u*t*t*dx+t*t*t*nx,u*u*u*y+3*u*u*t*cy+3*u*t*t*dy+t*t*t*ny);}x=nx;y=ny;
        }else throw std::runtime_error("不支持的测试路径指令");
    }
    return points;
}
static nlohmann::json forge_flow_test_edge(ForgeFlowDocument& doc,const std::string& id) {
    const auto state=nlohmann::json::parse(doc.snapshot());
    for(const auto& e:state["edges"])if(e["id"]==id)return e;
    throw std::runtime_error("测试连线不存在");
}
static void forge_flow_test_clear(ForgeFlowDocument& doc,const std::string& id) {
    const auto edge=forge_flow_test_edge(doc,id);
    const auto points=forge_flow_test_path(edge["path"]);
    const auto document=nlohmann::json::parse(doc.export_json());
    for(const auto& node:document["nodes"]){
        const double x=node["x"],y=node["y"],w=node["width"],h=node["height"];
        for(const auto& [px,py]:points)if(px>x+.02&&px<x+w-.02&&py>y+.02&&py<y+h-.02)
            throw std::runtime_error("连线穿过节点 "+node["id"].get<std::string>()+" path="+edge["path"].get<std::string>());
    }
    forge_flow_test_require(!points.empty() && std::isfinite(edge["label_x"].get<double>()) && std::isfinite(edge["label_y"].get<double>()),"路径或标签无效");
}
static void forge_flow_test_routing_and_appearance() {
    ForgeFlowDocument sample;
    forge_flow_test_command(sample,{{"op","demo"},{"number",1}});
    forge_flow_test_clear(sample,"edge-3");
    for(int style:{1,2,3})for(int source=0;source<4;++source)for(int target=0;target<4;++target)
        for(const auto& position:std::vector<std::pair<double,double>>{{400,240},{-350,240},{400,0},{0,300}}){
            ForgeFlowDocument doc;
            doc.add_node({{"id","a"},{"ports",nlohmann::json::array({{{"id","p"},{"direction",1},{"side",source},{"offset",.5}}})}});
            doc.add_node({{"id","b"},{"x",position.first},{"y",position.second},{"ports",nlohmann::json::array({{{"id","p"},{"direction",0},{"side",target},{"offset",.5}}})}});
            auto linked=doc.connect("a","p","b","p",style);
            forge_flow_test_require(linked["ok"].get<bool>(),"四边端口连接失败");
            const auto edge=linked["edges"][0];forge_flow_test_clear(doc,edge["id"]);
            const auto points=forge_flow_test_path(edge["path"]);const auto first=points[1],last=points[points.size()-2];
            const double sx=edge["source_x"],sy=edge["source_y"],tx=edge["target_x"],ty=edge["target_y"];
            forge_flow_test_require(source==0?first.first<sx:source==1?first.first>sx:source==2?first.second<sy:first.second>sy,"起点未沿端口向外引出");
            forge_flow_test_require(target==0?last.first<tx:target==1?last.first>tx:target==2?last.second<ty:last.second>ty,"终点未从端口外侧进入");
        }
    ForgeFlowDocument obstacle;
    obstacle.add_node({{"id","a"}});obstacle.add_node({{"id","b"},{"x",600}});
    obstacle.add_node({{"id","blocker"},{"x",300},{"y",300}});
    const auto connected=obstacle.connect("a","out","b","in",2);const std::string id=connected["edges"][0]["id"];
    const auto before=forge_flow_test_edge(obstacle,id)["path"];
    forge_flow_test_command(obstacle,{{"op","drag-begin"},{"id","blocker"}});
    const auto delta=forge_flow_test_command(obstacle,{{"op","drag-move"},{"y",-300}});
    forge_flow_test_require(delta["edges"].size()==1,"未刷新受遮挡的非相邻连线");
    forge_flow_test_clear(obstacle,id);forge_flow_test_command(obstacle,{{"op","drag-end"}});
    forge_flow_test_require(forge_flow_test_edge(obstacle,id)["path"]!=before,"路径缓存没有失效");
    forge_flow_test_command(obstacle,{{"op","undo"}});forge_flow_test_require(forge_flow_test_edge(obstacle,id)["path"]==before,"撤销移动未恢复路径");
    const auto row=forge_flow_test_edge(obstacle,id);
    const auto selection=forge_flow_test_command(obstacle,{{"op","pick"},{"x",row["label_x"]},{"y",row["label_y"]}});
    forge_flow_test_require(selection["selected_edge"]["id"]==id,"绕行路径命中失败");
    const auto original=obstacle.export_json();
    auto changed=obstacle.update_edge_appearance(id,{{"style",1},{"arrow",false},{"stroke_width",5},{"label","跨行连线"}});
    forge_flow_test_require(changed["ok"].get<bool>() && changed["selected_edge"]["style"]==1 && changed["selected_edge"]["stroke_width"]==5 && changed["selected_edge"]["arrow_path"]=="","样式增量或选中状态未更新");
    const auto styled=obstacle.export_json();ForgeFlowDocument restored;restored.import_json(styled);
    forge_flow_test_require(restored.export_json()==styled,"样式保存加载失败");
    obstacle.undo();forge_flow_test_require(obstacle.export_json()==original,"样式不是一步撤销");obstacle.redo();forge_flow_test_require(obstacle.export_json()==styled,"样式重做失败");
    for(const auto& bad:std::vector<nlohmann::json>{{{"stroke_width",0}},{{"stroke_width",9}},{{"style",1.5}},{{"source_node","blocker"}},{{"arrow","yes"}}}){
        const auto result=obstacle.update_edge_appearance(id,bad);forge_flow_test_require(!result["ok"].get<bool>()&&obstacle.export_json()==styled,"非法样式污染文档");
    }
    obstacle.update_edge_appearance(id,{{"stroke_width",8}});obstacle.set_viewport(0,0,4);
    auto thick=forge_flow_test_edge(obstacle,id);
    auto picked=forge_flow_test_command(obstacle,{{"op","pick"},{"x",thick["label_x"]},{"y",thick["label_y"].get<double>()+3}});
    forge_flow_test_require(picked["selected_edge"]["id"]==id,"放大后可见粗线的边缘不能命中");
    for(double gap:{0.,.000001,8.})for(int style:{1,2}){
        ForgeFlowDocument close;close.add_node({{"id","a"}});close.add_node({{"id","b"},{"x",220+gap}});
        auto connected_close=close.connect("a","out","b","in",style);
        forge_flow_test_require(connected_close["ok"].get<bool>(),"接近或重合端口的路径失败");
        forge_flow_test_require(std::isfinite(connected_close["edges"][0]["label_x"].get<double>()),"退化路径标签无效");
        forge_flow_test_clear(close,connected_close["edges"][0]["id"]);
    }
    std::cout<<"FLOW_ROUTING_OK directions=192 obstacle_move=incremental appearance=undo_redo_roundtrip\n";
}
