// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

static void forge_flow_test_clipboard() {
    ForgeFlowDocument source,target;
    const auto input=nlohmann::json{{"version",1},{"mode","graph"},{"nodes",nlohmann::json::array({
        {{"id","g"},{"kind","group"},{"title","G"},{"x",100},{"y",200},{"width",620},{"height",480}},
        {{"id","a"},{"title","A"},{"parent","g"},{"x",20},{"y",60},{"width",180},{"height",100},{"constrain_parent",true},{"data",{{"text","中文表单"}}}},
        {{"id","h"},{"title","H"},{"kind","group"},{"parent","g"},{"x",300},{"y",60},{"width",260},{"height",260},{"collapsed",true}},
        {{"id","b"},{"title","B"},{"parent","h"},{"x",20},{"y",60},{"width",160},{"height",100},{"constrain_parent",true}},
        {{"id","outside"},{"title","外部"},{"x",900},{"y",200}}
    })},{"edges",nlohmann::json::array({
        {{"id","ab"},{"source_node","a"},{"source_port","out"},{"target_node","b"},{"target_port","in"},{"kind","custom-bus"},{"data",{{"channel",3}}},{"path_kind","polyline"},{"control_points",nlohmann::json::array({{{"x",600},{"y",500}},{{"x",500},{"y",500}}})}},
        {{"id","be"},{"source_node","b"},{"source_port","out"},{"target_node","outside"},{"target_port","in"}}
    })}};
    forge_flow_test_require(source.import_json(input.dump())["ok"],"剪贴板场景加载失败");
    source.select("g");source.select("a",true);
    const auto baseline=source.export_json();const auto fragment=source.selection_fragment();
    forge_flow_test_require(fragment["nodes"].size()==4&&fragment["edges"].size()==1,"片段未包含隐藏后代或错误包含外部连线");
    source.copy_selection();
    forge_flow_test_require(source.export_json()==baseline&&!nlohmann::json::parse(source.snapshot())["can_undo"].get<bool>(),"复制修改了文档或编辑历史");
    target.add_node({{"id","node-1"},{"x",-300}});const auto target_before=target.export_json();
    forge_flow_test_require(target.paste_clipboard(800,1000)["ok"],"跨文档粘贴失败");
    const auto pasted=target.query("nodes");std::map<std::string,std::string> ids;
    for(const auto& node:pasted)if(node["id"]!="node-1")ids[node["title"].get<std::string>()]=node["id"].get<std::string>();
    forge_flow_test_require(pasted.size()==5&&ids.size()==4&&ids["G"]!="g","粘贴没有重新分配唯一 ID");
    forge_flow_test_require(target.query("node",ids["A"])["parent"]==ids["G"]&&target.query("node",ids["B"])["parent"]==ids["H"],"粘贴父级引用未重建");
    forge_flow_test_require(target.node_world(ids["A"])["x"]==820&&target.node_world(ids["B"])["x"]==1120,"粘贴局部与世界坐标错误");
    auto edge=target.query("edges")[0];
    forge_flow_test_require(edge["source_node"]==ids["A"]&&edge["target_node"]==ids["B"]&&edge["source_port"]=="out"&&edge["target_port"]=="in"&&edge["kind"]=="custom-bus"&&edge["data"]["channel"]==3,"粘贴丢失端口引用或业务数据");
    forge_flow_test_require(edge["control_points"][0]["x"]==1300&&edge["control_points"][0]["y"]==1300,"粘贴手动控制点没有按世界坐标平移");
    const auto pasted_text=target.export_json();target.undo();forge_flow_test_require(target.export_json()==target_before,"粘贴不是一步撤销");target.redo();forge_flow_test_require(target.export_json()==pasted_text,"粘贴重做不一致");
    forge_flow_test_require(source.export_json()==baseline,"粘贴污染源文档");
    auto invalid=fragment;invalid["nodes"][1]["parent"]="absent";
    forge_flow_test_require(!target.paste_fragment(invalid,0,0)["ok"].get<bool>()&&target.export_json()==pasted_text,"非法片段没有原子拒绝");
    forge_flow_test_command(target,{{"op","batch"},{"commands",nlohmann::json::array({{{"op","paste"},{"fragment",fragment}},{{"op","node-patch"},{"id","absent"},{"patch",{{"x",0}}}}})}},false);
    forge_flow_test_require(target.export_json()==pasted_text,"包含粘贴的失败事务没有回滚");
    forge_flow_test_command(source,{{"op","duplicate"}});
    forge_flow_test_require(source.query("nodes").size()==9&&source.query("edges").size()==3,"复制分组未包含子树和内部连线");
    forge_flow_test_require(source.query("edges")[2]["control_points"][0]["x"]==632,"就地复制手动连线没有偏移");
    source.undo();source.select("g");forge_flow_test_require(source.cut_selection()["ok"],"剪切分组失败");
    forge_flow_test_require(source.query("nodes").size()==1&&source.query("edges").empty(),"剪切残留后代或悬空连线");
    source.undo();forge_flow_test_require(source.export_json()==baseline,"剪切撤销没有恢复外部连线");
    source.select("outside");source.copy_selection();source.patch_node("b",{{"deletable",false}});source.select("g");
    const auto locked=source.export_json();forge_flow_test_require(!source.cut_selection()["ok"].get<bool>()&&source.export_json()==locked,"剪切绕过后代删除权限");
    ForgeFlowDocument clipboard_probe;clipboard_probe.paste_clipboard(0,0);
    forge_flow_test_require(clipboard_probe.query("nodes").size()==1&&clipboard_probe.query("nodes")[0]["title"]=="外部","失败剪切覆盖了共享剪贴板");
    forge_flow_test_command(source,{{"op","batch"},{"commands",nlohmann::json::array({{{"op","copy"}}})}},false);

    // 脱离原分组复制子节点时，片段根采用世界坐标；向新分组粘贴再转换为局部坐标。
    source.select("a");auto child=source.selection_fragment();
    forge_flow_test_require(child["nodes"][0]["x"]==120&&child["nodes"][0]["parent"]=="","子节点片段根没有转换为世界坐标");
    target.add_node({{"id","destination"},{"kind","group"},{"x",100},{"y",100},{"width",400},{"height",300}});
    forge_flow_test_require(target.paste_fragment(child,120,160,"destination")["ok"],"指定父级粘贴失败");
    const auto child_id=target.query("selection")["nodes"][0].get<std::string>();
    forge_flow_test_require(target.query("node",child_id)["x"]==20&&target.node_world(child_id)["x"]==120,"父级局部坐标粘贴错误");
    const auto bounded=target.export_json();forge_flow_test_require(!target.paste_fragment(child,0,0,"destination")["ok"].get<bool>()&&target.export_json()==bounded,"粘贴越过父级约束");

    ForgeFlowDocument mind("mindmap"),other_mind("mindmap");
    forge_flow_test_command(mind,{{"op","demo"},{"number",4}});mind.select("node-2");const auto branch=mind.selection_fragment();
    forge_flow_test_require(other_mind.paste_fragment(branch,0,0)["ok"],"思维导图分支粘贴到空文档失败");
    const auto root_id=other_mind.query("selection")["nodes"][0].get<std::string>();const auto count=other_mind.query("nodes").size();
    forge_flow_test_require(other_mind.paste_fragment(branch,0,0,root_id)["ok"]&&other_mind.query("nodes").size()==count*2,"思维导图分支粘贴没有生成父子连接");
    other_mind.undo();forge_flow_test_require(other_mind.query("nodes").size()==count,"思维导图粘贴不是一步撤销");
    forge_flow_test_require(!target.paste_fragment(branch,0,0)["ok"].get<bool>(),"图模式不兼容片段未拒绝");
    std::cout<<"FLOW_CLIPBOARD_OK copy=read_only cut=atomic_permissions paste=cross_document group=subtree_hidden ports=remapped bends=translated history=one_step tree=branches\n";
}
