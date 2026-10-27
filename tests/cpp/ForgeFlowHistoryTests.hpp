#pragma once
#include <unordered_map>
#include <set>
using ForgeFlowJson = nlohmann::json;
#include "ForgeFlowHistory.hpp"

static void forge_flow_test_incremental_history() {
    // 直接验证存储算法中的稳定 ID、插入位置和重排，避免数组下标差量复制后续元素。
    ForgeFlowHistory history;
    auto original = ForgeFlowJson{{"nodes",ForgeFlowJson::array()},{"edges",ForgeFlowJson::array()},{"viewport",{{"zoom",1}}},{"extension",{{"value",nullptr}}}};
    for(int i=0;i<1000;++i)original["nodes"].push_back({{"id",std::to_string(i)},{"data",{{"unchanged",std::string(1000,'x')},{"v",i}}}});
    auto deleted=original;deleted["nodes"].erase(deleted["nodes"].begin());
    history.record(original,deleted);
    forge_flow_test_require(history.status()["history_bytes"].get<std::size_t>()<2048,"删除首节点复制了未修改节点");
    auto restored=deleted;history.restore(restored,false);
    forge_flow_test_require(restored==original,"稳定 ID 删除撤销丢失位置或数据");
    const auto bytes=history.status()["history_bytes"];history.restore(restored,true);
    forge_flow_test_require(restored==deleted&&history.status()["history_bytes"]==bytes,"重做复制或破坏差量");
    auto reordered=deleted;
    std::reverse(reordered["nodes"].begin(),reordered["nodes"].end());
    reordered["nodes"].erase(reordered["nodes"].begin()+5);
    reordered["nodes"].insert(reordered["nodes"].begin()+3,ForgeFlowJson{{"id","new"},{"data",{{"array",{1,nullptr,"中文"}}}}});
    reordered["nodes"][10]["data"]["v"]=-1;reordered["extension"]["escaped/key~"]={true,false};
    history.record(deleted,reordered);restored=reordered;history.restore(restored,false);
    forge_flow_test_require(restored==deleted,"重排与增删组合撤销失败");
    history.restore(restored,true);forge_flow_test_require(restored==reordered,"重排与增删组合重做失败");

    ForgeFlowDocument graph;
    graph.add_node({{"id","n"},{"kind","unknown-card"},{"data",{{"counter","0"},{"nested",{{"a/b~",{1,nullptr,"中文🙂"}}}}}}});
    graph.import_json(graph.export_json());
    for(int i=1;i<=5;++i)forge_flow_test_command(graph,{{"op","field"},{"id","n"},{"target","counter"},{"text",std::to_string(i)}});
    const auto budget=graph.history_status()["history_bytes"].get<std::size_t>()/5*2;
    const auto saved=graph.export_json();graph.mark_saved();
    graph.configure_history(100,budget);
    auto status=graph.history_status();
    forge_flow_test_require(status["undo_count"]==2&&status["history_bytes"].get<std::size_t>()<=budget&&status["history_evicted"]==3,"字节预算未裁剪最早历史");
    graph.set_viewport(25,60,2);const auto viewport=ForgeFlowJson::parse(graph.export_json())["viewport"];
    graph.undo();graph.undo();graph.undo();
    forge_flow_test_require(graph.query("node","n")["data"]["counter"]=="3"&&graph.history_status()["redo_count"]==2,"裁剪后的撤销越过边界");
    const auto retained=graph.history_status()["history_bytes"];
    graph.redo();graph.redo();graph.redo();
    auto expected=ForgeFlowJson::parse(saved);expected["viewport"]=viewport;
    forge_flow_test_require(ForgeFlowJson::parse(graph.export_json())==expected&&graph.history_status()["history_bytes"]==retained&&!graph.document_status()["dirty"].get<bool>(),"重做改变视口、保存点或重复计费");
    const auto unchanged=graph.document_status();
    graph.patch_node("n",{{"title",graph.query("node","n")["title"]}});
    forge_flow_test_require(graph.document_status()==unchanged,"无修改操作消耗历史");
    for(const auto& bad:ForgeFlowJson::array({{{"max_bytes",-1}},{{"max_steps",1.5}},{{"max_bytes",true}},{{"max_steps",10001}},{{"max_bytes",1073741825ull}},{{"max_steps",1},{"unknown",1}}})) {
        forge_flow_test_command(graph,{{"op","history-configure"},{"options",bad}},false);
        forge_flow_test_require(graph.document_status()==unchanged,"非法预算部分生效");
    }
    const auto before_failure=graph.export_json();
    forge_flow_test_command(graph,{{"op","batch"},{"commands",ForgeFlowJson::array({{{"op","title"},{"id","n"},{"text","不能保留"}},{{"op","insert"},{"node",{{"id","n"}}}}})}},false);
    forge_flow_test_command(graph,{{"op","load"},{"text","{}"}},false);
    forge_flow_test_require(graph.export_json()==before_failure&&graph.document_status()==unchanged,"失败事务或加载改变历史/文档");

    graph.configure_history(100,2048);
    graph.update_data("n",{{"blob",std::string(8192,'a')}});
    status=graph.history_status();
    forge_flow_test_require(status["undo_count"]==0&&status["redo_count"]==0&&status["history_bytes"]==0&&status["history_barriers"]==1,"超大编辑未建立安全边界");
    const auto barrier=graph.export_json();graph.undo();forge_flow_test_require(graph.export_json()==barrier,"跨过超大编辑撤销");
    graph.patch_node("n",{{"title","边界之后"}});graph.undo();forge_flow_test_require(graph.export_json()==barrier,"边界后的增量撤销失败");
    graph.redo();graph.configure_history(0,2048);graph.patch_node("n",{{"title","关闭历史"}});
    forge_flow_test_require(graph.history_status()["history_bytes"]==0,"关闭历史仍持有差量");
    graph.configure_history(10,0);graph.patch_node("n",{{"title","零字节预算"}});
    forge_flow_test_require(!ForgeFlowJson::parse(graph.snapshot())["can_undo"].get<bool>(),"零字节预算仍允许撤销");
    graph.configure_history(10,4096);graph.import_json(graph.export_json());
    forge_flow_test_require(graph.history_status()["history_budget_bytes"]==4096&&graph.history_status()["history_barriers"]==0,"加载未清空历史或覆盖实例预算");

    for(int i=0;i<5;++i)graph.patch_node("n",{{"title",std::to_string(i)}});
    graph.undo();graph.undo();graph.undo();graph.configure_history(2,4096);
    forge_flow_test_require(graph.history_status()["undo_count"]==0&&graph.history_status()["redo_count"]==2,"合并步数预算未优先保留近期重做");
    graph.redo();graph.redo();graph.redo();
    forge_flow_test_require(graph.query("node","n")["title"]=="3","重做裁剪留下不可达历史");
    graph.undo();graph.patch_node("n",{{"title","新分支"}});
    forge_flow_test_require(graph.history_status()["redo_count"]==0&&graph.history_status()["redo_bytes"]==0,"新编辑未释放重做分支");

    ForgeFlowDocument big,other;
    forge_flow_test_command(big,{{"op","demo"},{"number",1},{"count",500}});
    for(int i=0;i<100;++i)forge_flow_test_command(big,{{"op","field"},{"id","node-1"},{"target","text"},{"text","大图编辑 "+std::to_string(i)}});
    const auto large_status=big.history_status();
    forge_flow_test_require(large_status["undo_count"]==100&&large_status["history_bytes"].get<std::size_t>()<64*1024,"大图小编辑历史没有按变更大小存储");
    big.configure_history(2,4096);
    forge_flow_test_require(other.history_status()["history_budget_bytes"]==32*1024*1024&&other.history_status()["history_limit"]==100,"实例预算串扰");
    std::cout<<"FLOW_INCREMENTAL_HISTORY_OK stable_ids=1000 budget=bytes_and_steps barriers=oversize_and_disabled atomic=batch_load_config redo=shared_storage large_history_bytes="<<large_status["history_bytes"]<<"\n";
}
