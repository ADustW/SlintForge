// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <filesystem>
#include <fstream>
#include <set>

static void forge_flow_test_dense_routing() {
    std::ifstream fixture(std::filesystem::path(__FILE__).parent_path()/"../fixtures/flow-routing-dense.json");
    const auto input=nlohmann::json::parse(fixture);
    ForgeFlowDocument dense;
    forge_flow_test_require(dense.import_json(input.dump())["ok"],"密集连线场景加载失败");
    for(const auto& edge:input["edges"]) {
        const auto id=edge["id"].get<std::string>();
        forge_flow_test_clear(dense,id);
        const auto path=forge_flow_test_path(forge_flow_test_edge(dense,id)["path"]);
        // 实际描边也应在卡片外，不能只检查线条中心。端点卡片允许接入自身边界。
        for(const auto& node:input["nodes"])if(node["id"]!=edge["source_node"]&&node["id"]!=edge["target_node"]) {
            const double x=node["x"],y=node["y"],w=node["width"],h=node["height"];
            for(const auto [px,py]:path)forge_flow_test_require(!(px>x-3.75&&px<x+w+3.75&&py>y-3.75&&py<y+h+3.75),"粗线或圆角侵入非端点卡片");
        }
    }
    for(bool incoming:{false,true}) {
        ForgeFlowDocument fan;
        fan.add_node({{"id","center"},{"x",incoming?700:0},{"y",240}});
        for(int i=0;i<3;++i) {
            const auto id="branch-"+std::to_string(i);
            fan.add_node({{"id",id},{"x",incoming?0:700},{"y",i*220}});
            forge_flow_test_require((incoming?fan.connect(id,"out","center","in",2):fan.connect("center","out",id,"in",2))["ok"],"共用端口连接失败");
        }
        std::set<double> channels;
        const auto state=nlohmann::json::parse(fan.snapshot());
        for(const auto& edge:state["edges"]) {
            forge_flow_test_clear(fan,edge["id"]);
            const auto points=forge_flow_test_path(edge["path"]);
            const double anchor_x=incoming?edge["target_x"].get<double>():edge["source_x"].get<double>();
            const double anchor_y=incoming?edge["target_y"].get<double>():edge["source_y"].get<double>();
            // 第一段转弯应在共用端口附近，分支不能合并为难以选择的长主干。
            double channel=anchor_x;
            if(incoming){for(auto p=points.rbegin();p!=points.rend();++p)if(std::abs(p->second-anchor_y)>.01){channel=p->first;break;}}
            else {for(auto p:points)if(std::abs(p.second-anchor_y)>.01){channel=p.first;break;}}
            forge_flow_test_require(std::abs(channel-anchor_x)>=24&&std::abs(channel-anchor_x)<=84,"分支未在端口附近分流");
            channels.insert(channel);
            // 每条分支上独立的竖直段可被选中。
            const double other_y=incoming?edge["source_y"].get<double>():edge["target_y"].get<double>();
            forge_flow_test_require(fan.query("edge-at","",{{"x",channel},{"y",(anchor_y+other_y)/2},{"width",1}})==edge["id"],"共用端口分支无法独立命中");
        }
        forge_flow_test_require(channels.size()==3,"共用端口的分支通道重合");
        const auto original=fan.export_json();
        fan.remove_edge(state["edges"][0]["id"]);fan.undo();
        forge_flow_test_require(fan.export_json()==original,"分支删除撤销未恢复文档");
        const auto restored=nlohmann::json::parse(fan.snapshot());
        for(std::size_t i=0;i<state["edges"].size();++i)forge_flow_test_require(restored["edges"][i]["path"]==state["edges"][i]["path"],"分支通道恢复不确定");
    }
    ForgeFlowDocument straight;
    straight.add_node({{"id","a"}});straight.add_node({{"id","b"},{"x",650}});
    straight.add_node({{"id","blocker"},{"x",330},{"y",250}});
    const auto connected=straight.connect("a","out","b","in",0);
    const auto id=connected["edges"][0]["id"].get<std::string>();
    const auto direct=forge_flow_test_edge(straight,id)["path"];
    straight.patch_node("blocker",{{"y",0}});
    forge_flow_test_clear(straight,id);
    forge_flow_test_require(forge_flow_test_edge(straight,id)["path"]!=direct,"直线优先模式未避开新增障碍物");
    straight.undo();forge_flow_test_require(forge_flow_test_edge(straight,id)["path"]==direct,"移走障碍物未恢复直连");
    ForgeFlowDocument group;
    group.add_node({{"id","a"},{"y",-30}});group.add_node({{"id","b"},{"x",900},{"y",-30}});
    group.add_node({{"id","group"},{"x",330},{"width",350},{"height",300},{"is_group",true}});
    const auto over_header=group.connect("a","out","b","in",0);
    for(const auto [x,y]:forge_flow_test_path(over_header["edges"][0]["path"]))
        forge_flow_test_require(!(x>330&&x<680&&y>0&&y<36),"分组标题底部未参与避障");
    ForgeFlowDocument close;
    close.configure({{"allow_parallel_connections",true}});
    close.add_node({{"id","a"}});close.add_node({{"id","b"},{"x",240}});
    close.connect("a","out","b","in",2);close.connect("a","out","b","in",2);
    const auto close_state=nlohmann::json::parse(close.snapshot());
    for(const auto& edge:close_state["edges"]) {
        const auto points=forge_flow_test_path(edge["path"]);
        double left=1e9,right=-1e9;
        for(const auto [x,y]:points)if(std::abs(y-edge["source_y"].get<double>())>1){left=std::min(left,x);right=std::max(right,x);}
        forge_flow_test_require(right-left>1,"近距离并行边退化为原路折返的尖刺");
    }
    std::cout<<"FLOW_DENSE_ROUTING_OK stroke_clearance=12 fan_out=3 fan_in=3 pick=independent straight=avoids_obstacles history=deterministic\n";
}
