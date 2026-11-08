// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

static void forge_flow_test_groups() {
    ForgeFlowDocument graph;
    const auto document=nlohmann::json{
        {"version",1},{"mode","graph"},{"viewport",{{"x",40},{"y",40},{"zoom",1}}},
        {"nodes",nlohmann::json::array({
            {{"id","g"},{"kind","group"},{"x",100},{"y",100},{"width",600},{"height",480}},
            {{"id","a"},{"parent","g"},{"x",20},{"y",60},{"width",180},{"height",90},{"constrain_parent",true}},
            {{"id","h"},{"kind","group"},{"parent","g"},{"x",300},{"y",60},{"width",220},{"height",200},{"constrain_parent",true}},
            {{"id","b"},{"parent","h"},{"x",20},{"y",50},{"width",140},{"height",70},{"draggable",false},{"constrain_parent",true}},
            {{"id","c"},{"x",1000},{"y",300},{"width",180},{"height",90}}
        })},
        {"edges",nlohmann::json::array({
            {{"id","ac"},{"source_node","a"},{"source_port","out"},{"target_node","c"},{"target_port","in"}},
            {{"id","ba"},{"source_node","b"},{"source_port","out"},{"target_node","a"},{"target_port","in"},{"path_kind","polyline"},{"control_points",nlohmann::json::array({{{"x",500},{"y",340}},{{"x",100},{"y",340}}})}}
        })}
    };
    forge_flow_test_command(graph,{{"op","load"},{"text",document.dump()}});
    const auto baseline=graph.export_json();
    auto a=graph.query("node-world","a"),b=graph.query("node-world","b");
    forge_flow_test_require(a["x"]==120&&a["y"]==160&&b["x"]==420&&b["y"]==210,"嵌套世界坐标错误");
    const auto snapshot=nlohmann::json::parse(graph.snapshot());
    forge_flow_test_require(snapshot["edges"][0]["source_x"]==300,"分组端口没有采用世界坐标");
    graph.select("g");graph.select("a",true);graph.select("b",true);
    forge_flow_test_command(graph,{{"op","drag-begin"},{"id","g"},{"x",100},{"y",100}});
    auto moved=forge_flow_test_command(graph,{{"op","drag-move"},{"x",140},{"y",130}});
    forge_flow_test_require(moved["nodes"].size()==4,"分组拖动没有更新所有后代的显示行");
    b=graph.query("node-world","b");
    forge_flow_test_require(b["x"]==460&&b["y"]==240&&graph.query("node","b")["x"]==20,"锁定后代未跟随容器或父子同时选中导致双重移动");
    forge_flow_test_require(graph.query("edge","ba")["control_points"][0]["x"]==540,"组内手动控制点没有同步平移");
    forge_flow_test_command(graph,{{"op","drag-end"}});graph.undo();
    forge_flow_test_require(graph.export_json()==baseline,"分组移动不是一步撤销");
    forge_flow_test_command(graph,{{"op","collapse"},{"id","h"}});
    auto folded=nlohmann::json::parse(graph.snapshot());
    for(const auto& n:folded["nodes"]){if(n["id"]=="b")forge_flow_test_require(n["hidden"]==true,"折叠未隐藏后代");if(n["id"]=="h")forge_flow_test_require(n["height"]==48,"折叠容器未使用紧凑高度");}
    forge_flow_test_require(graph.query("node","h")["height"]==200&&graph.query("nodes").size()==5,"折叠污染数据或丢失原始尺寸");
    graph.undo();forge_flow_test_require(graph.export_json()==baseline,"折叠撤销未恢复全部数据");
    forge_flow_test_command(graph,{{"op","reparent"},{"id","b"},{"target","g"}});
    forge_flow_test_require(graph.query("node","b")["x"]==320&&graph.query("node-world","b")["x"]==420,"改父级没有保持世界位置");
    graph.undo();
    forge_flow_test_command(graph,{{"op","reparent"},{"id","g"},{"target","h"}},false);
    forge_flow_test_command(graph,{{"op","reparent"},{"id","a"},{"target","c"}},false);
    forge_flow_test_command(graph,{{"op","node-patch"},{"id","a"},{"patch",{{"x",-40}}}},false);
    forge_flow_test_require(graph.export_json()==baseline,"非法父级或边界操作污染文档");
    graph.select("a");forge_flow_test_command(graph,{{"op","drag-begin"},{"id","a"},{"x",120},{"y",160}});
    forge_flow_test_command(graph,{{"op","drag-move"},{"x",-200},{"y",-200}});
    forge_flow_test_require(graph.query("node","a")["x"]==12&&graph.query("node","a")["y"]==48,"子节点拖动没有遵守父级内容边界");
    forge_flow_test_command(graph,{{"op","cancel"}});
    forge_flow_test_command(graph,{{"op","resize-begin"},{"id","g"},{"number",3},{"x",700},{"y",580}});
    forge_flow_test_command(graph,{{"op","resize-move"},{"x",200},{"y",200}});
    auto resized=graph.query("node","g");forge_flow_test_require(resized["width"].get<double>()>=532&&resized["height"].get<double>()>=272,"缩小分组后约束子节点超出边界");
    forge_flow_test_command(graph,{{"op","cancel"}});
    forge_flow_test_command(graph,{{"op","ungroup"},{"id","h"}});
    forge_flow_test_require(graph.query("node","b")["parent"]=="g"&&graph.query("node-world","b")["x"]==420&&graph.query("edges").size()==2,"解除嵌套分组丢失子节点或内部连线");
    graph.undo();forge_flow_test_require(graph.export_json()==baseline,"解除分组撤销失败");
    graph.select("a");graph.select("h",true);forge_flow_test_command(graph,{{"op","group"},{"text","新容器"}});
    const auto selection=graph.query("selection");const auto group_id=selection["nodes"][0].get<std::string>();
    forge_flow_test_require(graph.query("node","a")["parent"]==group_id&&graph.query("node-world","a")["x"]==120&&graph.query("node-world","b")["x"]==420,"创建嵌套分组改变了节点位置");
    graph.undo();forge_flow_test_require(graph.export_json()==baseline,"创建分组撤销失败");
    // 被目标分组边界拒绝时，整次手势回退并释放拖动状态，后续编辑仍可继续。
    graph.patch_node("c",{{"constrain_parent",true}});const auto constrained=graph.export_json();
    graph.select("c");forge_flow_test_command(graph,{{"op","drag-begin"},{"id","c"},{"x",1000},{"y",300}});
    forge_flow_test_command(graph,{{"op","drag-move"},{"x",430},{"y",230}});
    forge_flow_test_command(graph,{{"op","drag-end"},{"flag",true}},false);
    forge_flow_test_require(graph.export_json()==constrained,"非法拖入没有恢复手势前的数据");
    forge_flow_test_command(graph,{{"op","node-patch"},{"id","c"},{"patch",{{"constrain_parent",false}}}});
    graph.select("c");forge_flow_test_command(graph,{{"op","drag-begin"},{"id","c"},{"x",1000},{"y",300}});
    forge_flow_test_command(graph,{{"op","drag-move"},{"x",430},{"y",230}});
    forge_flow_test_command(graph,{{"op","drag-end"},{"flag",true}});
    forge_flow_test_require(graph.query("node","c")["parent"]=="h"&&graph.query("node-world","c")["x"]==430,"拖入最深分组失败");
    forge_flow_test_command(graph,{{"op","drag-begin"},{"id","c"},{"x",430},{"y",230}});
    forge_flow_test_command(graph,{{"op","drag-move"},{"x",1100},{"y",700}});
    forge_flow_test_command(graph,{{"op","drag-end"},{"flag",true}});
    forge_flow_test_require(graph.query("node","c")["parent"]==""&&graph.query("node-world","c")["x"]==1100,"拖出分组没有恢复根坐标");
    ForgeFlowDocument loaded;loaded.import_json(graph.export_json());
    forge_flow_test_require(loaded.export_json()==graph.export_json(),"分组保存加载不一致");
    std::cout<<"FLOW_GROUPS_OK local_world=nested drag=once_locked_child route=ports_control_points hierarchy=atomic collapse=restore bounds=constrained drop=reparent history=single\n";
}
