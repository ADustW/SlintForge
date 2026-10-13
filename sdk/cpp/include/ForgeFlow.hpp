#pragma once
#include "ForgeFlowCore.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <nlohmann/json.hpp>

// 每个对象拥有独立文档；必须由所属 UI 线程串行调用。
class ForgeFlowDocument {
    std::unique_ptr<ForgeFlowCore, decltype(&forge_flow_destroy)> core;
public:
    explicit ForgeFlowDocument(const std::string& mode = "graph") : core(forge_flow_create(mode.c_str()), forge_flow_destroy) {
        if (!core) throw std::runtime_error("无法创建图文档");
    }
    std::string execute(const std::string& command) { return forge_flow_command(core.get(), command.c_str()); }
    std::string snapshot() { return forge_flow_snapshot(core.get()); }
    std::string export_json() { return forge_flow_export(core.get()); }
    std::string undo() { return execute(R"({"op":"undo"})"); }
    std::string redo() { return execute(R"({"op":"redo"})"); }
    // 返回包含 ok/error 和增量模型行的结果，业务失败不会更改当前文档。
    nlohmann::json apply(const nlohmann::json& action) { return nlohmann::json::parse(execute(action.dump())); }
    nlohmann::json add_node(const nlohmann::json& node) { return apply({{"op","insert"},{"node",node}}); }
    nlohmann::json update_node(const std::string& id,const nlohmann::json& node) { return apply({{"op","node"},{"id",id},{"node",node}}); }
    nlohmann::json update_data(const std::string& id,const nlohmann::json& data) { return apply({{"op","data"},{"id",id},{"text",data.dump()}}); }
    nlohmann::json select(const std::string& id,bool additive=false) { return apply({{"op","select"},{"id",id},{"additive",additive}}); }
    nlohmann::json remove_selected() { return apply({{"op","delete"}}); }
    nlohmann::json connect(const std::string& source,const std::string& port,const std::string& target,const std::string& target_port,int style=1,bool arrow=true,const std::string& label="") { return apply({{"op","connect"},{"id",source},{"port",port},{"target",target},{"target_port",target_port},{"number",style},{"flag",arrow},{"text",label}}); }
    nlohmann::json remove_edge(const std::string& id) { return apply({{"op","remove-edge"},{"id",id}}); }
    // 可传 style、arrow、label、stroke_width；其余连线数据保持不变。
    nlohmann::json update_edge_appearance(const std::string& id,const nlohmann::json& appearance) { return apply({{"op","edge-appearance"},{"id",id},{"appearance",appearance}}); }
    nlohmann::json set_viewport(double x,double y,double zoom) { return apply({{"op","view"},{"x",x},{"y",y},{"zoom",zoom}}); }
    nlohmann::json layout() { return apply({{"op","layout"}}); }
    nlohmann::json import_json(const std::string& text) { return apply({{"op","load"},{"text",text}}); }
};
