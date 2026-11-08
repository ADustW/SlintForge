// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

static void forge_flow_test_layout_no_overlap(ForgeFlowDocument& document) {
    const auto nodes=document.query("nodes");
    for(std::size_t i=0;i<nodes.size();++i)for(std::size_t j=i+1;j<nodes.size();++j) {
        const auto& a=nodes[i];const auto& b=nodes[j];if(a["parent"]!=b["parent"])continue;
        const double ax=a["x"],ay=a["y"],aw=a["width"],ah=a["height"],bx=b["x"],by=b["y"],bw=b["width"],bh=b["height"];
        forge_flow_test_require(ax+aw<=bx||bx+bw<=ax||ay+ah<=by||by+bh<=ay,"自动布局后兄弟节点重叠");
    }
}

static void forge_flow_test_general_layout() {
    ForgeFlowDocument graph;
    const auto input=nlohmann::json{{"version",1},{"mode","graph"},{"nodes",nlohmann::json::array({
        {{"id","a"},{"x",30},{"y",40},{"width",200},{"height",100}},
        {{"id","b"},{"x",500},{"y",10},{"width",80},{"height",160}},
        {{"id","c"},{"x",200},{"y",500},{"width",300},{"height",90}},
        {{"id","d"},{"x",600},{"y",500},{"width",160},{"height",70}}
    })},{"edges",nlohmann::json::array({
        {{"id","ab"},{"source_node","a"},{"source_port","out"},{"target_node","b"},{"target_port","in"}},
        {{"id","ac"},{"source_node","a"},{"source_port","out"},{"target_node","c"},{"target_port","in"}},
        {{"id","bd"},{"source_node","b"},{"source_port","out"},{"target_node","d"},{"target_port","in"}},
        {{"id","cd"},{"source_node","c"},{"source_port","out"},{"target_node","d"},{"target_port","in"}}
    })}};
    for(const std::string direction:{"LR","RL","TB","BT"}) {
        graph.import_json(input.dump());const auto before=graph.export_json();
        const auto options=nlohmann::json{{"direction",direction},{"rank_gap",70},{"node_gap",25}};
        forge_flow_test_require(graph.layout_graph(options)["ok"],"分层布局失败");forge_flow_test_layout_no_overlap(graph);
        for(const auto& edge:graph.query("edges")) {
            const auto a=graph.query("node",edge["source_node"]),b=graph.query("node",edge["target_node"]);
            const bool horizontal=direction=="LR"||direction=="RL",reverse=direction=="RL"||direction=="BT";
            const double start=a[horizontal?"x":"y"],end=b[horizontal?"x":"y"],as=a[horizontal?"width":"height"],bs=b[horizontal?"width":"height"];
            forge_flow_test_require(reverse?end+bs+70<=start:start+as+70<=end,"布局没有遵守方向、实际尺寸和层间距");
        }
        const auto first=graph.export_json();graph.layout_graph(options);forge_flow_test_require(graph.export_json()==first,"布局不确定或重复执行导致漂移");
        graph.undo();forge_flow_test_require(graph.export_json()==before,"一次布局没有一次撤销");graph.redo();forge_flow_test_require(graph.export_json()==first,"布局重做不一致");
    }
    graph.import_json(input.dump());graph.patch_node("a",{{"layout_fixed",true}});graph.patch_node("b",{{"draggable",false}});
    graph.layout_graph({{"direction","TB"}});forge_flow_test_layout_no_overlap(graph);
    forge_flow_test_require(graph.node_world("a")["x"]==30&&graph.node_world("a")["y"]==40&&graph.node_world("b")["x"]==500&&graph.node_world("b")["y"]==10,"布局移动了固定或锁定节点");
    graph.import_json(input.dump());graph.connect("d","out","a","in");
    forge_flow_test_require(graph.layout_graph({{"algorithm","layered"}})["ok"],"有环普通图不能执行分层布局");forge_flow_test_layout_no_overlap(graph);
    const auto cyclic=graph.export_json();forge_flow_test_require(!graph.layout_graph({{"algorithm","tree"}})["ok"].get<bool>()&&graph.export_json()==cyclic,"树布局没有原子拒绝环或多父节点");
    graph.import_json(input.dump());const auto diamond=graph.export_json();
    forge_flow_test_require(!graph.layout_graph({{"algorithm","tree"}})["ok"].get<bool>()&&graph.export_json()==diamond,"树布局接受了多父节点");
    graph.remove_edge("cd");graph.add_node({{"id","isolated"},{"width",250},{"height",200}});
    for(const std::string direction:{"LR","RL","TB","BT"}) {
        forge_flow_test_require(graph.layout_graph({{"algorithm","tree"},{"direction",direction}})["ok"],"森林树布局失败");forge_flow_test_layout_no_overlap(graph);
        const auto stable=graph.export_json();graph.layout_graph({{"algorithm","tree"},{"direction",direction}});forge_flow_test_require(graph.export_json()==stable,"树布局不确定");
    }
    const auto group=nlohmann::json{{"version",1},{"mode","graph"},{"nodes",nlohmann::json::array({
        {{"id","g"},{"kind","group"},{"x",400},{"y",300},{"width",800},{"height",600}},
        {{"id","a"},{"parent","g"},{"x",20},{"y",60},{"width",220},{"height",132},{"constrain_parent",true}},
        {{"id","b"},{"parent","g"},{"x",300},{"y",60},{"width",180},{"height",96},{"constrain_parent",true}},
        {{"id","outside"},{"x",1500},{"y",300}}
    })},{"edges",nlohmann::json::array({
        {{"id","ab"},{"source_node","a"},{"source_port","out"},{"target_node","b"},{"target_port","in"}},
        {{"id","be"},{"source_node","b"},{"source_port","out"},{"target_node","outside"},{"target_port","in"}}
    })}};
    graph.import_json(group.dump());const auto group_before=graph.export_json();
    forge_flow_test_require(graph.layout_graph({{"direction","LR"}})["ok"],"递归分组布局失败");forge_flow_test_layout_no_overlap(graph);
    auto a=graph.query("node","a"),b=graph.query("node","b"),g=graph.query("node","g");
    forge_flow_test_require(a["x"]==12&&a["y"].get<double>()>=48&&b["x"].get<double>()>=a["x"].get<double>()+220+80&&g["width"]==504,"组内布局或容器尺寸没有反映内容");
    forge_flow_test_require(graph.node_world("outside")["x"].get<double>()>=g["x"].get<double>()+504+80,"跨层连接没有映射到父级容器布局");
    graph.undo();forge_flow_test_require(graph.export_json()==group_before,"递归布局不是一次撤销");
    graph.patch_node("g",{{"resizable",false}});const auto limited=graph.export_json();
    forge_flow_test_require(!graph.layout_graph({{"rank_gap",1800}})["ok"].get<bool>()&&graph.export_json()==limited,"布局越过不可缩放父级边界");
    forge_flow_test_require(!graph.layout_graph({{"direction","unknown"}})["ok"].get<bool>()&&graph.export_json()==limited,"无效布局选项未原子拒绝");
    std::cout<<"FLOW_LAYOUT_OK layered=cycles tree=forest directions=4 sizes=actual fixed=preserved groups=recursive history=one_step deterministic=true\n";
}
