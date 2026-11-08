// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

static void forge_flow_test_policies_and_transactions() {
    ForgeFlowDocument maze;
    for(const auto& node:std::vector<nlohmann::json>{
        {{"id","s"},{"x",0},{"y",100},{"width",64},{"height",48}},
        {{"id","t"},{"x",600},{"y",400},{"width",64},{"height",48}},
        {{"id","upper"},{"x",100},{"y",60},{"width",200},{"height",100}},
        {{"id","lower-left"},{"x",20},{"y",250},{"width",200},{"height",100}},
        {{"id","upper-right"},{"x",550},{"y",250},{"width",100},{"height",160}},
        {{"id","lower"},{"x",250},{"y",380},{"width",180},{"height",100}}
    })forge_flow_test_require(maze.add_node(node)["ok"].get<bool>(),"迷宫场景构造失败");
    const auto routed=maze.connect("s","out","t","in",2);
    forge_flow_test_require(routed["ok"].get<bool>(),"密集障碍连接失败");forge_flow_test_clear(maze,routed["edges"][0]["id"]);
    ForgeFlowDocument nearby;
    nearby.add_node({{"id","source"}});nearby.add_node({{"id","target"},{"x",400},{"ports",nlohmann::json::array({{{"id","blocked"},{"side",0},{"offset",.5},{"direction",0},{"connectable",false}},{{"id","valid"},{"side",0},{"offset",.56},{"direction",0}}})}});
    const auto candidate=nearby.apply({{"op","resolve"},{"id","source"},{"port","out"},{"x",400},{"y",66}});
    forge_flow_test_require(candidate["target_port"]=="valid","最近的非法端口遮挡了合法候选");
    ForgeFlowDocument graph;
    graph.batch(nlohmann::json::array({{{"op","insert"},{"node",{{"id","a"},{"x",0},{"y",0}}}},{{"op","insert"},{"node",{{"id","b"},{"x",400},{"y",0}}}},{{"op","insert"},{"node",{{"id","c"},{"x",800},{"y",0}}}}}));
    forge_flow_test_require(graph.query("nodes").size()==3,"批量创建失败");
    graph.undo();forge_flow_test_require(graph.query("nodes").empty(),"批量操作没有合并历史");graph.redo();
    const auto before=graph.export_json();const auto history=nlohmann::json::parse(graph.snapshot());
    auto failed=graph.batch(nlohmann::json::array({{{"op","title"},{"id","a"},{"text","不应保留"}},{{"op","select"},{"id","b"}},{{"op","delete"}},{{"op","connect"},{"id","a"},{"port","out"},{"target","missing"},{"target_port","in"}}}));
    forge_flow_test_require(!failed["ok"].get<bool>()&&graph.export_json()==before&&graph.query("selection")["nodes"].empty(),"失败事务未恢复文档与选择");
    forge_flow_test_require(nlohmann::json::parse(graph.snapshot())["can_redo"]==history["can_redo"],"失败事务改变历史");
    graph.connect("a","out","b","in");graph.connect("b","out","c","in");
    forge_flow_test_require(graph.downstream("a").size()==1&&graph.downstream("a",true).size()==2&&graph.upstream("c",true).size()==2,"邻接查询错误");
    forge_flow_test_require(graph.connected_edges("b").size()==2&&graph.ports("b")[0]["connection_count"]==1,"端口或连接查询错误");
    const auto edge=graph.query("edges")[0];const std::string edge_id=edge["id"];
    graph.patch_node("b",{{"connectable",false}});
    forge_flow_test_require(!graph.connect("c","out","b","in")["ok"].get<bool>(),"SDK 绕过节点连接限制");
    forge_flow_test_require(graph.query("edges").size()==2,"锁定节点破坏已有连接");
    graph.patch_node("b",{{"connectable",true}});
    auto node=graph.query("node","b");node["ports"][0]["max_connections"]=1;graph.update_node("b",node);
    forge_flow_test_require(!graph.connect("c","out","b","in")["ok"].get<bool>(),"端口容量未生效");
    forge_flow_test_require(!graph.ports("b")[0]["available"].get<bool>(),"容量状态不一致");
    graph.configure({{"connection_mode","loose"},{"allow_self_connections",true},{"connection_radius",30}});
    forge_flow_test_require(graph.connect("a","in","c","out")["ok"].get<bool>(),"宽松方向模式失效");
    forge_flow_test_require(graph.connect("c","out","c","in",3)["ok"].get<bool>(),"自连接策略失效");
    const auto loop=graph.query("edges").back();const auto loop_view=forge_flow_test_edge(graph,loop["id"]);
    forge_flow_test_require(loop_view["path"].get<std::string>().size()>60,"自连接缺少外侧路径");
    graph.patch_node("a",{{"draggable",false},{"deletable",false},{"resizable",false}});
    forge_flow_test_command(graph,{{"op","drag-begin"},{"id","a"}},false);
    forge_flow_test_command(graph,{{"op","resize-begin"},{"id","a"}},false);
    graph.select("a");forge_flow_test_require(!graph.remove_selected()["ok"].get<bool>(),"删除保护失效");
    graph.patch_node("a",{{"hidden",true}});
    forge_flow_test_require(graph.intersections(-10,-10,250,180).empty(),"业务隐藏没有影响空间查询");
    graph.patch_node("a",{{"hidden",false}});
    graph.patch_edge(edge_id,{{"kind","business-wire"},{"data",{{"value",42}}},{"path_kind","polyline"},{"control_points",nlohmann::json::array({{{"x",300},{"y",260}}})}});
    const auto custom=forge_flow_test_edge(graph,edge_id);
    forge_flow_test_require(custom["source_node"]=="a"&&custom["target_node"]=="b"&&custom["kind"]=="business-wire"&&nlohmann::json::parse(custom["data_json"].get<std::string>())["value"]==42,"连线扩展数据未映射");
    forge_flow_test_require(graph.query("edge-at","",{{"x",300},{"y",260}})==edge_id,"自定义路径命中没有更新");
    const auto before_bend=graph.export_json();
    forge_flow_test_command(graph,{{"op","bend-begin"},{"id",edge_id},{"number",0}});
    for(int i=0;i<20;++i)forge_flow_test_command(graph,{{"op","bend-move"},{"x",300+i},{"y",270+i}});
    forge_flow_test_command(graph,{{"op","bend-end"}});graph.undo();
    forge_flow_test_require(graph.export_json()==before_bend,"连续控制点拖动没有合并历史");
    graph.patch_node("c",{{"width",240},{"height",120},{"min_width",160},{"min_height",80},{"max_width",320},{"max_height",160},{"aspect_ratio",2}});
    forge_flow_test_command(graph,{{"op","resize-begin"},{"id","c"},{"number",3}});
    forge_flow_test_command(graph,{{"op","resize-move"},{"x",600},{"y",600}});
    forge_flow_test_command(graph,{{"op","resize-end"}});
    auto resized=graph.query("node","c");forge_flow_test_require(resized["width"]==320&&resized["height"]==160,"固定比例和最大尺寸失效");
    ForgeFlowDocument constrained;
    forge_flow_test_require(constrained.add_node({{"id","ratio"},{"width",180},{"height",90},{"aspect_ratio",2},{"max_width",180}})["ok"].get<bool>(),"合法的小比例节点被拒绝");
    forge_flow_test_command(constrained,{{"op","resize-begin"},{"id","ratio"},{"number",3},{"width",160},{"height",112}});
    forge_flow_test_command(constrained,{{"op","resize-move"},{"x",100},{"y",100}});
    forge_flow_test_command(constrained,{{"op","resize-end"}});
    forge_flow_test_require(constrained.query("node","ratio")["height"]==90,"默认手柄最小尺寸突破比例上限");
    forge_flow_test_require(!constrained.patch_node("ratio",{{"header_height",100},{"height",120}})["ok"].get<bool>(),"标题高度与比例上限冲突未被拒绝");
    int notifications=0;graph.set_before_edit([](const auto& a){return a.value("op",std::string())=="delete"?std::string("业务禁止删除"):std::string();});
    graph.set_after_edit([&](const auto&,const auto&){++notifications;});
    graph.select("c");const auto protected_document=graph.export_json();auto denied=graph.remove_selected();
    forge_flow_test_require(!denied["ok"].get<bool>()&&denied["error"]=="业务禁止删除"&&graph.export_json()==protected_document&&notifications==1,"编辑审核或通知错误");
    ForgeFlowDocument restored;restored.import_json(protected_document);forge_flow_test_require(restored.export_json()==protected_document,"扩展文档往返不一致");
    std::cout<<"FLOW_POLICIES_OK batch=atomic_undo queries=neighbors_ports_intersections policy=capacity_direction_lock_hidden custom=geometry_hit_history events=veto_notify\n";
}
