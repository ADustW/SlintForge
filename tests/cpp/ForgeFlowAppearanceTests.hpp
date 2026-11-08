// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

static void forge_flow_test_advanced_appearance() {
    ForgeFlowDocument graph;
    graph.add_node({{"id","a"},{"x",20},{"y",80}});
    graph.add_node({{"id","b"},{"x",620},{"y",160}});
    graph.connect("a","out","b","in");
    const auto id=graph.query("edges")[0]["id"].get<std::string>();
    const auto original=graph.export_json();
    auto result=graph.update_edge_appearance(id,{{"marker_start","circle"},{"marker_end","open-arrow"},{"animated",true},{"animation_speed",120.},{"curvature",.7},{"layer",12}});
    forge_flow_test_require(result["ok"],"高级连线属性更新失败");
    auto view=nlohmann::json::parse(graph.snapshot())["edges"][0];
    forge_flow_test_require(view["start_marker"]=="circle"&&view["end_marker"]=="open-arrow"&&!view["marker_stroke_path"].get<std::string>().empty()&&!view["arrow_path"].get<std::string>().empty(),"双端标记几何缺失");
    forge_flow_test_require(view["motion_points"].size()==65&&view["motion_points"][0]["x"]==view["source_x"]&&view["motion_points"][64]["x"]==view["target_x"]&&view["path_length"].get<double>()>380,"流动采样没有覆盖实际连线");
    const auto decorated=graph.export_json();graph.undo();forge_flow_test_require(graph.export_json()==original,"高级连线修改未单步撤销");graph.redo();
    ForgeFlowDocument restored;restored.import_json(graph.export_json());forge_flow_test_require(restored.export_json()==decorated,"高级连线保存加载不一致");
    graph.update_edge_appearance(id,{{"curvature",0.}});const auto flat=nlohmann::json::parse(graph.snapshot())["edges"][0]["path"];
    graph.update_edge_appearance(id,{{"curvature",.7}});forge_flow_test_require(nlohmann::json::parse(graph.snapshot())["edges"][0]["path"]!=flat,"曲率没有改变几何路径");
    for(const auto& patch:nlohmann::json::array({{{"curvature",2.1}},{{"animation_speed",0}},{{"marker_end",{{"shape","custom"},{"points",nlohmann::json::array({{{"x",1},{"y",0}},{{"x",0},{"y",0}},{{"x",-1},{"y",1}}})}}}},{{"marker_start",{{"shape","circle"},{"size",100}}}},{{"layer",1501}}})) {
        const auto before=graph.export_json();forge_flow_test_require(!graph.update_edge_appearance(id,patch)["ok"].get<bool>()&&graph.export_json()==before,"无效高级样式未原子拒绝");
    }
    graph.patch_node("b",{{"y",80}});
    graph.update_edge_appearance(id,{{"marker_start","none"},{"marker_end",{{"shape","custom"},{"size",40},{"points",nlohmann::json::array({{{"x",0},{"y",0}},{{"x",-1},{"y",1}},{{"x",-1},{"y",-1}}})}}},{"curvature",0.}});
    forge_flow_test_require(graph.query("edge-at","",{{"x",590},{"y",166},{"width",1}})==id,"自定义填充标记没有参与命中");
    graph.update_edge_appearance(id,{{"arrow",false}});view=nlohmann::json::parse(graph.snapshot())["edges"][0];forge_flow_test_require(view["end_marker"]=="none"&&view["marker_stroke_path"]=="","兼容箭头开关未清除高级终点标记");
    graph.configure({{"allow_parallel_connections",true},{"parallel_spacing",48.}});
    graph.connect("a","out","b","in");const auto second=graph.query("edges")[1]["id"].get<std::string>();
    view=nlohmann::json::parse(graph.snapshot());
    forge_flow_test_require(view["edges"][0]["path"]!=view["edges"][1]["path"]&&view["edges"][0]["parallel_offset"]==-24&&view["edges"][1]["parallel_offset"]==24,"并行边没有使用独立通道");
    const auto before_options=graph.export_json();forge_flow_test_require(!graph.configure({{"allow_parallel_connections",false}})["ok"].get<bool>()&&graph.export_json()==before_options,"关闭并行连接策略没有原子拒绝既有重复边");
    graph.update_edge_appearance(id,{{"parallel_offset",0.},{"layer",0}});graph.update_edge_appearance(second,{{"parallel_offset",0.},{"layer",8}});
    forge_flow_test_require(graph.query("edge-at","",{{"x",430},{"y",146},{"width",2}})==second,"重合连线命中没有遵守显示层级");
    graph.update_edge_appearance(second,{{"layer",-1}});forge_flow_test_require(graph.query("edge-at","",{{"x",430},{"y",146},{"width",2}})==id,"调整层级后命中没有更新");
    // 移除一条边后，自动通道应归中；撤销应恢复两条路径的确定性排序。
    graph.patch_edge(id,{{"parallel_offset",24.}});auto edge=graph.query("edge",id);edge.erase("parallel_offset");graph.apply_checked({{"op","edge"},{"id",id},{"edge",edge}});
    graph.remove_edge(second);view=nlohmann::json::parse(graph.snapshot());forge_flow_test_require(view["edges"][0]["parallel_offset"]==0,"移除并行边没有重新分配通道");
    graph.undo();view=nlohmann::json::parse(graph.snapshot());forge_flow_test_require(view["edges"][0]["parallel_offset"]==-24,"撤销没有恢复并行通道");
    std::cout<<"FLOW_ADVANCED_APPEARANCE_OK markers=both_custom animation=distance_samples parallel=deterministic layer=hit_order history=roundtrip\n";
}
