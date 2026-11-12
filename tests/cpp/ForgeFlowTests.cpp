// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeFlow.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <cmath>

static void forge_flow_test_require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
static nlohmann::json forge_flow_test_command(ForgeFlowDocument& document, nlohmann::json action, bool expected = true) {
    auto result = nlohmann::json::parse(document.execute(action.dump()));
    if (result.at("ok").get<bool>() != expected) throw std::runtime_error(action.dump() + " : " + result.dump());
    return result;
}
#include "ForgeFlowRoutingTests.hpp"
#include "ForgeFlowDenseRoutingTests.hpp"
#include "ForgeFlowEditorTests.hpp"
#include "ForgeFlowPoliciesTests.hpp"
#include "ForgeFlowGroupsTests.hpp"
#include "ForgeFlowClipboardTests.hpp"
#include "ForgeFlowLayoutTests.hpp"
#include "ForgeFlowAlignmentTests.hpp"
#include "ForgeFlowNavigationTests.hpp"
#include "ForgeFlowEditorComponentsTests.hpp"
#include "ForgeFlowAppearanceTests.hpp"
#include "ForgeFlowDocumentTests.hpp"
#include "ForgeFlowLifecycleTests.hpp"
#include "ForgeFlowHistoryTests.hpp"
static void forge_flow_test_mindmap_edge_patches() {
    ForgeFlowDocument tree("mindmap"), restored("mindmap");
    forge_flow_test_command(tree, {{"op","demo"},{"number",4}});
    forge_flow_test_require(tree.patch_edge("tree:node-2", {{"label","保留连线属性"}})["ok"].get<bool>(), "合法连线补丁被拒绝");
    const auto decorated = tree.export_json();
    tree.undo();
    tree.select("tree:node-2");
    const auto before = tree.export_json();
    const auto status = tree.document_status();
    const auto selection = tree.query("selection");
    // 非法端点补丁必须保持文档、选区、保存状态及重做历史不变。
    for (const auto& patch : nlohmann::json::array({{{"source_node","node-3"},{"label","不应保留"}},{{"target_node","node-5"}}})) {
        forge_flow_test_require(!tree.patch_edge("tree:node-2", patch)["ok"].get<bool>(), "连线补丁绕过思维导图父级约束");
        forge_flow_test_command(tree, {{"op","edge-patch"},{"id","tree:node-2"},{"text",patch.dump()}}, false);
        forge_flow_test_require(tree.export_json()==before && tree.document_status()==status && tree.query("selection")==selection, "非法连线补丁未完整回滚");
    }
    forge_flow_test_require(restored.import_json(tree.export_json())["ok"].get<bool>() && restored.export_json()==before, "拒绝补丁后文档无法重新加载");
    tree.redo();
    forge_flow_test_require(tree.export_json()==decorated, "拒绝补丁破坏重做历史");
    forge_flow_test_require(restored.import_json(decorated)["ok"].get<bool>() && restored.export_json()==decorated, "合法连线补丁无法保存加载");
    std::cout << "FLOW_MINDMAP_EDGE_PATCH_OK invalid=atomic valid=roundtrip history=preserved\n";
}
int main() {
    try {
        forge_flow_test_mindmap_edge_patches();
        forge_flow_test_routing_and_appearance();
        forge_flow_test_dense_routing();
        forge_flow_test_editor_refinement();
        forge_flow_test_policies_and_transactions();
        forge_flow_test_groups();
        forge_flow_test_clipboard();
        forge_flow_test_general_layout();
        forge_flow_test_alignment();
        forge_flow_test_navigation();
        forge_flow_test_editor_components();
        forge_flow_test_advanced_appearance();
        forge_flow_test_documents_and_export();
        forge_flow_test_keyboard_and_lifecycle();
        forge_flow_test_incremental_history();
        ForgeFlowDocument graph, other;
        forge_flow_test_command(graph, {{"op","demo"},{"number",1}});
        auto original = graph.export_json();
        auto full = nlohmann::json::parse(graph.snapshot());
        forge_flow_test_require(full["nodes"].size() == 6 && full["edges"].size() == 5, "初始模型");
        forge_flow_test_command(graph, {{"op","connect"},{"id","node-1"},{"port","in"},{"target","node-3"},{"target_port","out"}}, false);
        forge_flow_test_command(graph, {{"op","connect"},{"id","node-1"},{"port","out"},{"target","node-1"},{"target_port","in"}}, false);
        forge_flow_test_command(graph, {{"op","connect"},{"id","node-1"},{"port","out"},{"target","node-2"},{"target_port","in"}}, false);
        forge_flow_test_require(graph.export_json() == original, "非法连线污染文档");
        forge_flow_test_command(graph, {{"op","connect"},{"id","node-1"},{"port","out"},{"target","node-3"},{"target_port","in"},{"number",8}}, false);
        forge_flow_test_command(graph, {{"op","connect"},{"id","node-1"},{"port","out"},{"target","node-3"},{"target_port","in"},{"number",1.5}}, false);
        forge_flow_test_require(graph.export_json() == original, "非法线型污染文档");
        forge_flow_test_command(graph, {{"op","select"},{"id","node-1"}});
        forge_flow_test_command(graph, {{"op","select"},{"id","node-2"},{"additive",true}});
        forge_flow_test_command(graph, {{"op","drag-begin"},{"id","node-1"},{"x",60},{"y",60}});
        for (int i=0;i<25;++i) forge_flow_test_command(graph, {{"op","drag-move"},{"x",60+i*2},{"y",80+i}});
        forge_flow_test_command(graph, {{"op","drag-end"}});
        auto moved = nlohmann::json::parse(graph.export_json());
        forge_flow_test_require(moved["nodes"][0]["x"] == 108 && moved["nodes"][1]["x"] == 398, "多节点拖动");
        forge_flow_test_command(graph, {{"op","undo"}});
        forge_flow_test_require(graph.export_json()==original, "一次拖动必须一步撤销");
        forge_flow_test_command(graph, {{"op","redo"}});
        forge_flow_test_command(graph, {{"op","drag-begin"},{"id","node-1"},{"x",0},{"y",0}});
        forge_flow_test_command(graph, {{"op","drag-move"},{"x",800},{"y",500}});
        forge_flow_test_command(graph, {{"op","cancel"}});
        forge_flow_test_require(nlohmann::json::parse(graph.export_json()) == moved, "取消拖动恢复");
        forge_flow_test_command(graph, {{"op","field"},{"id","node-1"},{"target","text"},{"text","中文 🙂 自定义"}});
        forge_flow_test_command(graph, {{"op","view"},{"x",21},{"y",35},{"zoom",2.5}});
        auto saved = graph.export_json();
        forge_flow_test_command(other, {{"op","load"},{"text",saved}});
        forge_flow_test_require(other.export_json()==saved, "双文档保存加载");
        auto invalid=nlohmann::json::parse(saved);invalid["nodes"][1]["id"]="node-1";
        forge_flow_test_command(other, {{"op","load"},{"text",invalid.dump()}},false);
        forge_flow_test_require(other.export_json()==saved, "错误加载不是原子的");
        forge_flow_test_command(other, {{"op","load"},{"text","{bad json"}},false);
        forge_flow_test_require(other.export_json()==saved, "语法错误的 JSON 加载污染文档");
        forge_flow_test_command(graph, {{"op","duplicate"}});
        forge_flow_test_command(graph, {{"op","delete"}});
        forge_flow_test_require(other.export_json()==saved, "文档实例串扰");
        forge_flow_test_command(graph, {{"op","undo"}});
        forge_flow_test_require(nlohmann::json::parse(graph.snapshot())["node_count"] == 8, "复制删除的历史");
        ForgeFlowDocument tree("mindmap");
        forge_flow_test_command(tree, {{"op","demo"},{"number",4}});
        auto tree_original=tree.export_json();
        forge_flow_test_command(tree, {{"op","reconnect"},{"text","tree:node-2"},{"id","node-3"},{"port","right"},{"target","node-2"},{"target_port","left"}},false);
        forge_flow_test_require(tree.export_json()==tree_original, "重连绕过树父级约束");
        forge_flow_test_command(tree, {{"op","reparent"},{"id","node-2"},{"target","node-5"}},false);
        forge_flow_test_require(tree.export_json()==tree_original, "树循环污染文档");
        auto folded=forge_flow_test_command(tree, {{"op","collapse"},{"id","node-2"}});
        auto tree_state=nlohmann::json::parse(tree.snapshot());
        int hidden=0;for(const auto& n:tree_state["nodes"])if(n["hidden"].get<bool>())++hidden;
        forge_flow_test_require(hidden==2 && tree_state["node_count"]==9, "折叠删除了数据或数量错误");
        forge_flow_test_command(tree, {{"op","undo"}});
        forge_flow_test_require(tree.export_json()==tree_original, "树布局撤销");
        forge_flow_test_command(tree, {{"op","add-child"},{"id","node-2"},{"text","新增子主题"}});
        auto added=nlohmann::json::parse(tree.snapshot());forge_flow_test_require(added["node_count"]==10&&added["edge_count"]==9,"树新增子主题");
        forge_flow_test_command(tree, {{"op","layout"}});auto layout_once=tree.export_json();forge_flow_test_command(tree, {{"op","layout"}});forge_flow_test_require(tree.export_json()==layout_once,"布局不确定");
        auto tree_data=nlohmann::json::parse(tree.export_json());auto decorated=tree_data["edges"][0];decorated["label"]="保留业务连线标签";decorated["style"]=2;decorated["arrow"]=true;
        forge_flow_test_command(tree, {{"op","edge"},{"id",decorated["id"]},{"edge",decorated}});
        const auto decorated_saved=tree.export_json();ForgeFlowDocument restored_tree("mindmap");restored_tree.import_json(decorated_saved);
        forge_flow_test_require(restored_tree.export_json()==decorated_saved,"思维导图加载丢失连线属性");
        forge_flow_test_command(tree, {{"op","layout"}});
        forge_flow_test_require(tree.export_json()==decorated_saved,"重新布局丢失连线属性");
        ForgeFlowDocument ports;
        auto custom_node=nlohmann::json{{"id","custom"},{"kind","unknown-business-card"},{"title","自定义"},{"data",{{"binary-reference","image://sample"}}},{"ports",nlohmann::json::array({{{"id","a"},{"side",3},{"offset",.25},{"direction",1}},{{"id","b"},{"side",1},{"offset",.75},{"direction",1}}})}};
        forge_flow_test_require(ports.add_node(custom_node)["ok"].get<bool>(),"自定义多端口插入");
        ports.add_node({{"id","target"},{"x",400},{"y",100}});ports.connect("custom","a","target","in");
        auto resized=nlohmann::json::parse(ports.export_json())["nodes"][0];resized["width"]=400;resized["height"]=240;ports.update_node("custom",resized);
        auto port_state=nlohmann::json::parse(ports.snapshot());forge_flow_test_require(port_state["edges"][0]["source_x"]==100&&port_state["edges"][0]["source_y"]==240,"调整卡片大小后端口没有更新");
        auto unchanged=ports.export_json();auto proposal=nlohmann::json::parse(ports.execute(R"({"op":"resolve","id":"custom","port":"b","x":400,"y":166})"));
        forge_flow_test_require(proposal["target"]=="target"&&proposal["target_port"]=="in"&&ports.export_json()==unchanged,"业务预检修改文档或端点解析错误");
        auto bad_version=nlohmann::json::parse(unchanged);bad_version["version"]=1.5;forge_flow_test_require(!ports.import_json(bad_version.dump())["ok"].get<bool>()&&ports.export_json()==unchanged,"非法文档版本未原子拒绝");
        forge_flow_test_command(ports,{{"op","node"},{"id","custom"},{"text",resized.dump()}});
        auto port_edge=nlohmann::json::parse(ports.export_json())["edges"][0];port_edge["label"]="Slint 完整属性提交";port_edge["style"]=2;
        forge_flow_test_command(ports,{{"op","edge"},{"id",port_edge["id"]},{"text",port_edge.dump()}});
        forge_flow_test_require(nlohmann::json::parse(ports.export_json())["edges"][0]==port_edge,"Slint 连线属性提交失败");
        ForgeFlowDocument big;forge_flow_test_command(big, {{"op","demo"},{"count",500}});
        auto initial=nlohmann::json::parse(big.snapshot());forge_flow_test_require(initial["node_count"]==500&&initial["edge_count"]==1000,"性能数据规模");
        forge_flow_test_command(big,{{"op","drag-begin"},{"id","node-1"}});
        std::vector<double> durations;
        for(int i=0;i<100;++i){auto start=std::chrono::steady_clock::now();auto state=forge_flow_test_command(big,{{"op","drag-move"},{"x",i*2},{"y",i}});durations.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());forge_flow_test_require(state["nodes"].size()<=1&&state["edges"].size()<=32,"拖动未局部刷新相邻或受遮挡连线");}
        forge_flow_test_command(big,{{"op","drag-end"}});std::sort(durations.begin(),durations.end());
        std::cout << "FLOW_CORE_OK validation=atomic history=transaction data=roundtrip tree=layout_collapse_cycle models=isolated nodes=500 edges=1000 core_p95_ms=" << durations[94] << '\n';
    } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
