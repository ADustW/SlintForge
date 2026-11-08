// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

static void forge_flow_test_documents_and_export() {
    ForgeFlowDocument graph;
    forge_flow_test_require(graph.document_status()["dirty"]==false,"新文档不应被标为已修改");
    graph.add_node({{"id","a"},{"title","中文 <节点> & 参数"},{"x",20},{"y",40},{"data",{{"text","正文 & <内容>"},{"private",{{"keep",true}}}}}});
    graph.add_node({{"id","b"},{"x",1800},{"y",200},{"kind","unknown-card"}});
    graph.connect("a","out","b","in",3,true,"<标签>");
    forge_flow_test_require(graph.document_status()["dirty"]==true,"编辑后没有未保存状态");
    graph.mark_saved();graph.set_viewport(800,600,.4);graph.select("a");
    forge_flow_test_require(graph.document_status()["dirty"]==false,"视口或选择污染未保存状态");
    graph.update_data("a",{{"text","新数据"}});graph.mark_saved();
    graph.undo();forge_flow_test_require(graph.document_status()["dirty"]==true,"跨保存点撤销应恢复未保存状态");
    graph.redo();forge_flow_test_require(graph.document_status()["dirty"]==false,"回到保存点没有清除未保存状态");
    forge_flow_test_command(graph,{{"op","drag-begin"},{"id","a"},{"x",20},{"y",40}});
    forge_flow_test_command(graph,{{"op","drag-move"},{"x",85},{"y",40}});
    forge_flow_test_require(graph.document_status()["dirty"]==true,"拖动过程中没有未保存状态");
    forge_flow_test_command(graph,{{"op","mark-saved"}},false);
    forge_flow_test_command(graph,{{"op","cancel"}});
    forge_flow_test_require(graph.document_status()["dirty"]==false,"取消拖动未恢复保存点");
    auto document=nlohmann::json::parse(graph.export_json());document["version"]=1;document["business_extension"]={{"opaque",42}};
    graph.import_json(document.dump());
    forge_flow_test_require(graph.document_status()["loaded_version"]==1&&graph.document_status()["schema_version"]==2&&graph.document_status()["dirty"]==false,"旧版本迁移状态错误");
    auto migrated=nlohmann::json::parse(graph.export_json());
    forge_flow_test_require(migrated["version"]==2&&migrated["business_extension"]["opaque"]==42&&migrated["nodes"][1]["kind"]=="unknown-card","版本迁移丢失扩展或未知卡片");
    graph.patch_node("a",{{"title","已修改"}});const auto before=graph.export_json();const auto status=graph.document_status();
    migrated["version"]=3;forge_flow_test_require(!graph.import_json(migrated.dump())["ok"].get<bool>()&&graph.export_json()==before&&graph.document_status()==status,"不支持版本加载没有完整回滚");
    graph.patch_node("a",{{"title","中文 <节点> & 参数"}});graph.select("a");
    const auto saved=graph.export_json();const auto saved_status=graph.document_status();
    const auto full=graph.export_svg(),selected=graph.export_svg(true);
    forge_flow_test_require(full.find("data-node-id=\"b\"")!=std::string::npos&&full.find("&lt;节点&gt; &amp; 参数")!=std::string::npos&&full.find("&lt;标签&gt;")!=std::string::npos,"整图裁剪到视口或文本没有转义");
    forge_flow_test_require(selected.find("data-node-id=\"b\"")==std::string::npos&&selected.find("data-edge-id=")==std::string::npos,"选区导出了外部节点或连线");
    graph.select("b",true);forge_flow_test_require(graph.export_svg(true).find("data-edge-id=")!=std::string::npos,"选区未保留内部连线");
    int adapted=0;
    const auto custom=graph.export_svg(false,{{"background","none"}},[&](const auto& node){++adapted;return nlohmann::json::array({{{"type","text"},{"text",node["id"].template get<std::string>()+" <adapter>"}},{{"type","rect"},{"width",20},{"height",10},{"fill","#12abef"}}});});
    forge_flow_test_require(adapted==2&&custom.find("&lt;adapter&gt;")!=std::string::npos&&custom.find("#12abef")!=std::string::npos,"自定义卡片导出适配器未执行");
    forge_flow_test_require(graph.export_json()==saved&&graph.document_status()==saved_status,"导出修改文档或历史");
    for(const auto& options:nlohmann::json::array({{{"scale",0}},{{"theme","invalid"}},{{"node_content",{{"a",nlohmann::json::array({{{"type","image"},{"source","https://example.invalid/a.png"},{"width",10},{"height",10}}})}}}}})) {
        bool rejected=false;try{graph.export_svg(false,options);}catch(const std::exception&){rejected=true;}
        forge_flow_test_require(rejected&&graph.export_json()==saved&&graph.document_status()==saved_status,"无效导出选项未原子拒绝");
    }
    graph.patch_node("b",{{"hidden",true}});forge_flow_test_require(graph.export_svg().find("data-node-id=\"b\"")==std::string::npos,"隐藏卡片仍进入导出");
    ForgeFlowDocument tree("mindmap");forge_flow_test_command(tree,{{"op","demo"},{"number",4}});tree.toggle_collapse("node-2");
    forge_flow_test_require(tree.export_svg().find("data-node-id=\"node-5\"")==std::string::npos,"折叠子主题仍进入导出");
    std::cout<<"FLOW_DOCUMENT_EXPORT_OK dirty=save_undo_cancel migration=v1_v2 export=whole_selection_adapter hidden=excluded atomic=invalid\n";
}
