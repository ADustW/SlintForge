#pragma once

static void forge_flow_test_alignment() {
    ForgeFlowDocument graph;
    auto input=nlohmann::json{{"version",1},{"mode","graph"},{"nodes",nlohmann::json::array({
        {{"id","a"},{"x",103},{"y",127},{"width",100},{"height",80}},
        {{"id","b"},{"x",300},{"y",100},{"width",160},{"height",100}},
        {{"id","c"},{"x",700},{"y",220},{"width",120},{"height",60}}
    })},{"edges",nlohmann::json::array()}};
    for(const std::string mode:{"left","right","top","bottom","center-x","center-y","distribute-x","distribute-y"}) {
        // 纵向等距测试采用足够空间；空间不足必须明确拒绝，不能静默重叠。
        auto fixture=input;if(mode=="distribute-y")fixture["nodes"][2]["y"]=600;
        graph.import_json(fixture.dump());const auto before=graph.export_json();graph.apply_checked({{"op","select-all"}});
        forge_flow_test_require(graph.align_selected(mode)["ok"],"对齐或等距排列失败");
        const auto nodes=graph.query("nodes");
        if(mode=="left")for(const auto& node:nodes)forge_flow_test_require(node["x"]==103,"左对齐错误");
        if(mode=="right")for(const auto& node:nodes)forge_flow_test_require(node["x"].get<double>()+node["width"].get<double>()==820,"右对齐错误");
        if(mode=="top")for(const auto& node:nodes)forge_flow_test_require(node["y"]==100,"顶部对齐错误");
        if(mode=="bottom")for(const auto& node:nodes)forge_flow_test_require(node["y"].get<double>()+node["height"].get<double>()==280,"底部对齐错误");
        if(mode=="center-x")for(const auto& node:nodes)forge_flow_test_require(node["x"].get<double>()+node["width"].get<double>()/2==461.5,"水平居中错误");
        if(mode=="center-y")for(const auto& node:nodes)forge_flow_test_require(node["y"].get<double>()+node["height"].get<double>()/2==190,"垂直居中错误");
        if(mode=="distribute-x")forge_flow_test_require(nodes[1]["x"]==371.5&&nodes[2]["x"]==700,"水平等距没有保持两端");
        if(mode=="distribute-y")forge_flow_test_require(nodes[0]["y"]==360&&nodes[1]["y"]==100&&nodes[2]["y"]==600,"垂直等距没有按位置排序");
        graph.undo();forge_flow_test_require(graph.export_json()==before,"对齐不是单步撤销");
    }
    graph.import_json(input.dump());graph.apply_checked({{"op","select-all"}});const auto limited=graph.export_json();
    forge_flow_test_require(!graph.align_selected("distribute-y")["ok"].get<bool>()&&graph.export_json()==limited,"空间不足的等距排列未拒绝");
    graph.patch_node("b",{{"draggable",false}});const auto locked=graph.export_json();forge_flow_test_require(!graph.align_selected("left")["ok"].get<bool>()&&graph.export_json()==locked,"对齐绕过了移动权限");

    graph.import_json(input.dump());graph.configure({{"grid_size",32},{"guides_enabled",false}});
    graph.select("a");graph.apply_checked({{"op","drag-begin"},{"id","a"},{"x",110},{"y",140}});
    auto state=graph.apply_checked({{"op","drag-move"},{"id","a"},{"x",142},{"y",174},{"flag",true}});
    forge_flow_test_require(graph.node_world("a")["x"]==128&&graph.node_world("a")["y"]==160&&state["grid_size"]==32,"吸附按位移而非画布网格坐标计算");
    graph.apply_checked({{"op","cancel"}});forge_flow_test_require(graph.node_world("a")["x"]==103,"取消吸附拖动没有恢复");
    graph.configure({{"grid_size",20},{"guides_enabled",true},{"guide_snap",true},{"guide_tolerance",6}});
    for(const double zoom:{0.5,1.,2.}) {
        graph.set_viewport(40,40,zoom);graph.select("a");graph.apply_checked({{"op","drag-begin"},{"id","a"},{"x",103},{"y",127}});
        // 网格落点 200 与节点 b 的左边相距 100；a 的右边正好与 b 的左边对齐。
        state=graph.apply_checked({{"op","drag-move"},{"id","a"},{"x",198},{"y",101},{"flag",true}});
        forge_flow_test_require(graph.node_world("a")["x"]==200&&graph.node_world("a")["y"]==100&&state["guides"].size()==2,"边缘对齐吸附或辅助线错误");
        graph.apply_checked({{"op","cancel"}});forge_flow_test_require(graph.query("guides").empty(),"取消手势后残留辅助线");
    }
    const auto stable=graph.export_json();forge_flow_test_require(!graph.configure({{"grid_size",0}})["ok"].get<bool>()&&graph.export_json()==stable,"非法网格配置污染文档");
    graph.select("a");graph.select("b",true);graph.apply_checked({{"op","drag-begin"},{"id","a"},{"x",103},{"y",127}});
    graph.apply_checked({{"op","drag-move"},{"id","a"},{"x",135},{"y",161},{"flag",true}});
    forge_flow_test_require(graph.node_world("b")["x"].get<double>()-graph.node_world("a")["x"].get<double>()==197,"多选吸附改变相对位置");graph.apply_checked({{"op","drag-end"}});
    forge_flow_test_require(graph.query("guides").empty(),"提交拖动后残留辅助线");
    std::cout<<"FLOW_ALIGNMENT_OK align=6 distribute=2 permissions=atomic grid=absolute configurable=true guides=screen_tolerance history=single\n";
}
