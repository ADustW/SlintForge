#pragma once

static void forge_flow_test_editor_components() {
    ForgeFlowDocument graph;graph.add_node({{"id","a"}});graph.add_node({{"id","b"},{"x",400}});graph.apply_checked({{"op","select-all"}});
    const auto original=graph.export_json();
    auto result=graph.patch_selected_nodes({{"width",260},{"height",180},{"layout_fixed",true}});
    forge_flow_test_require(result["ok"]&&graph.query("node","a")["width"]==260&&graph.query("node","b")["layout_fixed"]==true,"批量尺寸属性没有应用到全部节点");
    graph.undo();forge_flow_test_require(graph.export_json()==original,"批量属性没有作为一步撤销");graph.redo();
    graph.patch_node("a",{{"resizable",false}});const auto before=graph.export_json();
    forge_flow_test_require(!graph.patch_selected_nodes({{"width",300}})["ok"].get<bool>()&&graph.export_json()==before,"尺寸权限失败没有回滚整个选区");
    forge_flow_test_require(!graph.patch_selected_nodes({{"id","invalid"}})["ok"].get<bool>()&&graph.export_json()==before,"批量属性允许修改标识");
    graph.patch_selected_nodes({{"selectable",false}});forge_flow_test_require(graph.query("selection")["nodes"].empty(),"禁用选择没有清理选区");graph.undo();
    graph.select("a");graph.apply_checked({{"op","drag-begin"},{"id","a"},{"x",0},{"y",0}});graph.apply_checked({{"op","drag-move"},{"id","a"},{"x",40},{"y",50}});graph.set_viewport(-50,-30,1.5);graph.apply_checked({{"op","cancel"}});
    const auto cancelled=nlohmann::json::parse(graph.export_json());forge_flow_test_require(cancelled["viewport"]["x"]==-50&&graph.query("node","a")["x"]==0,"取消拖动错误地撤销了自动平移");
    std::cout<<"FLOW_EDITOR_COMPONENTS_OK bulk=atomic history=single permissions=all cancel=viewport_preserved\n";
}
