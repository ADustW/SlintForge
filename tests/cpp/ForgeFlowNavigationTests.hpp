// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

static void forge_flow_test_navigation() {
    ForgeFlowDocument graph;
    const auto fixture=nlohmann::json{{"version",1},{"mode","graph"},{"nodes",nlohmann::json::array({
        {{"id","group"},{"kind","group"},{"x",100},{"y",80},{"width",500},{"height",360},{"collapsed",true}},
        {{"id","nested"},{"kind","group"},{"parent","group"},{"x",20},{"y",50},{"width",300},{"height",200},{"collapsed",true}},
        {{"id","sensor"},{"parent","nested"},{"title","温度 Sensor"},{"x",30},{"y",60},{"width",100},{"height",80}},
        {{"id","hidden"},{"parent","group"},{"title","Hidden Sensor"},{"hidden",true}},
        {{"id","other"},{"x",900},{"y",300},{"width",120},{"height",80}}
    })},{"edges",nlohmann::json::array()}};
    graph.import_json(fixture.dump());
    forge_flow_test_require(graph.search("SENSOR")==nlohmann::json::array({"sensor"})&&graph.search("温度")==nlohmann::json::array({"sensor"}),"搜索未覆盖折叠节点或暴露业务隐藏节点");
    forge_flow_test_require(graph.bounds(false,"sensor")["width"]==0,"折叠子节点参与了可见边界");
    auto state=graph.reveal("sensor");forge_flow_test_require(state["ok"]&&state["selected_id"]=="sensor","定位没有选择节点");
    forge_flow_test_require(graph.bounds(true)==nlohmann::json({{"x",150.},{"y",190.},{"width",100.},{"height",80.}}),"嵌套节点定位没有使用世界坐标");
    graph.undo();auto doc=nlohmann::json::parse(graph.export_json());
    forge_flow_test_require(doc["nodes"][0]["collapsed"]==true&&doc["nodes"][1]["collapsed"]==true,"展开祖先没有作为一步历史恢复");
    const auto before=graph.export_json();forge_flow_test_require(!graph.reveal("hidden")["ok"].get<bool>()&&graph.export_json()==before,"定位业务隐藏节点未原子拒绝");
    forge_flow_test_require(graph.configure({{"min_zoom",.5},{"max_zoom",2.},{"wheel_mode","pan"},{"wheel_zoom_step",1.2}})["ok"],"配置导航选项失败");
    forge_flow_test_require(graph.set_viewport(10,20,10)["viewport"]["zoom"]==2.,"视口没有服从最大缩放");
    graph.configure({{"max_zoom",1.5}});graph.undo();forge_flow_test_require(nlohmann::json::parse(graph.export_json())["viewport"]["zoom"]==1.5,"撤销选项修改错误地撤销了视口");
    for(const auto& invalid:nlohmann::json::array({{{"min_zoom",3},{"max_zoom",2}},{{"min_zoom",0}},{{"wheel_zoom_step",1}},{{"auto_pan_margin",0}},{{"auto_pan","yes"}},{{"wheel_mode","invalid"}}})) {
        const auto saved=graph.export_json();forge_flow_test_require(!graph.configure(invalid)["ok"].get<bool>()&&graph.export_json()==saved,"无效导航选项未原子拒绝");
    }
    auto bad=nlohmann::json::parse(graph.export_json());bad["viewport"]["zoom"]=10;
    const auto saved=graph.export_json();forge_flow_test_require(!graph.import_json(bad.dump())["ok"].get<bool>()&&graph.export_json()==saved,"加载越界视口破坏了原文档");
    graph.reveal("sensor");graph.select("other",true);graph.align_selected("top");graph.layout_graph({{"algorithm","layered"},{"direction","TB"}});
    std::cout<<"FLOW_NAVIGATION_OK\n";
}
