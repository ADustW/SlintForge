// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

// 由 tools/generate_flow_bindings.py 生成。
#pragma once
#include "ForgeFlow.hpp"
#include "ForgeFlowImageExport.hpp"
#include <slint.h>
#include <nlohmann/json.hpp>
#include <map>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <algorithm>

template<typename ForgeFlowPointData, typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState>
ForgeFlowPointData forge_flow_decode_ForgeFlowPointData(const nlohmann::json& value) {
    ForgeFlowPointData result{};
    result.x = value.at("x").get<float>();
    result.y = value.at("y").get<float>();
    return result;
}

template<typename ForgeFlowPointData, typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState>
ForgeFlowPortData forge_flow_decode_ForgeFlowPortData(const nlohmann::json& value) {
    ForgeFlowPortData result{};
    result.id = value.at("id").get<std::string>().c_str();
    result.direction = value.at("direction").get<int>();
    result.side = value.at("side").get<int>();
    result.offset = value.at("offset").get<float>();
    result.connectable = value.at("connectable").get<bool>();
    result.max_connections = value.at("max_connections").get<int>();
    return result;
}

template<typename ForgeFlowPointData, typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState>
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
    result.local_x = value.at("local_x").get<float>();
    result.local_y = value.at("local_y").get<float>();
    result.depth = value.at("depth").get<int>();
    result.is_group = value.at("is_group").get<bool>();
    result.constrain_parent = value.at("constrain_parent").get<bool>();
    result.layout_fixed = value.at("layout_fixed").get<bool>();
    std::vector<ForgeFlowPortData> ports;
    for (const auto& item : value.at("ports")) ports.push_back(forge_flow_decode_ForgeFlowPortData<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(item));
    result.ports = std::make_shared<slint::VectorModel<ForgeFlowPortData>>(std::move(ports));
    result.parent = value.at("parent").get<std::string>().c_str();
    result.side = value.at("side").get<int>();
    result.order = value.at("order").get<int>();
    result.collapsed = value.at("collapsed").get<bool>();
    result.selected = value.at("selected").get<bool>();
    result.hidden = value.at("hidden").get<bool>();
    result.draggable = value.at("draggable").get<bool>();
    result.selectable = value.at("selectable").get<bool>();
    result.connectable = value.at("connectable").get<bool>();
    result.deletable = value.at("deletable").get<bool>();
    result.resizable = value.at("resizable").get<bool>();
    result.header_visible = value.at("header_visible").get<bool>();
    result.header_height = value.at("header_height").get<float>();
    result.min_width = value.at("min_width").get<float>();
    result.min_height = value.at("min_height").get<float>();
    result.max_width = value.at("max_width").get<float>();
    result.max_height = value.at("max_height").get<float>();
    result.aspect_ratio = value.at("aspect_ratio").get<float>();
    return result;
}

template<typename ForgeFlowPointData, typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState>
ForgeFlowEdgeData forge_flow_decode_ForgeFlowEdgeData(const nlohmann::json& value) {
    ForgeFlowEdgeData result{};
    result.id = value.at("id").get<std::string>().c_str();
    result.path = value.at("path").get<std::string>().c_str();
    result.arrow_path = value.at("arrow_path").get<std::string>().c_str();
    result.label = value.at("label").get<std::string>().c_str();
    result.style = value.at("style").get<int>();
    result.arrow = value.at("arrow").get<bool>();
    result.stroke_width = value.at("stroke_width").get<float>();
    result.stroke_color = value.at("stroke_color").get<std::string>().c_str();
    result.color_red = value.at("color_red").get<int>();
    result.color_green = value.at("color_green").get<int>();
    result.color_blue = value.at("color_blue").get<int>();
    result.dashed = value.at("dashed").get<bool>();
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
    result.source_node = value.at("source_node").get<std::string>().c_str();
    result.source_port = value.at("source_port").get<std::string>().c_str();
    result.target_node = value.at("target_node").get<std::string>().c_str();
    result.target_port = value.at("target_port").get<std::string>().c_str();
    result.kind = value.at("kind").get<std::string>().c_str();
    result.data_json = value.at("data_json").get<std::string>().c_str();
    result.path_kind = value.at("path_kind").get<std::string>().c_str();
    std::vector<ForgeFlowPointData> control_points;
    for (const auto& item : value.at("control_points")) control_points.push_back(forge_flow_decode_ForgeFlowPointData<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(item));
    result.control_points = std::make_shared<slint::VectorModel<ForgeFlowPointData>>(std::move(control_points));
    result.selectable = value.at("selectable").get<bool>();
    result.deletable = value.at("deletable").get<bool>();
    result.reconnectable = value.at("reconnectable").get<bool>();
    result.start_marker = value.at("start_marker").get<std::string>().c_str();
    result.end_marker = value.at("end_marker").get<std::string>().c_str();
    result.marker_stroke_path = value.at("marker_stroke_path").get<std::string>().c_str();
    result.animated = value.at("animated").get<bool>();
    result.animation_speed = value.at("animation_speed").get<float>();
    result.path_length = value.at("path_length").get<float>();
    std::vector<ForgeFlowPointData> motion_points;
    for (const auto& item : value.at("motion_points")) motion_points.push_back(forge_flow_decode_ForgeFlowPointData<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(item));
    result.motion_points = std::make_shared<slint::VectorModel<ForgeFlowPointData>>(std::move(motion_points));
    result.curvature = value.at("curvature").get<float>();
    result.layer = value.at("layer").get<int>();
    result.parallel_offset = value.at("parallel_offset").get<float>();
    return result;
}

template<typename ForgeFlowPointData, typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState>
ForgeFlowViewport forge_flow_decode_ForgeFlowViewport(const nlohmann::json& value) {
    ForgeFlowViewport result{};
    result.x = value.at("x").get<float>();
    result.y = value.at("y").get<float>();
    result.zoom = value.at("zoom").get<float>();
    return result;
}

template<typename ForgeFlowPointData, typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState>
ForgeFlowRect forge_flow_decode_ForgeFlowRect(const nlohmann::json& value) {
    ForgeFlowRect result{};
    result.x = value.at("x").get<float>();
    result.y = value.at("y").get<float>();
    result.width = value.at("width").get<float>();
    result.height = value.at("height").get<float>();
    return result;
}

template<typename ForgeFlowPointData, typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState>
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

template<typename ForgeFlowPointData, typename ForgeFlowPortData, typename ForgeFlowNodeData, typename ForgeFlowEdgeData, typename ForgeFlowViewport, typename ForgeFlowRect, typename ForgeFlowAction, typename ForgeFlowState, typename Service, typename Window>
void forge_bind_flow_services(const slint::ComponentHandle<Window>& ui) {
    struct Document { std::unordered_map<std::string,std::size_t> nodes_index,edges_index; ForgeFlowDocument core; std::shared_ptr<slint::VectorModel<ForgeFlowNodeData>> nodes = std::make_shared<slint::VectorModel<ForgeFlowNodeData>>(); std::shared_ptr<slint::VectorModel<ForgeFlowEdgeData>> edges = std::make_shared<slint::VectorModel<ForgeFlowEdgeData>>(); explicit Document(const std::string& mode) : core(mode) {} };
    auto documents = std::make_shared<std::map<int, std::shared_ptr<Document>>>();
    auto next = std::make_shared<int>(0);
    auto update = [documents](int token, const std::string& text) {
        ForgeFlowState state{}; state.token = token;
        auto found = documents->find(token); if (found == documents->end()) { state.error = "图文档已释放"; return state; }
        auto d = found->second;
        const auto value = nlohmann::json::parse(text);
        if (!value.at("removed_nodes").empty()) {
          for (std::size_t i = d->nodes->row_count(); i-- > 0;) {
            const std::string id(d->nodes->row_data(i)->id); bool remove = false;
            for (const auto& gone : value.at("removed_nodes")) if (gone.get<std::string>() == id) remove = true;
            if (remove) d->nodes->erase(i);
          }
          d->nodes_index.clear(); for(std::size_t i=0;i<d->nodes->row_count();++i)d->nodes_index[std::string(d->nodes->row_data(i)->id)]=i;
        }
        for (const auto& item : value.at("nodes")) {
            auto row = forge_flow_decode_ForgeFlowNodeData<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(item);
            const std::string row_id(row.id); const auto position=d->nodes_index.find(row_id);
            if (position == d->nodes_index.end()) { d->nodes_index[row_id]=d->nodes->row_count(); d->nodes->push_back(row); } else {
                const auto i=position->second;
                const auto previous = *d->nodes->row_data(i);
                if (auto stable = std::dynamic_pointer_cast<slint::VectorModel<ForgeFlowPortData>>(previous.ports)) {
                    while (stable->row_count() > row.ports->row_count()) stable->erase(stable->row_count()-1);
                    for (std::size_t j=0; j<row.ports->row_count(); ++j) { auto value=*row.ports->row_data(j); if (j==stable->row_count()) stable->push_back(value); else if (*stable->row_data(j)!=value) stable->set_row_data(j,value); }
                    row.ports=stable;
                }
                if(previous!=row)d->nodes->set_row_data(i,row);
            }
        }
        if (!value.at("removed_edges").empty()) {
          for (std::size_t i = d->edges->row_count(); i-- > 0;) {
            const std::string id(d->edges->row_data(i)->id); bool remove = false;
            for (const auto& gone : value.at("removed_edges")) if (gone.get<std::string>() == id) remove = true;
            if (remove) d->edges->erase(i);
          }
          d->edges_index.clear(); for(std::size_t i=0;i<d->edges->row_count();++i)d->edges_index[std::string(d->edges->row_data(i)->id)]=i;
        }
        for (const auto& item : value.at("edges")) {
            auto row = forge_flow_decode_ForgeFlowEdgeData<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(item);
            const std::string row_id(row.id); const auto position=d->edges_index.find(row_id);
            if (position == d->edges_index.end()) { d->edges_index[row_id]=d->edges->row_count(); d->edges->push_back(row); } else {
                const auto i=position->second;
                const auto previous = *d->edges->row_data(i);
                if (auto stable = std::dynamic_pointer_cast<slint::VectorModel<ForgeFlowPointData>>(previous.control_points)) {
                    while (stable->row_count() > row.control_points->row_count()) stable->erase(stable->row_count()-1);
                    for (std::size_t j=0; j<row.control_points->row_count(); ++j) { auto value=*row.control_points->row_data(j); if (j==stable->row_count()) stable->push_back(value); else if (*stable->row_data(j)!=value) stable->set_row_data(j,value); }
                    row.control_points=stable;
                }
                if (auto stable = std::dynamic_pointer_cast<slint::VectorModel<ForgeFlowPointData>>(previous.motion_points)) {
                    while (stable->row_count() > row.motion_points->row_count()) stable->erase(stable->row_count()-1);
                    for (std::size_t j=0; j<row.motion_points->row_count(); ++j) { auto value=*row.motion_points->row_data(j); if (j==stable->row_count()) stable->push_back(value); else if (*stable->row_data(j)!=value) stable->set_row_data(j,value); }
                    row.motion_points=stable;
                }
                if(previous!=row)d->edges->set_row_data(i,row);
            }
        }
        state.revision = value.at("revision").get<int>();
        state.ok = value.at("ok").get<bool>();
        state.error = value.at("error").get<std::string>().c_str();
        state.mode = value.at("mode").get<std::string>().c_str();
        state.dirty = value.at("dirty").get<bool>();
        state.schema_version = value.at("schema_version").get<int>();
        state.loaded_version = value.at("loaded_version").get<int>();
        state.save_serial = value.at("save_serial").get<int>();
        state.nodes = d->nodes;
        state.edges = d->edges;
        state.viewport = value.at("viewport").empty() ? ForgeFlowViewport{} : forge_flow_decode_ForgeFlowViewport<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(value.at("viewport"));
        state.min_zoom = value.at("min_zoom").get<float>();
        state.max_zoom = value.at("max_zoom").get<float>();
        state.wheel_mode = value.at("wheel_mode").get<std::string>().c_str();
        state.wheel_zoom_step = value.at("wheel_zoom_step").get<float>();
        state.wheel_zoom_requires_control = value.at("wheel_zoom_requires_control").get<bool>();
        state.auto_pan = value.at("auto_pan").get<bool>();
        state.auto_pan_margin = value.at("auto_pan_margin").get<float>();
        state.auto_pan_speed = value.at("auto_pan_speed").get<float>();
        state.touch_enabled = value.at("touch_enabled").get<bool>();
        std::vector<ForgeFlowRect> guides;
        for (const auto& item : value.at("guides")) guides.push_back(forge_flow_decode_ForgeFlowRect<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(item));
        state.guides = std::make_shared<slint::VectorModel<ForgeFlowRect>>(std::move(guides));
        state.grid_size = value.at("grid_size").get<float>();
        state.selected_node_count = value.at("selected_node_count").get<int>();
        state.bounds = value.at("bounds").empty() ? ForgeFlowRect{} : forge_flow_decode_ForgeFlowRect<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(value.at("bounds"));
        state.can_undo = value.at("can_undo").get<bool>();
        state.can_redo = value.at("can_redo").get<bool>();
        state.selected_id = value.at("selected_id").get<std::string>().c_str();
        state.selected_title = value.at("selected_title").get<std::string>().c_str();
        state.selected_edge = value.at("selected_edge").empty() ? ForgeFlowEdgeData{} : forge_flow_decode_ForgeFlowEdgeData<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(value.at("selected_edge"));
        state.selected_edge_count = value.at("selected_edge_count").get<int>();
        state.node_count = value.at("node_count").get<int>();
        state.edge_count = value.at("edge_count").get<int>();
        state.selected_is_group = value.at("selected_is_group").get<bool>();
        state.connection_mode = value.at("connection_mode").get<std::string>().c_str();
        state.connection_radius = value.at("connection_radius").get<float>();
        state.allow_self_connections = value.at("allow_self_connections").get<bool>();
        state.click_to_connect = value.at("click_to_connect").get<bool>();
        return state;
    };
    auto& service = ui->template global<Service>();
    service.on_create([documents, next, update](slint::SharedString mode) { int id = ++*next; auto doc = std::make_shared<Document>(std::string(mode)); (*documents)[id] = doc; return update(id, doc->core.snapshot()); });
    service.on_release([documents](int token) { documents->erase(token); });
    service.on_active_documents([documents]() { return static_cast<int>(documents->size()); });
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
    service.on_save_file([documents](int token, slint::SharedString name) { try { auto found = documents->find(token); if (found == documents->end()) return slint::SharedString("图文档已释放"); const nlohmann::json status=found->second->core.document_status(); if(status.value("editing",false))return slint::SharedString("请先结束当前拖动再保存"); std::filesystem::path path = std::filesystem::u8path(std::string(name)); if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path()); std::ofstream file(path, std::ios::binary | std::ios::trunc); file << found->second->core.export_json(); file.close(); if (!file) return slint::SharedString("保存失败，请检查文件路径"); found->second->core.mark_saved(); return slint::SharedString(); } catch (const std::exception& e) { return slint::SharedString(e.what()); } });
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
        return forge_flow_decode_ForgeFlowAction<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(nlohmann::json::parse(it->second->core.execute(value.dump())));
    });
    service.on_preview([documents](int token, const ForgeFlowAction& action) {
        auto it = documents->find(token); if (it == documents->end()) return ForgeFlowEdgeData{};
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
        value["op"] = "preview";
        const auto result = nlohmann::json::parse(it->second->core.execute(value.dump()));
        return result.contains("path") ? forge_flow_decode_ForgeFlowEdgeData<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(result) : ForgeFlowEdgeData{};
    });
    service.on_query([documents](int token, slint::SharedString request) { auto it = documents->find(token); if (it == documents->end()) return slint::SharedString(); auto value = nlohmann::json::parse(std::string(request),nullptr,false); if (!value.is_object()) return slint::SharedString(); value["op"]="query"; return slint::SharedString(it->second->core.execute(value.dump()).c_str()); });
    service.on_bounds([documents](int token, bool selected, slint::SharedString id) { auto it=documents->find(token); if(it==documents->end())return ForgeFlowRect{}; try {return forge_flow_decode_ForgeFlowRect<ForgeFlowPointData, ForgeFlowPortData, ForgeFlowNodeData, ForgeFlowEdgeData, ForgeFlowViewport, ForgeFlowRect, ForgeFlowAction, ForgeFlowState>(it->second->core.bounds(selected,std::string(id)));} catch(...) {return ForgeFlowRect{};} });
    service.on_search([documents](int token, slint::SharedString text) { std::vector<slint::SharedString> ids; auto it=documents->find(token); if(it!=documents->end()){ const nlohmann::json result=it->second->core.search(std::string(text)); for(const auto& id:result)ids.emplace_back(id.get<std::string>().c_str()); } return std::make_shared<slint::VectorModel<slint::SharedString>>(std::move(ids)); });
    const slint::ComponentWeakHandle<Window> export_weak(ui);
    service.on_export_image([documents,export_weak](int token,slint::SharedString name,bool selected,slint::SharedString text) {
        try {
            auto found=documents->find(token);if(found==documents->end())throw std::runtime_error("图文档已释放");
            auto document=found->second;auto options=nlohmann::json::parse(std::string(text));
            if(!options.is_object()||(options.contains("node_content")&&!options["node_content"].is_object()))throw std::runtime_error("导出选项必须为对象");
            const auto before=document->core.export_json();
            const nlohmann::json nodes=document->core.query("export-nodes","",{{"flag",selected}});
            for(const auto& node:nodes) {
                auto locked=export_weak.lock();if(!locked)throw std::runtime_error("窗口已销毁");
                const auto content=(*locked)->template global<Service>().invoke_export_card(node.dump().c_str());
                if(!content.empty())options["node_content"][node["id"].get<std::string>()]=nlohmann::json::parse(std::string(content));
            }
            found=documents->find(token);
            if(found==documents->end()||found->second!=document||document->core.export_json()!=before)throw std::runtime_error("导出适配器修改或释放了文档");
            return slint::SharedString(forge_flow_save_image(document->core.export_svg(selected,options),std::filesystem::u8path(std::string(name))).c_str());
        } catch(const std::exception& error){return slint::SharedString(error.what());}
    });
    service.set_connected(true);
}
