// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "ForgeFlowCore.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <functional>
#include <nlohmann/json.hpp>

using ForgeFlowCardExportAdapter=std::function<nlohmann::json(const nlohmann::json&)>;

// 每个对象拥有独立文档；必须由所属 UI 线程串行调用。
class ForgeFlowDocument {
    std::unique_ptr<ForgeFlowCore, decltype(&forge_flow_destroy)> core;
    std::function<std::string(const nlohmann::json&)> before_edit;
    std::function<void(const nlohmann::json&,const nlohmann::json&)> after_edit;
public:
    explicit ForgeFlowDocument(const std::string& mode = "graph") : core(forge_flow_create(mode.c_str()), forge_flow_destroy) {
        if (!core) throw std::runtime_error("无法创建图文档");
    }
    std::string execute(const std::string& command) {
        if(!before_edit&&!after_edit)return forge_flow_command(core.get(),command.c_str());
        const auto action=nlohmann::json::parse(command,nullptr,false);
        const auto op=action.is_object()?action.value("op",std::string()):std::string();
        const bool notify=op!="resolve"&&op!="preview"&&op!="query"&&op!="refresh";
        if(notify&&before_edit&&op!="cancel"&&op!="mark-saved") {
            const auto error=before_edit(action);
            if(!error.empty()){if(op=="drag-end"||op=="resize-end"||op=="bend-end")forge_flow_command(core.get(),R"({"op":"cancel"})");auto state=nlohmann::json::parse(snapshot());state["ok"]=false;state["error"]=error;return state.dump();}
        }
        const std::string result=forge_flow_command(core.get(),command.c_str());
        if(notify&&after_edit){const auto state=nlohmann::json::parse(result);if(state.value("ok",false))after_edit(action,state);}
        return result;
    }
    std::string snapshot() { return forge_flow_snapshot(core.get()); }
    std::string export_json() { return forge_flow_export(core.get()); }
    nlohmann::json document_status() {return query("document-status");}
    // 实例级预算不进入文档与撤销历史；0 表示关闭历史，默认 100 步和 32 MiB。
    nlohmann::json configure_history(std::size_t max_steps=100,std::size_t max_bytes=32*1024*1024) {return apply_checked({{"op","history-configure"},{"options",{{"max_steps",max_steps},{"max_bytes",max_bytes}}}});}
    nlohmann::json history_status() {return query("history");}
    // 宿主必须在持久化成功后调用；单纯生成 JSON 或图片不会清除未保存状态。
    nlohmann::json mark_saved() {return apply_checked({{"op","mark-saved"}});}
    std::string export_svg(bool selected=false,nlohmann::json options=nlohmann::json::object(),ForgeFlowCardExportAdapter adapter={}) {
        if(!options.is_object())throw std::runtime_error("导出选项必须为对象");
        if(adapter) {
            const auto before=export_json();
            for(const auto& node:query("export-nodes","",{{"flag",selected}}))options["node_content"][node["id"].get<std::string>()]=adapter(node);
            if(export_json()!=before)throw std::runtime_error("卡片导出适配器修改了文档，请使用只读适配器");
        }
        return query("export-svg","",{{"flag",selected},{"options",options}}).get<std::string>();
    }
    std::string undo() { return execute(R"({"op":"undo"})"); }
    std::string redo() { return execute(R"({"op":"redo"})"); }
    // 返回包含 ok/error 和增量模型行的结果，业务失败不会更改当前文档。
    nlohmann::json apply(const nlohmann::json& action) { return nlohmann::json::parse(execute(action.dump())); }
    nlohmann::json add_node(const nlohmann::json& node) { return apply({{"op","insert"},{"node",node}}); }
    nlohmann::json update_node(const std::string& id,const nlohmann::json& node) { return apply({{"op","node"},{"id",id},{"node",node}}); }
    nlohmann::json update_data(const std::string& id,const nlohmann::json& data) { return apply({{"op","data"},{"id",id},{"text",data.dump()}}); }
    nlohmann::json select(const std::string& id,bool additive=false) { return apply({{"op","select"},{"id",id},{"additive",additive}}); }
    nlohmann::json select_next(bool reverse=false,bool edges=false) {return apply({{"op","select-next"},{"flag",reverse},{"target",edges?"edge":"node"}});}
    nlohmann::json nudge_selected(double dx,double dy) {return apply({{"op","nudge"},{"x",dx},{"y",dy}});}
    nlohmann::json remove_selected() { return apply({{"op","delete"}}); }
    nlohmann::json connect(const std::string& source,const std::string& port,const std::string& target,const std::string& target_port,int style=1,bool arrow=true,const std::string& label="") { return apply({{"op","connect"},{"id",source},{"port",port},{"target",target},{"target_port",target_port},{"number",style},{"flag",arrow},{"text",label}}); }
    nlohmann::json remove_edge(const std::string& id) { return apply({{"op","remove-edge"},{"id",id}}); }
    // 基础样式及 marker_start/marker_end、animated、animation_speed、curvature、layer、parallel_offset；其余业务数据保持不变。
    nlohmann::json update_edge_appearance(const std::string& id,const nlohmann::json& appearance) { return apply({{"op","edge-appearance"},{"id",id},{"appearance",appearance}}); }
    nlohmann::json set_viewport(double x,double y,double zoom) { return apply({{"op","view"},{"x",x},{"y",y},{"zoom",zoom}}); }
    nlohmann::json layout() { return apply({{"op","layout"}}); }
    // algorithm=layered/tree，direction=LR/RL/TB/BT，可指定间距、父级和递归范围。
    nlohmann::json layout_graph(const nlohmann::json& options) {return apply({{"op","layout"},{"options",options}});}
    nlohmann::json align_selected(const std::string& mode) {return apply({{"op","align"},{"target",mode}});}
    // 普通图使用父级局部坐标；修改父级和解除分组保持画布中的位置。
    nlohmann::json group_selection(const std::string& title="新分组") {return apply({{"op","group"},{"text",title}});}
    nlohmann::json ungroup(const std::string& id) {return apply({{"op","ungroup"},{"id",id}});}
    nlohmann::json reparent(const std::string& id,const std::string& parent) {return apply({{"op","reparent"},{"id",id},{"target",parent}});}
    nlohmann::json toggle_collapse(const std::string& id) {return apply({{"op","collapse"},{"id",id}});}
    nlohmann::json node_world(const std::string& id) {return query("node-world",id);}
    // 应用内复制可跨文档粘贴；片段接口可由宿主传递给系统剪贴板。
    nlohmann::json copy_selection() {return apply({{"op","copy"}});}
    nlohmann::json cut_selection() {return apply({{"op","cut"}});}
    nlohmann::json selection_fragment() {return query("selection-fragment");}
    nlohmann::json paste_fragment(const nlohmann::json& fragment,double x,double y,const std::string& parent="") {return apply({{"op","paste"},{"fragment",fragment},{"x",x},{"y",y},{"target",parent},{"flag",true}});}
    nlohmann::json paste_clipboard(double x,double y,const std::string& parent="") {return apply({{"op","paste"},{"x",x},{"y",y},{"target",parent},{"flag",true}});}
    nlohmann::json import_json(const std::string& text) { return apply({{"op","load"},{"text",text}}); }
    // 返回空字符串表示允许；拒绝时返回业务原因。回调中不要递归编辑本文档。
    void set_before_edit(std::function<std::string(const nlohmann::json&)> callback) {before_edit=std::move(callback);}
    void set_after_edit(std::function<void(const nlohmann::json&,const nlohmann::json&)> callback) {after_edit=std::move(callback);}
    nlohmann::json apply_checked(const nlohmann::json& action) {auto state=apply(action);if(!state.value("ok",false))throw std::runtime_error(state.value("error","图操作失败"));return state;}
    nlohmann::json batch(const nlohmann::json& commands) {return apply({{"op","batch"},{"commands",commands}});}
    nlohmann::json configure(const nlohmann::json& options) {return apply({{"op","options"},{"options",options}});}
    nlohmann::json patch_node(const std::string& id,const nlohmann::json& patch) {return apply({{"op","node-patch"},{"id",id},{"patch",patch}});}
    nlohmann::json patch_edge(const std::string& id,const nlohmann::json& patch) {return apply({{"op","edge-patch"},{"id",id},{"patch",patch}});}
    nlohmann::json update_edge_data(const std::string& id,const nlohmann::json& data) {return apply({{"op","edge-data"},{"id",id},{"data",data}});}
    nlohmann::json query(const std::string& kind,const std::string& id="",nlohmann::json options=nlohmann::json::object()) {options.update({{"op","query"},{"target",kind},{"id",id}});return apply_checked(options).at("result");}
    nlohmann::json patch_selected_nodes(const nlohmann::json& patch) {return apply({{"op","node-properties"},{"patch",patch}});}
    nlohmann::json bounds(bool selected=false,const std::string& id="") {return query("bounds",id,{{"flag",selected}});}
    nlohmann::json search(const std::string& text) {return query("search","",{{"text",text}});}
    nlohmann::json reveal(const std::string& id) {return apply({{"op","reveal"},{"id",id}});}
    nlohmann::json upstream(const std::string& id,bool recursive=false) {return query("upstream",id,{{"flag",recursive}});}
    nlohmann::json downstream(const std::string& id,bool recursive=false) {return query("downstream",id,{{"flag",recursive}});}
    nlohmann::json connected_edges(const std::string& id,const std::string& port="") {return query("connected-edges",id,{{"port",port}});}
    nlohmann::json ports(const std::string& id) {return query("ports",id);}
    nlohmann::json intersections(double x,double y,double width,double height,bool partial=true) {return query("intersections","",{{"x",x},{"y",y},{"width",width},{"height",height},{"flag",partial}});}
};
