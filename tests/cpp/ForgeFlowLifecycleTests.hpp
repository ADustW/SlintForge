// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

static void forge_flow_test_keyboard_and_lifecycle() {
    ForgeFlowDocument graph;
    graph.add_node({{"id","a"},{"x",40},{"y",60}});graph.add_node({{"id","b"},{"x",400},{"y",60}});
    graph.add_node({{"id","hidden"},{"hidden",true}});graph.add_node({{"id","locked"},{"selectable",false}});
    graph.connect("a","out","b","in");graph.select("a");graph.mark_saved();
    graph.select_next();forge_flow_test_require(graph.query("selection")["nodes"][0]=="b","键盘顺序选择错误");
    graph.select_next();forge_flow_test_require(graph.query("selection")["nodes"][0]=="a","键盘选择没有跳过隐藏或禁选节点");
    graph.select_next(true);forge_flow_test_require(graph.query("selection")["nodes"][0]=="b"&&graph.document_status()["dirty"]==false,"反向选择或选择历史错误");
    graph.select_next(false,true);forge_flow_test_require(graph.query("selection")["edges"].size()==1,"键盘无法选择连线");
    graph.select("a");graph.select("b",true);const auto before=graph.export_json();
    forge_flow_test_require(graph.nudge_selected(10,-1)["ok"],"键盘移动失败");
    forge_flow_test_require(graph.node_world("a")["x"]==50&&graph.node_world("b")["x"]==410&&graph.node_world("a")["y"]==59,"键盘移动没有保持多选相对位置");
    graph.undo();forge_flow_test_require(graph.export_json()==before&&graph.document_status()["dirty"]==false,"键盘移动没有单步撤销");
    graph.select("a");graph.patch_node("a",{{"draggable",false}});const auto locked=graph.export_json();
    forge_flow_test_require(!graph.nudge_selected(1,0)["ok"].get<bool>()&&graph.export_json()==locked,"键盘绕过拖动权限");
    ForgeFlowDocument history;history.add_node({{"id","n"}});history.mark_saved();
    for(int i=0;i<300;++i)history.update_data("n",{{"counter",i}});
    auto status=history.document_status();forge_flow_test_require(status["undo_count"]==100&&status["redo_count"]==0,"长编辑历史超过 100 步");
    for(int i=0;i<120;++i)history.undo();
    status=history.document_status();forge_flow_test_require(status["undo_count"]==0&&status["redo_count"]==100&&history.query("node","n")["data"]["counter"]==199,"历史裁剪或边界撤销错误");
    for(int i=0;i<120;++i)history.redo();
    forge_flow_test_require(history.query("node","n")["data"]["counter"]==299&&history.document_status()["undo_count"]==100,"长历史重做错误");
    const auto preserved=history.export_json();
    for(int i=0;i<64;++i) {ForgeFlowDocument temporary;forge_flow_test_command(temporary,{{"op","demo"},{"number",i%4},{"count",12}});temporary.select("node-1");temporary.nudge_selected(i,1);}
    forge_flow_test_require(history.export_json()==preserved,"实例反复释放影响其他文档");
    std::cout<<"FLOW_LIFECYCLE_CORE_OK keyboard=visible_order_atomic_move edits=300 history_cap=100 undo_redo=bounded isolated_destroy=64\n";
}
