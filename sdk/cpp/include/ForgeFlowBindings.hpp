// 由 tools/generate_flow_bindings.py 生成。
#pragma once
#include "ForgeFlow.hpp"
#include <slint.h>
#include <nlohmann/json.hpp>
#include <map>
#include <fstream>
#include <filesystem>
#include <algorithm>

template<typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState>
ForgeFlowPortData forge_flow_decode_ForgeFlowPortData(const nlohmann::json& value) {
    ForgeFlowPortData result{};
    result.id = value.at("id").get<std::string>().c_str();
    result.direction = value.at("direction").get<int>();
    result.side = value.at("side").get<int>();
    result.offset = value.at("offset").get<float>();
    return result;
}

template<typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState>
ForgeFlowNodeData forge_flow_decode_ForgeFlowNodeData(const nlohmann::json& value) {
    ForgeFlowNodeData result{};
    result.id = value.at("id").get<std::string>().c_str();
    result.kind = value.at("kind").get<std::string>().c_str();
    result.title = value.at("title").get<std::string>().c_str();
    result.data_json = value.at("data_json").get<std::string>().c_str();
    result.x = value.at("x").get<float>();
    result.y = value.at("y").get<float>();
    result.width = value.at("width").get<float>();
    result.height = value.at("height").get<float>();
    std::vector<ForgeFlowPortData> ports;
    for (const auto& item : value.at("ports")) ports.push_back(forge_flow_decode_ForgeFlowPortData<ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(item));
    result.ports = std::make_shared<slint::VectorModel<ForgeFlowPortData>>(std::move(ports));
    result.parent = value.at("parent").get<std::string>().c_str();
    result.side = value.at("side").get<int>();
    result.order = value.at("order").get<int>();
    result.collapsed = value.at("collapsed").get<bool>();
    result.selected = value.at("selected").get<bool>();
    result.hidden = value.at("hidden").get<bool>();
    return result;
}

template<typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState>
ForgeFlowEdgeData forge_flow_decode_ForgeFlowEdgeData(const nlohmann::json& value) {
    ForgeFlowEdgeData result{};
    result.id = value.at("id").get<std::string>().c_str();
    result.path = value.at("path").get<std::string>().c_str();
    result.arrow_path = value.at("arrow_path").get<std::string>().c_str();
    result.label = value.at("label").get<std::string>().c_str();
    result.style = value.at("style").get<int>();
    result.arrow = value.at("arrow").get<bool>();
    result.stroke_width = value.at("stroke_width").get<float>();
    result.label_x = value.at("label_x").get<float>();
    result.label_y = value.at("label_y").get<float>();
    result.source_x = value.at("source_x").get<float>();
    result.source_y = value.at("source_y").get<float>();
    result.target_x = value.at("target_x").get<float>();
    result.target_y = value.at("target_y").get<float>();
    result.min_x = value.at("min_x").get<float>();
    result.min_y = value.at("min_y").get<float>();
    result.max_x = value.at("max_x").get<float>();
    result.max_y = value.at("max_y").get<float>();
    result.selected = value.at("selected").get<bool>();
    result.hidden = value.at("hidden").get<bool>();
    return result;
}

template<typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState>
ForgeFlowViewport forge_flow_decode_ForgeFlowViewport(const nlohmann::json& value) {
    ForgeFlowViewport result{};
    result.x = value.at("x").get<float>();
    result.y = value.at("y").get<float>();
    result.zoom = value.at("zoom").get<float>();
    return result;
}

template<typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState>
ForgeFlowRect forge_flow_decode_ForgeFlowRect(const nlohmann::json& value) {
    ForgeFlowRect result{};
    result.x = value.at("x").get<float>();
    result.y = value.at("y").get<float>();
    result.width = value.at("width").get<float>();
    result.height = value.at("height").get<float>();
    return result;
}

template<typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState>
ForgeFlowAction forge_flow_decode_ForgeFlowAction(const nlohmann::json& value) {
    ForgeFlowAction result{};
    result.op = value.at("op").get<std::string>().c_str();
    result.id = value.at("id").get<std::string>().c_str();
    result.target = value.at("target").get<std::string>().c_str();
    result.port = value.at("port").get<std::string>().c_str();
    result.target_port = value.at("target_port").get<std::string>().c_str();
    result.text = value.at("text").get<std::string>().c_str();
    result.x = value.at("x").get<float>();
    result.y = value.at("y").get<float>();
    result.width = value.at("width").get<float>();
    result.height = value.at("height").get<float>();
    result.zoom = value.at("zoom").get<float>();
    result.number = value.at("number").get<int>();
    result.count = value.at("count").get<int>();
    result.flag = value.at("flag").get<bool>();
    result.additive = value.at("additive").get<bool>();
    return result;
}

template<typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState, typename Service, typename Window>
void forge_bind_flow_services(const slint::ComponentHandle<Window>& ui) {
    struct Document { ForgeFlowDocument core; std::shared_ptr<slint::VectorModel<ForgeFlowNodeData>> nodes = std::make_shared<slint::VectorModel<ForgeFlowNodeData>>(); std::shared_ptr<slint::VectorModel<ForgeFlowEdgeData>> edges = std::make_shared<slint::VectorModel<ForgeFlowEdgeData>>(); explicit Document(const std::string& mode) : core(mode) {} };
    auto documents = std::make_shared<std::map<int, std::shared_ptr<Document>>>();
    auto next = std::make_shared<int>(0);
    auto update = [documents](int token, const std::string& text) {
        ForgeFlowState state{}; state.token = token;
        auto found = documents->find(token); if (found == documents->end()) { state.error = "图文档已释放"; return state; }
        auto d = found->second;
        const auto value = nlohmann::json::parse(text);
        for (std::size_t i = d->nodes->row_count(); i-- > 0;) {
            const std::string id(d->nodes->row_data(i)->id); bool remove = false;
            for (const auto& gone : value.at("removed_nodes")) if (gone.get<std::string>() == id) remove = true;
            if (remove) d->nodes->erase(i);
        }
        for (const auto& item : value.at("nodes")) {
            auto row = forge_flow_decode_ForgeFlowNodeData<ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(item);
            std::size_t i = 0; for (; i < d->nodes->row_count(); ++i) if (d->nodes->row_data(i)->id == row.id) break;
            if (i == d->nodes->row_count()) d->nodes->push_back(row); else d->nodes->set_row_data(i, row);
        }
        for (std::size_t i = d->edges->row_count(); i-- > 0;) {
            const std::string id(d->edges->row_data(i)->id); bool remove = false;
            for (const auto& gone : value.at("removed_edges")) if (gone.get<std::string>() == id) remove = true;
            if (remove) d->edges->erase(i);
        }
        for (const auto& item : value.at("edges")) {
            auto row = forge_flow_decode_ForgeFlowEdgeData<ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(item);
            std::size_t i = 0; for (; i < d->edges->row_count(); ++i) if (d->edges->row_data(i)->id == row.id) break;
            if (i == d->edges->row_count()) d->edges->push_back(row); else d->edges->set_row_data(i, row);
        }
        state.revision = value.at("revision").get<int>();
        state.ok = value.at("ok").get<bool>();
        state.error = value.at("error").get<std::string>().c_str();
        state.mode = value.at("mode").get<std::string>().c_str();
        state.nodes = d->nodes;
        state.edges = d->edges;
        state.viewport = value.at("viewport").empty() ? ForgeFlowViewport{} : forge_flow_decode_ForgeFlowViewport<ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(value.at("viewport"));
        state.bounds = value.at("bounds").empty() ? ForgeFlowRect{} : forge_flow_decode_ForgeFlowRect<ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(value.at("bounds"));
        state.can_undo = value.at("can_undo").get<bool>();
        state.can_redo = value.at("can_redo").get<bool>();
        state.selected_id = value.at("selected_id").get<std::string>().c_str();
        state.selected_title = value.at("selected_title").get<std::string>().c_str();
        state.selected_edge = value.at("selected_edge").empty() ? ForgeFlowEdgeData{} : forge_flow_decode_ForgeFlowEdgeData<ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(value.at("selected_edge"));
        state.selected_edge_count = value.at("selected_edge_count").get<int>();
        state.node_count = value.at("node_count").get<int>();
        state.edge_count = value.at("edge_count").get<int>();
        return state;
    };
    auto& service = ui->template global<Service>();
    service.on_create([documents, next, update](slint::SharedString mode) { int id = ++*next; auto doc = std::make_shared<Document>(std::string(mode)); (*documents)[id] = doc; return update(id, doc->core.snapshot()); });
    service.on_release([documents](int token) { documents->erase(token); });
    service.on_execute([documents, update](int token, const ForgeFlowAction& action) {
        auto found = documents->find(token); if (found == documents->end()) { ForgeFlowState state{}; state.error = "图文档已释放"; return state; }
        nlohmann::json value = nlohmann::json::object();
        value["op"] = std::string(action.op);
        value["id"] = std::string(action.id);
        value["target"] = std::string(action.target);
        value["port"] = std::string(action.port);
        value["target_port"] = std::string(action.target_port);
        value["text"] = std::string(action.text);
        value["x"] = action.x;
        value["y"] = action.y;
        value["width"] = action.width;
        value["height"] = action.height;
        value["zoom"] = action.zoom;
        value["number"] = action.number;
        value["count"] = action.count;
        value["flag"] = action.flag;
        value["additive"] = action.additive;
        return update(token, found->second->core.execute(value.dump()));
    });
    service.on_export_json([documents](int token) { auto found = documents->find(token); return slint::SharedString(found == documents->end() ? "" : found->second->core.export_json().c_str()); });
    service.on_field([](slint::SharedString data, slint::SharedString key) { auto value = nlohmann::json::parse(std::string(data), nullptr, false); if (!value.is_object() || !value.contains(std::string(key))) return slint::SharedString(); const auto& field = value[std::string(key)]; return slint::SharedString((field.is_string() ? field.get<std::string>() : field.dump()).c_str()); });
    service.on_save_file([documents](int token, slint::SharedString name) { try { auto found = documents->find(token); if (found == documents->end()) return slint::SharedString("图文档已释放"); std::filesystem::path path = std::filesystem::u8path(std::string(name)); if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path()); std::ofstream file(path, std::ios::binary | std::ios::trunc); file << found->second->core.export_json(); file.close(); if (!file) return slint::SharedString("保存失败，请检查文件路径"); return slint::SharedString(); } catch (const std::exception& e) { return slint::SharedString(e.what()); } });
    service.on_load_file([documents, update](int token, slint::SharedString name) { auto found = documents->find(token); if (found == documents->end()) return ForgeFlowState{}; try { std::ifstream file(std::filesystem::u8path(std::string(name)), std::ios::binary); if (!file) throw std::runtime_error("无法打开图文档"); std::string text((std::istreambuf_iterator<char>(file)), {}); return update(token, found->second->core.execute(nlohmann::json{{"op","load"},{"text",text}}.dump())); } catch (const std::exception& e) { auto state = update(token, found->second->core.snapshot()); state.ok = false; state.error = e.what(); return state; } });
    service.on_resolve([documents](int token, const ForgeFlowAction& action) {
        auto it = documents->find(token); if (it == documents->end()) return ForgeFlowAction{};
        nlohmann::json value = nlohmann::json::object();
        value["op"] = std::string(action.op);
        value["id"] = std::string(action.id);
        value["target"] = std::string(action.target);
        value["port"] = std::string(action.port);
        value["target_port"] = std::string(action.target_port);
        value["text"] = std::string(action.text);
        value["x"] = action.x;
        value["y"] = action.y;
        value["width"] = action.width;
        value["height"] = action.height;
        value["zoom"] = action.zoom;
        value["number"] = action.number;
        value["count"] = action.count;
        value["flag"] = action.flag;
        value["additive"] = action.additive;
        value["op"] = "resolve";
        return forge_flow_decode_ForgeFlowAction<ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(nlohmann::json::parse(it->second->core.execute(value.dump())));
    });
    service.set_connected(true);
}
