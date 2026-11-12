// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

static void forge_flow_test_editor_refinement() {
    ForgeFlowDocument graph;
    graph.add_node({{"id","source"},{"x",0},{"y",0}});
    graph.add_node({{"id","target"},{"x",420},{"y",200}});
    const auto connected=graph.connect("source","out","target","in",3);
    forge_flow_test_require(connected["ok"].get<bool>(),"圆角折线不能创建");
    const std::string edge_id=connected["edges"][0]["id"];
    forge_flow_test_clear(graph,edge_id);
    const auto original=graph.export_json();
    const auto invalid=graph.update_edge_appearance(edge_id,{{"stroke_color","#zz0102"},{"dashed",true}});
    forge_flow_test_require(!invalid["ok"].get<bool>()&&graph.export_json()==original,"非法颜色必须原子拒绝");
    graph.update_edge_appearance(edge_id,{{"stroke_color","#7C5CE7"},{"dashed",true}});
    const auto edge=forge_flow_test_edge(graph,edge_id);
    forge_flow_test_require(edge["color_red"]==124&&edge["color_green"]==92&&edge["color_blue"]==231&&edge["dashed"]==true,"颜色未解码到绘制模型");
    forge_flow_test_require(edge["path"].get<std::string>().find(" M ")!=std::string::npos,"虚线未分段");
    const auto hit=forge_flow_test_command(graph,{{"op","pick"},{"x",edge["label_x"]},{"y",edge["label_y"]}});
    forge_flow_test_require(hit["selected_edge_count"]==1,"虚线间隙应保持可选中");
    const auto styled=graph.export_json();ForgeFlowDocument loaded;loaded.import_json(styled);
    forge_flow_test_require(loaded.export_json()==styled,"外观存储未保留");
    graph.undo();forge_flow_test_require(graph.export_json()==original,"颜色及虚线未合并到一步历史");graph.redo();
    for(int corner=0;corner<4;++corner) {
        const auto before=graph.export_json();
        forge_flow_test_command(graph,{{"op","resize-begin"},{"id","source"},{"number",corner},{"x",0},{"y",0}});
        const auto moved=forge_flow_test_command(graph,{{"op","resize-move"},{"x",corner%2?40:-40},{"y",corner>=2?30:-30}});
        forge_flow_test_require(moved["nodes"].size()==1&&moved["nodes"][0]["width"]==260&&moved["nodes"][0]["height"]==162,"四角缩放增量数据错误");
        forge_flow_test_require(moved["edges"].size()==1,"缩放未更新端口连线");
        forge_flow_test_command(graph,{{"op","resize-end"}});
        graph.undo();forge_flow_test_require(graph.export_json()==before,"连续缩放不是一步撤销");
    }
    forge_flow_test_command(graph,{{"op","resize-begin"},{"id","source"},{"number",3},{"width",160},{"height",112}});
    forge_flow_test_command(graph,{{"op","resize-move"},{"x",-800},{"y",-800}});
    auto resized=nlohmann::json::parse(graph.export_json())["nodes"][0];
    forge_flow_test_require(resized["width"]==160&&resized["height"]==112,"最小尺寸未限制");
    forge_flow_test_command(graph,{{"op","cancel"}});forge_flow_test_require(graph.export_json()==styled,"取消缩放没有恢复原文档");
    auto duplicate=nlohmann::json::parse(graph.execute(R"({"op":"resolve","id":"source","port":"out","x":420,"y":266})"));
    forge_flow_test_require(duplicate["op"]=="","重复连线仍显示有效预览");
    graph.add_node({{"id","other"},{"x",700},{"y",400}});
    auto candidate=nlohmann::json::parse(graph.execute(R"({"op":"resolve","id":"source","port":"out","x":704,"y":462})"));
    forge_flow_test_require(candidate["op"]=="connect"&&candidate["x"]==700&&candidate["y"]==466,"端口预检没有返回精确吸附位置");
    std::cout << "FLOW_EDITOR_OK resize=4_corners_undo_cancel_minimum appearance=color_dash_smoothstep preview=validated_snap\n";
}
