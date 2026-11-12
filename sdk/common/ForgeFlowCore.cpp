// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeFlowCore.h"
#include "json.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <queue>
#include <limits>

using ForgeFlowJson = nlohmann::json;
#include "ForgeFlowHistory.hpp"
struct ForgeFlowPoint { double x = 0, y = 0; };
struct ForgeFlowBox { double left, top, right, bottom; };
struct ForgeFlowObstacle { std::string id; ForgeFlowBox box; };
struct ForgeFlowMarkerGeometry {
    std::vector<ForgeFlowPoint> points;
    bool filled=false,closed=false;
};
struct ForgeFlowEdgeGeometry {
    ForgeFlowJson source;
    ForgeFlowJson target;
    ForgeFlowJson edge;
    ForgeFlowJson view;
    std::vector<ForgeFlowPoint> points;
    std::vector<ForgeFlowMarkerGeometry> markers;
    double parallel_offset=0;
    std::pair<double,double> terminal_spacing{24.,24.};
    bool valid = false;
};
struct ForgeFlowCore {
    ForgeFlowJson document;
    ForgeFlowJson saved_content;
    bool dirty=false,drag_was_dirty=false;
    int loaded_version=2,save_serial=0;
    ForgeFlowHistory history;
    std::set<std::string> selected_nodes, selected_edges;
    std::unordered_map<std::string, std::size_t> indices;
    std::unordered_map<std::string, ForgeFlowJson> last_nodes, last_edges;
    std::unordered_map<std::string, ForgeFlowJson> scene_nodes;
    std::unordered_map<std::string, ForgeFlowEdgeGeometry> geometry;
    std::unordered_map<std::string,double> parallel_offsets;
    std::unordered_map<std::string,std::pair<double,double>> terminal_spacing;
    bool parallel_dirty=true;
    ForgeFlowEdgeGeometry preview_geometry;
    std::vector<ForgeFlowObstacle> obstacles;
    std::unordered_map<std::string, std::vector<std::size_t>> adjacency;
    bool adjacency_dirty = true;
    ForgeFlowJson summary;
    ForgeFlowJson drag_before;
    std::map<std::string, ForgeFlowPoint> drag_origins;
    std::map<std::string, ForgeFlowPoint> drag_scene_origins;
    ForgeFlowJson guides=ForgeFlowJson::array();
    std::set<std::string> drag_effective;
    ForgeFlowPoint drag_pointer;
    bool dragging = false;
    bool batching = false;
    std::string bend_edge;
    int bend_index = -1;
    std::string resize_id;
    int resize_corner = 3;
    double resize_width = 0, resize_height = 0, resize_min_width = 64, resize_min_height = 48;
    uint64_t serial = 0;
    int revision = 0;
    std::string result;
};

static void forge_flow_require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
static std::string forge_flow_id(const ForgeFlowJson& v, const char* key = "id") { return v.value(key, std::string()); }
static double forge_flow_number(const ForgeFlowJson& value, const char* key, double fallback = 0) {
    const auto n = value.value(key, fallback);
    forge_flow_require(std::isfinite(n) && std::abs(n) <= 1e9, "坐标或数值无效");
    return n;
}
static int forge_flow_integer(const ForgeFlowJson& value, const char* key, int fallback = 0) {
    if (!value.contains(key)) return fallback;
    forge_flow_require(value.at(key).is_number_integer(), "枚举和顺序必须为整数");
    return static_cast<int>(forge_flow_number(value, key));
}
static ForgeFlowJson forge_flow_ports(bool tree) {
    return ForgeFlowJson::array({{{"id", tree ? "left" : "in"}, {"direction", tree ? 2 : 0}, {"side", 0}, {"offset", .5}},
        {{"id", tree ? "right" : "out"}, {"direction", tree ? 2 : 1}, {"side", 1}, {"offset", .5}}});
}
static ForgeFlowJson forge_flow_empty(const std::string& mode) {
    return {{"version", 2}, {"mode", mode}, {"nodes", ForgeFlowJson::array()}, {"edges", ForgeFlowJson::array()},
        {"viewport", {{"x", 40.}, {"y", 40.}, {"zoom", 1.}}}};
}
static void forge_flow_reindex(ForgeFlowCore& f) {
    f.indices.clear();
    f.scene_nodes.clear();
    for (std::size_t i = 0; i < f.document["nodes"].size(); ++i) f.indices.emplace(forge_flow_id(f.document["nodes"][i]), i);
}
static ForgeFlowJson& forge_flow_node(ForgeFlowCore& f, const std::string& id) {
    const auto it = f.indices.find(id);
    forge_flow_require(it != f.indices.end(), "节点不存在");
    return f.document["nodes"][it->second];
}
// 普通图的子节点保存局部坐标；绘制、端口、路由和命中只读取此世界坐标视图。
// 根节点走引用路径，避免为没有分组的文档复制全部节点。
static const ForgeFlowJson& forge_flow_scene_node(ForgeFlowCore& f, const ForgeFlowJson& node) {
    if(f.document["mode"]=="mindmap")return node;
    const auto parent=forge_flow_id(node,"parent");
    const bool folded=node.value("is_group",false)&&node.value("collapsed",false);
    if(parent.empty()&&!folded)return node;
    const auto id=forge_flow_id(node);
    if(const auto found=f.scene_nodes.find(id);found!=f.scene_nodes.end())return found->second;
    auto scene=node;
    if(!parent.empty()) {
        const auto& ancestor=forge_flow_scene_node(f,forge_flow_node(f,parent));
        scene["x"]=node["x"].get<double>()+ancestor["x"].get<double>();
        scene["y"]=node["y"].get<double>()+ancestor["y"].get<double>();
    }
    if(folded)scene["height"]=std::max(48.,node.value("header_height",36.)+12.);
    return f.scene_nodes.emplace(id,std::move(scene)).first->second;
}
static int forge_flow_depth(ForgeFlowCore& f,const ForgeFlowJson& node) {
    if(f.document["mode"]=="mindmap")return 0;
    int depth=0;auto parent=forge_flow_id(node,"parent");
    while(!parent.empty()){++depth;parent=forge_flow_id(forge_flow_node(f,parent),"parent");}
    return depth;
}
static bool forge_flow_has_ancestor(ForgeFlowCore& f,const ForgeFlowJson& node,const std::set<std::string>& ancestors) {
    auto parent=forge_flow_id(node,"parent");
    while(!parent.empty()){if(ancestors.contains(parent))return true;parent=forge_flow_id(forge_flow_node(f,parent),"parent");}
    return false;
}
static std::set<std::string> forge_flow_descendants(ForgeFlowCore& f,const std::set<std::string>& roots) {
    auto result=roots;
    for(const auto& node:f.document["nodes"])if(forge_flow_has_ancestor(f,node,roots))result.insert(forge_flow_id(node));
    return result;
}
static const ForgeFlowJson& forge_flow_port(const ForgeFlowJson& node, const std::string& id) {
    for (const auto& p : node["ports"]) if (forge_flow_id(p) == id) return p;
    throw std::runtime_error("端口不存在");
}
static ForgeFlowPoint forge_flow_anchor(const ForgeFlowJson& n, const ForgeFlowJson& p) {
    const auto x = n["x"].get<double>(), y = n["y"].get<double>();
    const auto w = n["width"].get<double>(), h = n["height"].get<double>();
    const auto t = p["offset"].get<double>();
    switch (p["side"].get<int>()) {
    case 0: return {x, y + h * t}; case 1: return {x + w, y + h * t};
    case 2: return {x + w * t, y}; default: return {x + w * t, y + h};
    }
}
static bool forge_flow_hidden(ForgeFlowCore& f, const ForgeFlowJson& n) {
    if (n.value("hidden", false)) return true;
    auto id = forge_flow_id(n, "parent");
    while (!id.empty()) {
        const auto& p = forge_flow_node(f, id);
        if (p.value("collapsed", false) || p.value("hidden", false)) return true;
        id = forge_flow_id(p, "parent");
    }
    return false;
}
static ForgeFlowJson forge_flow_normalize_node(ForgeFlowJson node, bool tree) {
    forge_flow_require(node.is_object() && !forge_flow_id(node).empty(), "节点 ID 不能为空");
    node["kind"] = node.value("kind", "text"); node["title"] = node.value("title", "节点");
    for(const auto* key:{"is_group","constrain_parent","layout_fixed"}) {
        forge_flow_require(!node.contains(key)||node[key].is_boolean(),"分组与布局开关必须为布尔值");
        node[key]=node.value(key,std::string(key)=="is_group"&&node["kind"]=="group");
    }
    forge_flow_require(!tree||!node["is_group"].get<bool>(),"思维导图不能包含普通分组容器");
    for (const auto* key : {"x", "y", "width", "height"}) node[key] = forge_flow_number(node, key, std::string(key) == "width" ? 220 : std::string(key) == "height" ? 132 : 0);
    forge_flow_require(node["width"].get<double>() >= 64 && node["height"].get<double>() >= 48, "节点尺寸不能小于 64×48");
    node["parent"] = node.value("parent", ""); node["order"] = forge_flow_integer(node, "order");
    node["side"] = forge_flow_integer(node, "side"); node["collapsed"] = node.value("collapsed", false);
    for (const auto* key : {"draggable", "selectable", "connectable", "deletable", "resizable", "header_visible"}) {
        forge_flow_require(!node.contains(key) || node[key].is_boolean(), "节点开关必须为布尔值");
        node[key] = node.value(key, true);
    }
    forge_flow_require(!node.contains("hidden") || node["hidden"].is_boolean(), "隐藏状态必须为布尔值");
    node["hidden"] = node.value("hidden", false);
    for (const auto* key : {"min_width", "min_height", "max_width", "max_height", "aspect_ratio", "header_height"})
        node[key] = forge_flow_number(node, key, std::string(key)=="min_width"?64.:std::string(key)=="min_height"?48.:std::string(key).starts_with("max_")?1e6:std::string(key)=="header_height"?36.:0.);
    forge_flow_require(node["min_width"]>=64 && node["min_height"]>=48 && node["max_width"]>=node["min_width"] && node["max_height"]>=node["min_height"], "节点尺寸范围无效");
    forge_flow_require(node["width"]>=node["min_width"] && node["width"]<=node["max_width"] && node["height"]>=node["min_height"] && node["height"]<=node["max_height"], "节点尺寸超出允许范围");
    forge_flow_require(node["aspect_ratio"]>=0 && node["header_height"]>=0 && node["header_height"]<=node["height"], "节点比例或标题高度无效");
    if(node["aspect_ratio"].get<double>()>0) {
        const double ratio=node["aspect_ratio"];
        forge_flow_require(std::max(node["min_width"].get<double>(),std::max(node["min_height"].get<double>(),node["header_height"].get<double>())*ratio)<=std::min(node["max_width"].get<double>(),node["max_height"].get<double>()*ratio), "尺寸范围与固定比例冲突");
    }
    forge_flow_require(node["side"].get<int>() >= -1 && node["side"].get<int>() <= 1, "主题方向必须为左、自动或右");
    if (!node.contains("data")) node["data"] = ForgeFlowJson::object();
    forge_flow_require(node["data"].is_object(), "节点业务数据必须为 JSON 对象");
    if (!node.contains("ports")) node["ports"] = node["is_group"].get<bool>()?ForgeFlowJson::array():forge_flow_ports(tree);
    forge_flow_require(node["ports"].is_array(), "端口必须为数组");
    std::set<std::string> ids;
    for (auto& p : node["ports"]) {
        const auto id = forge_flow_id(p); forge_flow_require(!id.empty() && ids.insert(id).second, "端口 ID 为空或重复");
        p["direction"] = forge_flow_integer(p, "direction", 2); p["side"] = forge_flow_integer(p, "side", 1); p["offset"] = forge_flow_number(p, "offset", .5);
        forge_flow_require(!p.contains("connectable") || p["connectable"].is_boolean(), "端口开关必须为布尔值");
        p["connectable"] = p.value("connectable", true);
        p["max_connections"] = forge_flow_integer(p, "max_connections", 0);
        forge_flow_require(p["max_connections"].get<int>()>=0, "端口容量不能为负数");
        forge_flow_require(p["direction"].get<int>() >= 0 && p["direction"].get<int>() <= 2 && p["side"].get<int>() >= 0 && p["side"].get<int>() <= 3 && p["offset"].get<double>() >= 0 && p["offset"].get<double>() <= 1, "端口方向或位置无效");
    }
    return node;
}
static ForgeFlowJson forge_flow_options(const ForgeFlowCore& f) {
    return f.document.value("options", ForgeFlowJson::object());
}
static int forge_flow_port_count(const ForgeFlowCore& f, const std::string& node, const std::string& port, const std::string& exclude="") {
    int count=0;
    for(const auto& e:f.document["edges"]) if(forge_flow_id(e)!=exclude) {
        if((forge_flow_id(e,"source_node")==node && forge_flow_id(e,"source_port")==port) || (forge_flow_id(e,"target_node")==node && forge_flow_id(e,"target_port")==port)) ++count;
    }
    return count;
}
#include "ForgeFlowAppearance.hpp"
static void forge_flow_validate_connection(ForgeFlowCore& f, const ForgeFlowJson& e, const std::string& exclude = "", bool duplicates = true, bool editing = false) {
    forge_flow_validate_appearance(e);
    const auto style = forge_flow_integer(e, "style", 1);
    forge_flow_require(style >= 0 && style <= 3, "连线样式无效");
    const auto width = forge_flow_number(e, "stroke_width", 2);
    forge_flow_require(width >= 1 && width <= 8, "连线宽度必须位于 1 至 8");
    forge_flow_require(!e.contains("arrow") || e["arrow"].is_boolean(), "箭头状态必须为布尔值");
    forge_flow_require(!e.contains("label") || e["label"].is_string(), "连线标签必须为文本");
    forge_flow_require(!e.contains("dashed") || e["dashed"].is_boolean(), "虚线状态必须为布尔值");
    const auto color = e.value("stroke_color", std::string());
    forge_flow_require(color.empty() || (color.size() == 7 && color[0] == '#' && color.find_first_not_of("0123456789abcdefABCDEF", 1) == std::string::npos), "连线颜色应为 #RRGGBB");
    const auto source = forge_flow_id(e, "source_node"), target = forge_flow_id(e, "target_node");
    const auto options=forge_flow_options(f);
    forge_flow_require(source != target || options.value("allow_self_connections",false), "不能连接同一节点");
    forge_flow_require(source != target || e["source_port"] != e["target_port"], "不能连接同一个端口");
    const auto& a = forge_flow_port(forge_flow_node(f, source), forge_flow_id(e, "source_port"));
    const auto& b = forge_flow_port(forge_flow_node(f, target), forge_flow_id(e, "target_port"));
    if(options.value("connection_mode",std::string("strict"))=="strict")
        forge_flow_require(a["direction"].get<int>() != 0 && b["direction"].get<int>() != 1, "端口方向不匹配");
    if(editing) {
        forge_flow_require(forge_flow_node(f,source).value("connectable",true) && forge_flow_node(f,target).value("connectable",true) && a.value("connectable",true) && b.value("connectable",true), "节点或端口已禁止连接");
        forge_flow_require(!forge_flow_hidden(f,forge_flow_node(f,source)) && !forge_flow_hidden(f,forge_flow_node(f,target)), "隐藏节点不能连接");
    }
    for(const auto* p : {&a,&b}) {
        const auto& n=p==&a?source:target;
        const int maximum=p->value("max_connections",0);
        if(maximum>0) forge_flow_require(forge_flow_port_count(f,n,forge_flow_id(*p),exclude)<maximum,"端口连接数已达到上限");
    }
    for(const auto* key:{"selectable","deletable","reconnectable","hidden"}) forge_flow_require(!e.contains(key)||e[key].is_boolean(),"连线开关必须为布尔值");
    forge_flow_require(!e.contains("kind") || e["kind"].is_string(),"连线类型必须为文本");
    forge_flow_require(!e.contains("data") || e["data"].is_object(),"连线业务数据必须为对象");
    const auto path_kind=e.value("path_kind",std::string("auto"));
    forge_flow_require(path_kind=="auto"||path_kind=="polyline"||path_kind=="cubic","自定义路径类型无效");
    const auto points=e.value("control_points",ForgeFlowJson::array());
    forge_flow_require(points.is_array()&&points.size()<=512,"控制点必须为数组且不超过 512 个");
    for(const auto& p:points) {forge_flow_require(p.is_object()&&p.contains("x")&&p.contains("y"),"控制点缺少坐标");forge_flow_number(p,"x");forge_flow_number(p,"y");}
    forge_flow_require(path_kind!="cubic" || points.size()%3==2,"三次曲线路径需要每段两个控制点及中间端点");
    if (duplicates&&!options.value("allow_parallel_connections",false)) for (const auto& other : f.document["edges"]) {
        if (forge_flow_id(other) == exclude) continue;
        forge_flow_require(!(other["source_node"] == e["source_node"] && other["source_port"] == e["source_port"] && other["target_node"] == e["target_node"] && other["target_port"] == e["target_port"]), "同一端口连接已存在");
    }
}
static void forge_flow_validate(ForgeFlowCore& f) {
    auto& d = f.document;
    forge_flow_require(d.is_object() && d.contains("version") && d["version"].is_number_integer() && (d["version"] == 1||d["version"]==2), "不支持的文档版本");
    // v1 的缺省字段沿用原语义；未知业务字段保留，只升级文档版本。
    d["version"]=2;
    forge_flow_require(d.value("mode", "") == "graph" || d.value("mode", "") == "mindmap", "文档模式无效");
    forge_flow_require(d.contains("nodes") && d["nodes"].is_array() && d.contains("edges") && d["edges"].is_array(), "节点和连线必须为数组");
    const bool tree = d["mode"] == "mindmap";
    const auto options=forge_flow_options(f);
    forge_flow_require(options.is_object(),"画布选项必须为对象");
    const auto connection_mode=options.value("connection_mode",std::string("strict"));
    forge_flow_require(connection_mode=="strict"||connection_mode=="loose","连接模式无效");
    const auto radius=forge_flow_number(options,"connection_radius",16.);
    forge_flow_require(radius>=1&&radius<=200,"连接半径必须位于 1 至 200 像素");
    for(const auto* key:{"allow_self_connections","allow_parallel_connections","click_to_connect"}) forge_flow_require(!options.contains(key)||options[key].is_boolean(),"连接选项必须为布尔值");
    const double parallel_spacing=forge_flow_number(options,"parallel_spacing",24.);
    forge_flow_require(parallel_spacing>=0&&parallel_spacing<=256,"并行连线间距必须位于 0 至 256");
    const double grid=forge_flow_number(options,"grid_size",20.),tolerance=forge_flow_number(options,"guide_tolerance",6.);
    forge_flow_require(grid>=2&&grid<=256&&tolerance>=1&&tolerance<=32,"网格或辅助线容差超出范围");
    for(const auto* key:{"guides_enabled","guide_snap"})forge_flow_require(!options.contains(key)||options[key].is_boolean(),"对齐选项必须为布尔值");
    const double min_zoom=forge_flow_number(options,"min_zoom",.1),max_zoom=forge_flow_number(options,"max_zoom",4.);
    forge_flow_require(min_zoom>=.01&&max_zoom<=16&&min_zoom<=max_zoom,"缩放范围必须位于 1% 至 1600%，且最小值不大于最大值");
    const auto wheel=options.value("wheel_mode",std::string("zoom"));forge_flow_require(wheel=="zoom"||wheel=="pan"||wheel=="none","滚轮模式无效");
    const double factor=forge_flow_number(options,"wheel_zoom_step",1.12),margin=forge_flow_number(options,"auto_pan_margin",32.),speed=forge_flow_number(options,"auto_pan_speed",8.);
    forge_flow_require(factor>1&&factor<=2&&margin>=8&&margin<=160&&speed>0&&speed<=40,"滚轮或自动平移参数超出范围");
    for(const auto* key:{"wheel_zoom_requires_control","auto_pan","touch_enabled"})forge_flow_require(!options.contains(key)||options[key].is_boolean(),"导航选项必须为布尔值");
    std::set<std::string> node_ids, edge_ids;
    for (auto& n : d["nodes"]) { n = forge_flow_normalize_node(n, tree); forge_flow_require(node_ids.insert(forge_flow_id(n)).second, "节点 ID 重复"); }
    forge_flow_reindex(f);
    int roots = 0;
    for (const auto& n : d["nodes"]) {
        auto parent = forge_flow_id(n, "parent");
        if (parent.empty()) ++roots;
        if (!tree&&!parent.empty()) forge_flow_require(forge_flow_node(f,parent).value("is_group",false), "父级必须是分组容器");
        std::set<std::string> chain{forge_flow_id(n)};
        while (!parent.empty()) { forge_flow_require(chain.insert(parent).second, "节点存在父级循环"); forge_flow_require(chain.size()<=128,"嵌套层级不能超过 128"); parent = forge_flow_id(forge_flow_node(f, parent), "parent"); }
        if(!tree&&n.value("constrain_parent",false)&&!forge_flow_id(n,"parent").empty()) {
            const auto& p=forge_flow_node(f,forge_flow_id(n,"parent"));
            const double top=p.value("header_visible",true)?p.value("header_height",36.)+12.:12.;
            forge_flow_require(n["x"].get<double>()>=12&&n["y"].get<double>()>=top&&n["x"].get<double>()+n["width"].get<double>()<=p["width"].get<double>()-12+.001&&n["y"].get<double>()+n["height"].get<double>()<=p["height"].get<double>()-12+.001,"子节点超出父级内容边界");
        }
    }
    if (tree && !d["nodes"].empty()) forge_flow_require(roots == 1, "思维导图必须有且只有一个根节点");
    std::set<std::tuple<std::string,std::string,std::string,std::string>> connections;
    std::set<std::string> tree_targets;
    for (auto& e : d["edges"]) {
        forge_flow_require(!forge_flow_id(e).empty() && edge_ids.insert(forge_flow_id(e)).second, "连线 ID 为空或重复");
        e["style"] = forge_flow_integer(e, "style", 1); e["label"] = e.value("label", ""); e["arrow"] = e.value("arrow", !tree);
        forge_flow_require(e["style"].get<int>() >= 0 && e["style"].get<int>() <= 3, "连线样式无效");
        forge_flow_validate_connection(f, e, forge_flow_id(e), false);
        const bool unique=connections.emplace(forge_flow_id(e,"source_node"),forge_flow_id(e,"source_port"),forge_flow_id(e,"target_node"),forge_flow_id(e,"target_port")).second;
        forge_flow_require(unique||(!tree&&options.value("allow_parallel_connections",false)),"同一端口连接已存在");
        if(tree) {
            forge_flow_require(forge_flow_id(forge_flow_node(f,forge_flow_id(e,"target_node")),"parent")==forge_flow_id(e,"source_node"),"思维导图连线与父级关系不一致");
            forge_flow_require(tree_targets.insert(forge_flow_id(e,"target_node")).second,"主题只能有一条父级连线");
        }
    }
    if(tree)forge_flow_require(d["edges"].size()==(d["nodes"].empty()?0:d["nodes"].size()-1),"思维导图连线数量与树结构不一致");
    if (!d.contains("viewport")) d["viewport"] = {{"x", 40.}, {"y", 40.}, {"zoom", 1.}};
    for (const auto* key : {"x", "y", "zoom"}) d["viewport"][key] = forge_flow_number(d["viewport"], key, std::string(key) == "zoom" ? 1 : 0);
    forge_flow_require(d["viewport"]["zoom"].get<double>() >= min_zoom && d["viewport"]["zoom"].get<double>() <= max_zoom, "视口缩放超出文档配置范围");
}
static std::string forge_flow_new_id(ForgeFlowCore& f, const std::string& prefix) {
    for (;;) {
        auto id = prefix + std::to_string(++f.serial);
        if (f.indices.contains(id)) continue;
        bool exists = false; for (const auto& e : f.document["edges"]) if (forge_flow_id(e) == id) exists = true;
        if (!exists) return id;
    }
}
static std::string forge_flow_tree_port(const ForgeFlowJson& node, int side, bool source) {
    std::string fallback;
    for (const auto& port : node["ports"]) {
        if (port["direction"].get<int>() == (source ? 0 : 1)) continue;
        if (port["side"].get<int>() == side) return forge_flow_id(port);
        if (fallback.empty()) fallback = forge_flow_id(port);
    }
    forge_flow_require(!fallback.empty(), "主题缺少兼容方向的连接端口");
    return fallback;
}
static void forge_flow_tree_edges(ForgeFlowCore& f) {
    if (f.document["mode"] != "mindmap") return;
    // 父级和布局只改变端点，保留已有连线的稳定 ID、标签、线型及扩展数据。
    std::map<std::string, ForgeFlowJson> previous;
    for (const auto& edge : f.document["edges"]) previous[forge_flow_id(edge, "target_node")] = edge;
    auto edges = ForgeFlowJson::array();
    for (const auto& n : f.document["nodes"]) {
        auto parent = forge_flow_id(n, "parent"); if (parent.empty()) continue;
        const bool left = n.value("side", 1) < 0;
        const auto id = forge_flow_id(n);
        auto edge = previous.contains(id) ? previous.at(id) : ForgeFlowJson{{"id", "tree:" + id}, {"style", 1}, {"label", ""}, {"arrow", false}};
        if (!previous.contains(id)) for (const auto& old : f.document["edges"]) if (old["id"] == edge["id"]) { edge["id"] = forge_flow_new_id(f, "edge-"); break; }
        edge.update({{"source_node", parent}, {"target_node", id},
            {"source_port", forge_flow_tree_port(forge_flow_node(f, parent), left ? 0 : 1, true)},
            {"target_port", forge_flow_tree_port(n, left ? 1 : 0, false)}});
        edges.push_back(std::move(edge));
    }
    f.document["edges"] = std::move(edges);
}
static void forge_flow_layout(ForgeFlowCore& f) {
    forge_flow_require(f.document["mode"] == "mindmap", "仅思维导图提供自动布局");
    if (f.document["nodes"].empty()) return;
    std::map<std::string, std::vector<std::string>> children;
    std::string root;
    for (const auto& n : f.document["nodes"]) { auto p = forge_flow_id(n, "parent"); if (p.empty()) root = forge_flow_id(n); else children[p].push_back(forge_flow_id(n)); }
    for (auto& [id, ids] : children) std::stable_sort(ids.begin(), ids.end(), [&](const auto& a, const auto& b) { return forge_flow_node(f, a)["order"].get<int>() < forge_flow_node(f, b)["order"].get<int>(); });
    std::map<std::string, double> spans;
    std::function<double(const std::string&)> measure = [&](const std::string& id) {
        const auto& n = forge_flow_node(f, id); double sum = 0;
        for (const auto& child : children[id]) sum += measure(child) + 28.;
        return spans[id] = n.value("collapsed", false) ? n["height"].get<double>() : std::max(n["height"].get<double>(), sum > 0 ? sum - 28. : 0.);
    };
    measure(root);
    std::function<void(const std::string&, double, double, int)> place = [&](const std::string& id, double x, double center, int side) {
        auto& n = forge_flow_node(f, id); n["x"] = x; n["y"] = center - n["height"].get<double>() / 2; n["side"] = side;
        double sum = 0; for (const auto& child : children[id]) sum += spans[child] + 28.;
        double y = center - std::max(0., sum - 28.) / 2;
        for (const auto& child : children[id]) {
            auto& c = forge_flow_node(f, child);
            place(child, side > 0 ? x + n["width"].get<double>() + 80. : x - c["width"].get<double>() - 80., y + spans[child] / 2, side);
            y += spans[child] + 28.;
        }
    };
    auto& r = forge_flow_node(f, root);
    const double center = r["y"].get<double>() + r["height"].get<double>() / 2;
    double totals[2]{}; std::vector<std::string> sides[2];
    for (const auto& id : children[root]) { auto& n = forge_flow_node(f, id); auto s = n.value("side", 0); int index = s ? (s < 0 ? 0 : 1) : (totals[1] <= totals[0] ? 1 : 0); sides[index].push_back(id); totals[index] += spans[id] + 28.; }
    for (int side = 0; side < 2; ++side) { double y = center - std::max(0., totals[side] - 28.) / 2; for (const auto& id : sides[side]) {
        const auto& n = forge_flow_node(f, id);
        place(id, side ? r["x"].get<double>() + r["width"].get<double>() + 80. : r["x"].get<double>() - n["width"].get<double>() - 80., y + spans[id] / 2, side ? 1 : -1);
        y += spans[id] + 28.;
    }}
    forge_flow_tree_edges(f);
}
static ForgeFlowPoint forge_flow_vector(int side) { return side == 0 ? ForgeFlowPoint{-1,0} : side == 1 ? ForgeFlowPoint{1,0} : side == 2 ? ForgeFlowPoint{0,-1} : ForgeFlowPoint{0,1}; }
static ForgeFlowBox forge_flow_box(const ForgeFlowJson& n, double padding = 0) {
    const double x = n["x"], y = n["y"], w = n["width"], h = n["height"];
    return {x-padding,y-padding,x+w+padding,y+h+padding};
}
static bool forge_flow_boxes_intersect(ForgeFlowBox a, ForgeFlowBox b) {
    return a.left <= b.right && a.right >= b.left && a.top <= b.bottom && a.bottom >= b.top;
}
// 只在障碍物影响已有路径的区域时失效；业务内容、选择及视口变化不重新寻路。
static void forge_flow_sync_obstacles(ForgeFlowCore& f) {
    std::vector<ForgeFlowObstacle> next;
    std::vector<ForgeFlowBox> changed;
    for (const auto& n : f.document["nodes"]) if (!forge_flow_hidden(f,n)) {
        const auto& scene=forge_flow_scene_node(f,n);
        auto box = forge_flow_box(scene,16);
        // 展开分组只把标题区当作障碍物，组内连线可以通过正文区域。
        if(n.value("is_group",false)&&!n.value("collapsed",false)) {
            if(!n.value("header_visible",true))continue;
            box.bottom=scene["y"].get<double>()+n.value("header_height",36.)+16;
        }
        const auto index = next.size();
        const auto id = forge_flow_id(n);
        if (index < f.obstacles.size()) {
            const auto& old = f.obstacles[index];
            if (old.id != id || old.box.left != box.left || old.box.top != box.top || old.box.right != box.right || old.box.bottom != box.bottom) {
                changed.push_back(old.box); changed.push_back(box);
            }
        }
        next.push_back({id,box});
    }
    const bool reset = next.size() != f.obstacles.size();
    if(!reset && changed.empty()){f.obstacles=std::move(next);return;}
    for (auto& [id,g] : f.geometry) if (g.valid && g.edge.value("path_kind",std::string("auto"))=="auto") {
        const ForgeFlowBox extent{g.view["min_x"],g.view["min_y"],g.view["max_x"],g.view["max_y"]};
        if (reset || std::any_of(changed.begin(),changed.end(),[&](const auto& b){return forge_flow_boxes_intersect(extent,b);})) g.valid = false;
    }
    f.obstacles = std::move(next);
}
static bool forge_flow_segment_crosses(ForgeFlowPoint a, ForgeFlowPoint b, ForgeFlowBox box) {
    // 允许沿边界走线，不允许穿过矩形内部；同时适用于曲线采样和正交段。
    box.left += .001; box.top += .001; box.right -= .001; box.bottom -= .001;
    double low = 0, high = 1;
    const auto clip = [&](double origin, double delta, double minimum, double maximum) {
        if (std::abs(delta) < 1e-9) return origin >= minimum && origin <= maximum;
        double t1 = (minimum-origin)/delta, t2 = (maximum-origin)/delta;
        if (t1 > t2) std::swap(t1,t2);
        low = std::max(low,t1); high = std::min(high,t2); return low <= high;
    };
    return clip(a.x,b.x-a.x,box.left,box.right) && clip(a.y,b.y-a.y,box.top,box.bottom);
}
static std::vector<ForgeFlowPoint> forge_flow_simplify(std::vector<ForgeFlowPoint> points) {
    std::vector<ForgeFlowPoint> result;
    for (const auto p : points) {
        if (!result.empty() && std::hypot(p.x-result.back().x,p.y-result.back().y)<.001) continue;
        while (result.size()>1) {
            const auto a=result[result.size()-2],b=result.back();
            const double ux=b.x-a.x,uy=b.y-a.y,vx=p.x-b.x,vy=p.y-b.y;
            if (std::abs(ux*vy-uy*vx)>.001 || ux*vx+uy*vy<0) break;
            result.pop_back();
        }
        result.push_back(p);
    }
    return result;
}
struct ForgeFlowRouteContext {
    const ForgeFlowCore& flow;
    std::string source,target;
    double clearance=0;
};
static ForgeFlowBox forge_flow_route_box(const ForgeFlowRouteContext& context,const ForgeFlowObstacle& obstacle) {
    // 端点所属卡片允许线段接入边界，其余卡片预留描边半径与可见间距。
    const double inset=16-(obstacle.id==context.source||obstacle.id==context.target?0:context.clearance);
    const auto& b=obstacle.box;
    return {b.left+inset,b.top+inset,b.right-inset,b.bottom-inset};
}
static bool forge_flow_clear_route(const ForgeFlowRouteContext& context, const std::vector<ForgeFlowPoint>& points) {
    if(points.empty())return true;
    ForgeFlowBox extent{points[0].x,points[0].y,points[0].x,points[0].y};
    for(const auto p:points){extent.left=std::min(extent.left,p.x);extent.right=std::max(extent.right,p.x);extent.top=std::min(extent.top,p.y);extent.bottom=std::max(extent.bottom,p.y);}
    for (const auto& obstacle : context.flow.obstacles) {
        const auto box=forge_flow_route_box(context,obstacle);
        if(!forge_flow_boxes_intersect(extent,box))continue;
        for (std::size_t i=1;i<points.size();++i) if (forge_flow_segment_crosses(points[i-1],points[i],box)) return false;
    }
    return true;
}
#include "ForgeFlowRouting.hpp"
static ForgeFlowPoint forge_flow_route_escape(const ForgeFlowRouteContext& context,ForgeFlowPoint p,ForgeFlowPoint direction,double distance) {
        for (const auto& obstacle:context.flow.obstacles) {
            const auto box=forge_flow_route_box(context,obstacle);
            double gap=1e9;
            if (direction.x>0 && p.y>box.top && p.y<box.bottom && box.left>p.x) gap=box.left-p.x;
            if (direction.x<0 && p.y>box.top && p.y<box.bottom && box.right<p.x) gap=p.x-box.right;
            if (direction.y>0 && p.x>box.left && p.x<box.right && box.top>p.y) gap=box.top-p.y;
            if (direction.y<0 && p.x>box.left && p.x<box.right && box.bottom<p.y) gap=p.y-box.bottom;
            distance=std::min(distance,gap/2);
        }
        return {p.x+direction.x*distance,p.y+direction.y*distance};
}
static std::vector<ForgeFlowPoint> forge_flow_orthogonal_route(const ForgeFlowRouteContext& context, ForgeFlowPoint a, ForgeFlowPoint b, ForgeFlowPoint da, ForgeFlowPoint db,std::pair<double,double> spacing) {
    const auto s=forge_flow_route_escape(context,a,da,spacing.first),t=forge_flow_route_escape(context,b,db,spacing.second);
    // 端口被另一张卡片盖住时，任何外部通道都无法接入；无需遍历整张图寻找出口。
    if(!forge_flow_clear_route(context,{a,s})||!forge_flow_clear_route(context,{t,b}))return {};
    std::vector<ForgeFlowPoint> best;
    double best_cost=1e100;
    const auto consider = [&](std::vector<ForgeFlowPoint> points) {
        points=forge_flow_simplify(std::move(points));
        if (points.size()<2) return;
        const auto start=points[1],end=points[points.size()-2];
        if ((start.x-a.x)*da.x+(start.y-a.y)*da.y<=0 || (end.x-b.x)*db.x+(end.y-b.y)*db.y<=0) return;
        double cost=16.*(points.size()-2);
        for (std::size_t i=1;i<points.size();++i) cost+=std::hypot(points[i].x-points[i-1].x,points[i].y-points[i-1].y);
        if (cost>=best_cost || !forge_flow_clear_route(context,points)) return;
        best_cost=cost;best=std::move(points);
    };
    // 共用端口在靠近端口的位置分流，避免多条线沿同一长主干完全重合。
    if(spacing.first>24){consider({a,s,{s.x,t.y},t,b});consider({a,s,{t.x,s.y},t,b});}
    else {consider({a,s,{t.x,s.y},t,b});consider({a,s,{s.x,t.y},t,b});}
    const auto horizontal = [&](double y){consider({a,s,{s.x,y},{t.x,y},t,b});};
    const auto vertical = [&](double x){consider({a,s,{x,s.y},{x,t.y},t,b});};
    horizontal((a.y+b.y)/2);vertical((a.x+b.x)/2);
    if (!best.empty()) return best;
    // 回折、同侧及被其他卡片遮挡的连接，尝试卡片外侧的确定性通道。
    std::set<double> xs,ys;
    for (const auto& obstacle:context.flow.obstacles) {
        xs.insert(obstacle.box.left);xs.insert(obstacle.box.right);
        ys.insert(obstacle.box.top);ys.insert(obstacle.box.bottom);
    }
    for (double y:ys) horizontal(y);
    for (double x:xs) vertical(x);
    if (!best.empty()) return best;
    auto searched=forge_flow_search_route(context,s,t);
    if(!searched.empty()) {searched.insert(searched.begin(),a);searched.push_back(b);consider(std::move(searched));}
    if(!best.empty()) return best;
    return {};
}
static std::vector<ForgeFlowPoint> forge_flow_parallel_route(const ForgeFlowRouteContext& context,ForgeFlowPoint a,ForgeFlowPoint b,ForgeFlowPoint da,ForgeFlowPoint db,double offset,std::pair<double,double> spacing) {
    if(std::abs(offset)>.001) {
        auto s=forge_flow_route_escape(context,a,da,spacing.first),t=forge_flow_route_escape(context,b,db,spacing.second);
        // 相向端口间距很小时，两个引出点不能合并，否则并行通道会折回同一条竖线。
        if(std::hypot(s.x-t.x,s.y-t.y)<.001) {
            s={a.x+(s.x-a.x)*2/3,a.y+(s.y-a.y)*2/3};
            t={b.x+(t.x-b.x)*2/3,b.y+(t.y-b.y)*2/3};
        }
        for(int attempt=1;attempt<=3;++attempt) {
            std::vector<ForgeFlowPoint> points;
            if(std::abs(b.x-a.x)>=std::abs(b.y-a.y)) {
                const double y=(a.y+b.y)/2+offset*attempt;
                points={a,s,{s.x,y},{t.x,y},t,b};
            } else {
                const double x=(a.x+b.x)/2-offset*attempt;
                points={a,s,{x,s.y},{x,t.y},t,b};
            }
            points=forge_flow_simplify(std::move(points));
            if(points.size()>1&&forge_flow_clear_route(context,points))return points;
        }
    }
    // 通道被障碍物封住时优先避障，不能为了间距穿过卡片。
    return forge_flow_orthogonal_route(context,a,b,da,db,spacing);
}
static void forge_flow_sample_cubic(std::vector<ForgeFlowPoint>& points, ForgeFlowPoint a, ForgeFlowPoint c, ForgeFlowPoint d, ForgeFlowPoint b, int steps=32) {
    if (points.empty()) points.push_back(a);
    for (int i=1;i<=steps;++i) {const double t=i/double(steps),u=1-t;points.push_back({u*u*u*a.x+3*u*u*t*c.x+3*u*t*t*d.x+t*t*t*b.x,u*u*u*a.y+3*u*u*t*c.y+3*u*t*t*d.y+t*t*t*b.y});}
}
static ForgeFlowEdgeGeometry& forge_flow_edge_geometry(ForgeFlowCore& f, const ForgeFlowJson& e, const ForgeFlowJson* source_override=nullptr, const ForgeFlowJson* target_override=nullptr) {
    forge_flow_sync_parallel(f);
    const auto& n1 = source_override?*source_override:forge_flow_scene_node(f,forge_flow_node(f, forge_flow_id(e, "source_node"))); const auto& n2 = target_override?*target_override:forge_flow_scene_node(f,forge_flow_node(f, forge_flow_id(e, "target_node")));
    const auto& p1 = forge_flow_port(n1, forge_flow_id(e, "source_port")); const auto& p2 = forge_flow_port(n2, forge_flow_id(e, "target_port"));
    auto a = forge_flow_anchor(n1, p1), b = forge_flow_anchor(n2, p2);
    ForgeFlowJson ak = {a.x,a.y,p1["side"],n1["width"],n1["height"]}, bk = {b.x,b.y,p2["side"],n2["width"],n2["height"]};
    auto& g = source_override||target_override?f.preview_geometry:f.geometry[forge_flow_id(e)];
    const auto lane=f.parallel_offsets.find(forge_flow_id(e));
    const double offset=e.value("path_kind",std::string("auto"))=="auto"?e.value("parallel_offset",lane==f.parallel_offsets.end()?0.:lane->second):0.;
    const auto terminal=f.terminal_spacing.find(forge_flow_id(e));
    const auto spacing=terminal==f.terminal_spacing.end()?std::make_pair(24.,24.):terminal->second;
    if (g.valid && g.source == ak && g.target == bk && g.edge == e&&g.parallel_offset==offset&&g.terminal_spacing==spacing) return g;
    g.source = ak; g.target = bk; g.edge = e; g.points.clear();g.markers.clear();g.parallel_offset=offset;g.terminal_spacing=spacing;g.valid=true;
    ForgeFlowRouteContext context{f,forge_flow_id(n1),forge_flow_id(n2),e.value("stroke_width",2.)/2+2};
    std::ostringstream path; path.imbue(std::locale::classic()); path << std::fixed << std::setprecision(3) << "M " << a.x << ' ' << a.y;
    const int style = e.value("style", 1);
    const auto da=forge_flow_vector(p1["side"].get<int>()),db=forge_flow_vector(p2["side"].get<int>());
    const double direct_length=std::hypot(b.x-a.x,b.y-a.y),curvature=e.value("curvature",.4);
    const double distance=curvature>0?std::max(24.,direct_length*curvature):0.;
    const ForgeFlowPoint c{a.x+da.x*(distance+spacing.first-24),a.y+da.y*(distance+spacing.first-24)},d{b.x+db.x*(distance+spacing.second-24),b.y+db.y*(distance+spacing.second-24)};
    const auto path_kind=e.value("path_kind",std::string("auto"));
    const auto controls=e.value("control_points",ForgeFlowJson::array());
    if(style==1)forge_flow_sample_cubic(g.points,a,c,d,b);
    if(path_kind=="cubic") {
        g.points={a};auto current=a;
        for(std::size_t i=0;i<controls.size();i+=3) {
            const ForgeFlowPoint c1{controls[i]["x"],controls[i]["y"]},c2{controls[i+1]["x"],controls[i+1]["y"]};
            const ForgeFlowPoint end=i+2<controls.size()?ForgeFlowPoint{controls[i+2]["x"],controls[i+2]["y"]}:b;
            path<<" C "<<c1.x<<' '<<c1.y<<' '<<c2.x<<' '<<c2.y<<' '<<end.x<<' '<<end.y;
            forge_flow_sample_cubic(g.points,current,c1,c2,end,64);current=end;
        }
    }else if(path_kind=="polyline") {
        g.points={a};
        for(const auto& p:controls){g.points.push_back({p["x"],p["y"]});path<<" L "<<p["x"].get<double>()<<' '<<p["y"].get<double>();}
        g.points.push_back(b);path<<" L "<<b.x<<' '<<b.y;
    }else if(std::hypot(b.x-a.x,b.y-a.y)<.001){
        g.points={a,b};path<<" L "<<b.x<<' '<<b.y;
    }else if(style==1 && std::abs(offset)<.001 && forge_flow_clear_route(context,g.points)) {
        path << " C " << c.x << ' ' << c.y << ' ' << d.x << ' ' << d.y << ' ' << b.x << ' ' << b.y;
    }else if(style==0&&std::abs(offset)<.001&&forge_flow_clear_route(context,{a,b})) {
        g.points={a,b};path<<" L "<<b.x<<' '<<b.y;
    }else {
        auto route=forge_flow_parallel_route(context,a,b,da,db,offset,spacing);
        // 窄通道先缩减额外留白，再退至中心线；重叠卡片封住端口时保留可恢复的连接。
        for(double clearance:{e.value("stroke_width",2.)/2,0.})if(route.empty()) {
            context.clearance=clearance;route=forge_flow_parallel_route(context,a,b,da,db,offset,spacing);
        }
        if(route.empty()) {
            const auto s=forge_flow_route_escape(context,a,da,spacing.first),t=forge_flow_route_escape(context,b,db,spacing.second);
            route=forge_flow_simplify({a,s,{s.x,(s.y+t.y)/2},{t.x,(s.y+t.y)/2},t,b});
        }
        g.points={a};
        for(std::size_t i=1;i<route.size();++i){
            if((style==1 || style==3) && i+1<route.size()){
                const auto p=route[i-1],q=route[i],r=route[i+1];const double l1=std::hypot(q.x-p.x,q.y-p.y),l2=std::hypot(r.x-q.x,r.y-q.y);
                double radius=std::min({12.,l1/2,l2/2});
                ForgeFlowPoint enter=q,leave=q;std::vector<ForgeFlowPoint> corner;
                for(int attempt=0;attempt<8;++attempt) {
                    enter={q.x+(p.x-q.x)*radius/l1,q.y+(p.y-q.y)*radius/l1};leave={q.x+(r.x-q.x)*radius/l2,q.y+(r.y-q.y)*radius/l2};
                    corner.clear();forge_flow_sample_cubic(corner,enter,q,q,leave,16);
                    if(forge_flow_clear_route(context,corner))break;
                    radius/=2;corner.clear();
                }
                if(corner.empty()){path<<" L "<<q.x<<' '<<q.y;g.points.push_back(q);}
                else {path<<" L "<<enter.x<<' '<<enter.y<<" C "<<q.x<<' '<<q.y<<' '<<q.x<<' '<<q.y<<' '<<leave.x<<' '<<leave.y;g.points.insert(g.points.end(),corner.begin(),corner.end());}
            }else{path<<" L "<<route[i].x<<' '<<route[i].y;g.points.push_back(route[i]);}
        }
    }
    // 几乎重合的端口可能在去重后仅剩一点，仍保留终点并使用端口方向确定箭头。
    if(g.points.size()<2){g.points.push_back(b);path<<" L "<<b.x<<' '<<b.y;}
    auto before=g.points[g.points.size()-2],after=g.points[1];
    const double end_angle=std::hypot(b.x-before.x,b.y-before.y)<.001?std::atan2(-db.y,-db.x):std::atan2(b.y-before.y,b.x-before.x);
    const double start_angle=std::hypot(a.x-after.x,a.y-after.y)<.001?std::atan2(-da.y,-da.x):std::atan2(a.y-after.y,a.x-after.x);
    const auto start_marker=forge_flow_marker(e,true),end_marker=forge_flow_marker(e,false);
    g.markers.push_back(forge_flow_marker_geometry(start_marker,a,start_angle));
    g.markers.push_back(forge_flow_marker_geometry(end_marker,b,end_angle));
    std::string marker_fill,marker_stroke;
    for(const auto& marker:g.markers)(marker.filled?marker_fill:marker_stroke)+=forge_flow_marker_path(marker);
    double length=0;for(std::size_t i=1;i<g.points.size();++i)length+=std::hypot(g.points[i].x-g.points[i-1].x,g.points[i].y-g.points[i-1].y);
    auto middle=a;double remaining=length/2;for(std::size_t i=1;i<g.points.size();++i){const auto u=g.points[i-1],v=g.points[i];const auto segment=std::hypot(v.x-u.x,v.y-u.y);if(segment>=remaining&&segment>0){const double t=remaining/segment;middle={u.x+(v.x-u.x)*t,u.y+(v.y-u.y)*t};break;}remaining-=segment;}
    g.view={{"id",e["id"]},{"path",path.str()},{"arrow_path",marker_fill},{"marker_stroke_path",marker_stroke},{"style",style},{"arrow",end_marker["shape"]!="none"},{"stroke_width",e.value("stroke_width",2.)},{"label",e.value("label","")},{"label_x",middle.x},{"label_y",middle.y},{"source_x",a.x},{"source_y",a.y},{"target_x",b.x},{"target_y",b.y}};
    auto motion_points=ForgeFlowJson::array();
    if(e.value("animated",false))for(int i=0;i<=64;++i){const auto p=forge_flow_path_point(g.points,length*i/64.);motion_points.push_back({{"x",p.x},{"y",p.y}});}
    g.view.update({{"start_marker",start_marker["shape"]},{"end_marker",end_marker["shape"]},{"animated",e.value("animated",false)},{"animation_speed",e.value("animation_speed",60.)},{"path_length",length},{"motion_points",motion_points},{"curvature",curvature},{"layer",e.value("layer",0)},{"parallel_offset",offset}});
    for(const auto* key:{"source_node","source_port","target_node","target_port"})g.view[key]=e.value(key,std::string());
    g.view.update({{"kind",e.value("kind",std::string("default"))},{"data_json",e.value("data",ForgeFlowJson::object()).dump()},{"path_kind",path_kind},{"control_points",controls},{"selectable",e.value("selectable",true)},{"deletable",e.value("deletable",true)},{"reconnectable",e.value("reconnectable",true)}});
    const auto color = e.value("stroke_color", std::string());
    const auto rgb = color.empty() ? 0ul : std::stoul(color.substr(1), nullptr, 16);
    g.view.update({{"stroke_color",color},{"color_red",(rgb>>16)&255},{"color_green",(rgb>>8)&255},{"color_blue",rgb&255},{"dashed",e.value("dashed",false)}});
    // 虚线仅改变绘制路径，标签、箭头和命中仍使用完整的几何路径。
    if(e.value("dashed",false)) {
        std::ostringstream dashed; dashed.imbue(std::locale::classic()); dashed << std::fixed << std::setprecision(3);
        double phase = 0;
        for(std::size_t i=1;i<g.points.size();++i) {
            const auto u=g.points[i-1],v=g.points[i]; const double segment=std::hypot(v.x-u.x,v.y-u.y);
            double offset=0;
            while(offset<segment-1e-6) {
                const bool draw=phase<8.; const double step=std::min(segment-offset,(draw?8.:14.)-phase);
                if(draw) dashed << " M " << u.x+(v.x-u.x)*offset/segment << ' ' << u.y+(v.y-u.y)*offset/segment << " L " << u.x+(v.x-u.x)*(offset+step)/segment << ' ' << u.y+(v.y-u.y)*(offset+step)/segment;
                offset+=step; phase=std::fmod(phase+step,14.);
            }
        }
        g.view["path"]=dashed.str();
    }
    double minx=a.x,maxx=a.x,miny=a.y,maxy=a.y;
    const auto include=[&](ForgeFlowPoint p){minx=std::min(minx,p.x);maxx=std::max(maxx,p.x);miny=std::min(miny,p.y);maxy=std::max(maxy,p.y);};
    for(const auto p:g.points)include(p);
    for(const auto& marker:g.markers)for(const auto p:marker.points)include(p);
    g.view.update({{"min_x",minx-12},{"min_y",miny-12},{"max_x",maxx+12},{"max_y",maxy+12}});
    return g;
}
static double forge_flow_segment_distance(ForgeFlowPoint p, ForgeFlowPoint a, ForgeFlowPoint b) {
    const double dx=b.x-a.x,dy=b.y-a.y, length=dx*dx+dy*dy;
    const double t=length>0?std::clamp(((p.x-a.x)*dx+(p.y-a.y)*dy)/length,0.,1.):0.;
    return std::hypot(p.x-a.x-t*dx,p.y-a.y-t*dy);
}
static std::string forge_flow_hit_edge(ForgeFlowCore& f, double x, double y, double tolerance) {
    double best=1e100;int top_layer=std::numeric_limits<int>::min();std::string id;
    for (const auto& e:f.document["edges"]) {
        // 悬停也调用命中查询；先排除缓存边界外的连线，避免逐段扫描整张图。
        const auto cached=f.geometry.find(forge_flow_id(e));
        if(cached!=f.geometry.end()&&cached->second.valid){const auto& b=cached->second.view;const double margin=tolerance+e.value("stroke_width",2.)/2;if(x<b["min_x"].get<double>()-margin||x>b["max_x"].get<double>()+margin||y<b["min_y"].get<double>()-margin||y>b["max_y"].get<double>()+margin)continue;}
        if (!e.value("selectable",true)||e.value("hidden",false)||forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(e,"source_node"))) || forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(e,"target_node")))) continue;
        const auto& g=forge_flow_edge_geometry(f,e);
        const double hit_width=tolerance+e.value("stroke_width",2.)/2;
        double distance=1e100;
        for(std::size_t i=1;i<g.points.size();++i)distance=std::min(distance,forge_flow_segment_distance({x,y},g.points[i-1],g.points[i]));
        for(const auto& marker:g.markers)if(!marker.points.empty()) {
            bool inside=false;
            for(std::size_t i=0,j=marker.points.size()-1;i<marker.points.size();j=i++) {
                const auto a=marker.points[j],b=marker.points[i];
                if(i>0||marker.closed)distance=std::min(distance,forge_flow_segment_distance({x,y},a,b));
                if(marker.filled&&((a.y>y)!=(b.y>y))&&x<(b.x-a.x)*(y-a.y)/(b.y-a.y)+a.x)inside=!inside;
            }
            if(inside)distance=0;
        }
        const int layer=e.value("layer",0);
        if(distance<=hit_width&&(layer>top_layer||(layer==top_layer&&distance<=best))){best=distance;top_layer=layer;id=forge_flow_id(e);}
    }
    return id;
}
static std::pair<std::string,std::string> forge_flow_hit_port(ForgeFlowCore& f,double x,double y,int direction,const std::string& exclude,double tolerance) {
    double best=tolerance; std::pair<std::string,std::string> hit;
    for (const auto& n:f.document["nodes"]) {
        if(forge_flow_id(n)==exclude||forge_flow_hidden(f,n))continue;
        for(const auto& p:n["ports"]) {if(p["direction"].get<int>()==direction)continue;auto a=forge_flow_anchor(forge_flow_scene_node(f,n),p);double d=std::hypot(x-a.x,y-a.y);if(d<best){best=d;hit={forge_flow_id(n),forge_flow_id(p)};}}
    }
    return hit;
}
static ForgeFlowJson forge_flow_content(const ForgeFlowJson& d) { auto content=d;content.erase("viewport");return content; }
static bool forge_flow_saved(const ForgeFlowCore& f) {
    // 比较保存点时跳过视口，不再为每次编辑临时复制整张图。
    if(f.document.size()!=f.saved_content.size()+1)return false;
    for(const auto& [key,value]:f.saved_content.items())if(!f.document.contains(key)||f.document[key]!=value)return false;
    return true;
}
static ForgeFlowJson forge_flow_view(ForgeFlowCore& f,bool full,bool ok=true,const std::string& error="") {
    forge_flow_sync_obstacles(f);
    ForgeFlowJson out={{"ok",ok},{"error",error},{"full",full},{"revision",f.revision},{"mode",f.document["mode"]},{"viewport",f.document["viewport"]},{"can_undo",f.history.can_undo()},{"can_redo",f.history.can_redo()},{"history",f.history.status()},{"nodes",ForgeFlowJson::array()},{"edges",ForgeFlowJson::array()},{"removed_nodes",ForgeFlowJson::array()},{"removed_edges",ForgeFlowJson::array()},{"selected_id",""},{"selected_title",""}};
    out["selected_is_group"]=false;
    out.update({{"dirty",f.dirty},{"schema_version",2},{"loaded_version",f.loaded_version},{"save_serial",f.save_serial}});
    out["selected_edge"]=ForgeFlowJson::object();out["selected_edge_count"]=0;
    out["guides"]=f.guides;
    const auto options=forge_flow_options(f);
    out.update({{"connection_mode",options.value("connection_mode",std::string("strict"))},{"connection_radius",options.value("connection_radius",16.)},{"allow_self_connections",options.value("allow_self_connections",false)},{"click_to_connect",options.value("click_to_connect",false)}});
    out["grid_size"]=options.value("grid_size",20.);
    out.update({{"min_zoom",options.value("min_zoom",.1)},{"max_zoom",options.value("max_zoom",4.)},{"wheel_mode",options.value("wheel_mode",std::string("zoom"))},{"wheel_zoom_step",options.value("wheel_zoom_step",1.12)},{"wheel_zoom_requires_control",options.value("wheel_zoom_requires_control",false)},{"auto_pan",options.value("auto_pan",true)},{"auto_pan_margin",options.value("auto_pan_margin",32.)},{"auto_pan_speed",options.value("auto_pan_speed",8.)},{"touch_enabled",options.value("touch_enabled",true)}});
    std::set<std::string> node_ids,edge_ids;double minx=0,miny=0,maxx=1,maxy=1;bool first=true;
    for(const auto& n:f.document["nodes"]) {
        const auto id=forge_flow_id(n);node_ids.insert(id);auto row=forge_flow_scene_node(f,n);row["local_x"]=n["x"];row["local_y"]=n["y"];row["depth"]=forge_flow_depth(f,n);row["data_json"]=n["data"].dump();row.erase("data");row["hidden"]=forge_flow_hidden(f,n);if(row["hidden"].get<bool>()||!n.value("selectable",true))f.selected_nodes.erase(id);row["selected"]=f.selected_nodes.contains(id);
        if(row["selected"]==true && out["selected_id"]==""){out["selected_id"]=id;out["selected_title"]=n["title"];out["selected_is_group"]=n.value("is_group",false);}
        if(!row["hidden"].get<bool>()){double x=row["x"],y=row["y"],w=row["width"],h=row["height"];if(first){minx=x;miny=y;maxx=x+w;maxy=y+h;first=false;}else{minx=std::min(minx,x);miny=std::min(miny,y);maxx=std::max(maxx,x+w);maxy=std::max(maxy,y+h);}}
        if(full||!f.last_nodes.contains(id)||f.last_nodes[id]!=row)out["nodes"].push_back(row);f.last_nodes[id]=std::move(row);
    }
    for(const auto& e:f.document["edges"]) {
        auto row=forge_flow_edge_geometry(f,e).view;const auto id=forge_flow_id(e);edge_ids.insert(id);row["selected"]=f.selected_edges.contains(id);row["hidden"]=e.value("hidden",false)||forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(e,"source_node")))||forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(e,"target_node")));
        if(row["hidden"].get<bool>()||!e.value("selectable",true)){f.selected_edges.erase(id);row["selected"]=false;}
        if(row["selected"]==true && row["hidden"]==false){if(out["selected_edge"].empty())out["selected_edge"]=row;out["selected_edge_count"]=out["selected_edge_count"].get<int>()+1;}
        if(row["hidden"]==false){minx=std::min(minx,row["min_x"].get<double>());miny=std::min(miny,row["min_y"].get<double>());maxx=std::max(maxx,row["max_x"].get<double>());maxy=std::max(maxy,row["max_y"].get<double>());}
        if(full||!f.last_edges.contains(id)||f.last_edges[id]!=row)out["edges"].push_back(row);f.last_edges[id]=std::move(row);
    }
    for(auto it=f.last_nodes.begin();it!=f.last_nodes.end();)if(!node_ids.contains(it->first)){out["removed_nodes"].push_back(it->first);f.selected_nodes.erase(it->first);it=f.last_nodes.erase(it);}else ++it;
    for(auto it=f.last_edges.begin();it!=f.last_edges.end();)if(!edge_ids.contains(it->first)){out["removed_edges"].push_back(it->first);f.selected_edges.erase(it->first);f.geometry.erase(it->first);it=f.last_edges.erase(it);}else ++it;
    out["bounds"]={{"x",minx},{"y",miny},{"width",maxx-minx},{"height",maxy-miny}};
    out["node_count"]=node_ids.size();out["edge_count"]=edge_ids.size();
    out["selected_node_count"]=f.selected_nodes.size();
    f.summary=out;for(const auto* key:{"nodes","edges","removed_nodes","removed_edges"})f.summary[key]=ForgeFlowJson::array();
    return out;
}
// 连续拖动只产生涉及节点及相邻连线的行更新；视口变化不重新生成任何模型行。
static ForgeFlowJson forge_flow_motion_view(ForgeFlowCore& f,bool moved) {
    auto out=f.summary;out["ok"]=true;out["error"]="";out["full"]=false;out["revision"]=f.revision;out["viewport"]=f.document["viewport"];
    out["guides"]=f.guides;
    if(moved) {
        f.dirty=f.drag_was_dirty;
        for(const auto& [id,p]:f.drag_origins){const auto& n=forge_flow_node(f,id);if(n["x"]!=p.x||n["y"]!=p.y)f.dirty=true;}
        if(!f.resize_id.empty()){const auto& n=forge_flow_node(f,f.resize_id);if(n["width"]!=f.resize_width||n["height"]!=f.resize_height)f.dirty=true;}
    }
    out["dirty"]=f.dirty;f.summary["dirty"]=f.dirty;
    if(!moved)return out;
    forge_flow_sync_obstacles(f);
    if(f.adjacency_dirty){f.adjacency.clear();for(std::size_t i=0;i<f.document["edges"].size();++i){const auto& e=f.document["edges"][i];f.adjacency[forge_flow_id(e,"source_node")].push_back(i);f.adjacency[forge_flow_id(e,"target_node")].push_back(i);}f.adjacency_dirty=false;}
    std::set<std::size_t> changed;
    for(const auto& id:f.drag_effective){const auto& n=forge_flow_node(f,id);const auto& scene=forge_flow_scene_node(f,n);auto& row=f.last_nodes.at(id);if(row["x"]==scene["x"]&&row["y"]==scene["y"]&&row["width"]==scene["width"]&&row["height"]==scene["height"])continue;for(const auto* key:{"x","y","width","height"})row[key]=scene[key];row["local_x"]=n["x"];row["local_y"]=n["y"];out["nodes"].push_back(row);for(auto edge:f.adjacency[id])changed.insert(edge);}
    for(std::size_t i=0;i<f.document["edges"].size();++i)if(!f.geometry.at(forge_flow_id(f.document["edges"][i])).valid)changed.insert(i);
    for(auto i:changed){const auto& e=f.document["edges"][i];const auto id=forge_flow_id(e);auto& row=f.last_edges.at(id);auto next=forge_flow_edge_geometry(f,e).view;next["hidden"]=row["hidden"];next["selected"]=row["selected"];if(next!=row){row=std::move(next);out["edges"].push_back(row);}}
    bool first=true;double minx=0,miny=0,maxx=1,maxy=1;
    for(const auto& n:f.document["nodes"]){if(f.last_nodes.at(forge_flow_id(n))["hidden"]==true)continue;const auto& scene=forge_flow_scene_node(f,n);double x=scene["x"],y=scene["y"],w=scene["width"],h=scene["height"];if(first){minx=x;miny=y;maxx=x+w;maxy=y+h;first=false;}else{minx=std::min(minx,x);miny=std::min(miny,y);maxx=std::max(maxx,x+w);maxy=std::max(maxy,y+h);}}
    for(const auto& [id,row]:f.last_edges)if(row["hidden"]==false){minx=std::min(minx,row["min_x"].get<double>());miny=std::min(miny,row["min_y"].get<double>());maxx=std::max(maxx,row["max_x"].get<double>());maxy=std::max(maxy,row["max_y"].get<double>());}
    out["bounds"]={{"x",minx},{"y",miny},{"width",maxx-minx},{"height",maxy-miny}};f.summary["bounds"]=out["bounds"];return out;
}
static ForgeFlowJson forge_flow_make_node(ForgeFlowCore& f,const ForgeFlowJson& a,const std::string& parent="") {
    const bool tree=f.document["mode"]=="mindmap";auto id=forge_flow_id(a);if(id.empty())id=forge_flow_new_id(f,"node-");
    forge_flow_require(!f.indices.contains(id),"节点 ID 重复");
    return forge_flow_normalize_node({{"id",id},{"kind",a.value("kind","text")},{"title",a.value("text",tree?"新主题":"新节点")},{"x",a.value("x",80.)},{"y",a.value("y",80.)},{"parent",parent},{"order",f.document["nodes"].size()},{"data",{{"text",""},{"enabled",true}}}},tree);
}
static void forge_flow_demo(ForgeFlowCore& f,int variant,int count=0) {
    f.document=forge_flow_empty(variant==4?"mindmap":"graph");f.selected_nodes.clear();f.selected_edges.clear();forge_flow_reindex(f);
    int size=count>0?count:variant==4?9:6;
    for(int i=0;i<size;++i) {
        const std::string id="node-"+std::to_string(i+1);ForgeFlowJson a={{"id",id},{"x",60.+(i%3)*290.},{"y",60.+(i/3)*210.},{"text",variant==3?"设备 "+std::to_string(i+1):variant==2?"步骤 "+std::to_string(i+1):variant==4?(i==0?"研发计划":"主题 "+std::to_string(i)):"节点 "+std::to_string(i+1)},{"kind",variant==1?(i%3==0?"form":i%3==1?"device":"image"):"text"}};
        if(count>0){a["x"]=(i%25)*280.;a["y"]=(i/25)*190.;}
        auto n=forge_flow_make_node(f,a,variant==4&&i>0?(i<4?"node-1":"node-"+std::to_string((i-4)%3+2)):"");
        n["data"]={{"text",i%2==0?"可编辑内容":"实时状态"},{"enabled",true},{"value",i*17}};
        f.document["nodes"].push_back(n);forge_flow_reindex(f);
    }
    if(variant==4){forge_flow_layout(f);}else{
        int edges=count>0?count*2:size-1;for(int i=0;i<edges;++i){int s=i%size,t=(s+1+i/size)%size;f.document["edges"].push_back({{"id","edge-"+std::to_string(i+1)},{"source_node","node-"+std::to_string(s+1)},{"target_node","node-"+std::to_string(t+1)},{"source_port","out"},{"target_port","in"},{"style",variant==3?0:i%3},{"label",count>0?"":i%2==0?"数据":""},{"arrow",variant!=3}});}
    }
    if(count==0 && (variant==2 || variant==3)) {
        // 流程图展示条件分支，拓扑图展示星形设备关系，避免五个页面重复同一条链。
        const std::vector<std::string> names=variant==2?std::vector<std::string>{"接收任务","参数校验","执行采集","异常处理","归档结果","通知用户"}:std::vector<std::string>{"核心交换机","采集终端 A","采集终端 B","监控工作站","数据服务器","边缘网关"};
        const std::vector<ForgeFlowPoint> places=variant==2?std::vector<ForgeFlowPoint>{{40,150},{320,150},{600,40},{600,300},{880,40},{880,300}}:std::vector<ForgeFlowPoint>{{350,190},{40,20},{660,20},{40,350},{660,350},{350,450}};
        for(int i=0;i<6;++i) {
            auto& n=f.document["nodes"][i];n["title"]=names[i];n["x"]=places[i].x;n["y"]=places[i].y;n["width"]=200;n["height"]=variant==2?106:132;
            n["kind"]=variant==2?"text":"device";n["data"]["text"]=variant==2?std::vector<std::string>{"接入外部任务请求","检查参数与设备状态","读取数据并等待完成","记录异常，允许重新配置","保存数据与运行日志","发送本次处理结果"}[i]:"在线";
            if(variant==3)for(auto& port:n["ports"])port["direction"]=2;
        }
        f.document["edges"]=ForgeFlowJson::array();
        const std::vector<std::pair<int,int>> links=variant==2?std::vector<std::pair<int,int>>{{1,2},{2,3},{2,4},{3,5},{4,6}}:std::vector<std::pair<int,int>>{{2,1},{1,3},{4,1},{1,5},{1,6}};
        for(std::size_t i=0;i<links.size();++i) {
            const auto [source,target]=links[i];
            f.document["edges"].push_back({{"id","edge-"+std::to_string(i+1)},{"source_node","node-"+std::to_string(source)},{"target_node","node-"+std::to_string(target)},{"source_port","out"},{"target_port","in"},{"style",variant==2?3:1},{"arrow",variant==2},{"label",variant==2?(i==1?"通过":i==2?"未通过":""):"以太网"},{"stroke_color",variant==2&&i==2?"#df5262":"#8191ab"}});
        }
    }
    forge_flow_validate(f);f.history.reset();
}
#include "ForgeFlowGroups.hpp"
#include "ForgeFlowClipboard.hpp"
#include "ForgeFlowLayout.hpp"
#include "ForgeFlowAlignment.hpp"
#include "ForgeFlowNavigation.hpp"
#include "ForgeFlowEditor.hpp"
#include "ForgeFlowExport.hpp"
#include "ForgeFlowQueries.hpp"
static bool forge_flow_apply(ForgeFlowCore& f,const ForgeFlowJson& a) {
    f.scene_nodes.clear();
    const auto op=a.value("op","");const auto id=forge_flow_id(a);const double x=forge_flow_number(a,"x"),y=forge_flow_number(a,"y");const bool tree=f.document["mode"]=="mindmap";
    if(op=="batch") {
        forge_flow_require(!f.dragging&&!f.batching,"当前操作中不能开始批量事务");
        const auto commands=a.contains("commands")?a["commands"]:ForgeFlowJson::parse(a.at("text").get<std::string>());
        forge_flow_require(commands.is_array()&&commands.size()<=10000,"批量命令必须为数组且不超过 10000 项");
        // 命令入口统一提交历史并回滚文档；事务不再复制整个控制器和全部渲染缓存。
        try{f.batching=true;for(const auto& command:commands){const auto name=command.value("op",std::string());forge_flow_require(name!="batch"&&name!="history-configure"&&name!="mark-saved"&&name!="load"&&name!="demo"&&name!="undo"&&name!="redo"&&name!="cancel"&&!name.ends_with("-begin")&&!name.ends_with("-move")&&!name.ends_with("-end"),"事务中不允许文档替换、历史配置或连续手势");forge_flow_apply(f,command);}forge_flow_validate(f);f.batching=false;}
        catch(...){f.batching=false;throw;}
        return true;
    }
    if(op=="demo"){forge_flow_demo(f,a.value("number",0),a.value("count",0));forge_flow_clear_gesture(f);f.saved_content=forge_flow_content(f.document);f.dirty=false;f.loaded_version=2;return true;}
    if(op=="load"){auto parsed=ForgeFlowJson::parse(a.at("text").get<std::string>());const auto version=parsed.value("version",0);f.document=std::move(parsed);forge_flow_validate(f);forge_flow_clear_gesture(f);f.selected_nodes.clear();f.selected_edges.clear();f.history.reset();f.saved_content=forge_flow_content(f.document);f.dirty=false;f.loaded_version=version;return true;}
    if(op=="mark-saved"){forge_flow_require(!f.dragging,"请先结束当前拖动再保存");f.saved_content=forge_flow_content(f.document);f.dirty=false;++f.save_serial;return false;}
    if(op=="history-configure") {forge_flow_require(!f.dragging,"请先结束当前拖动再配置历史");f.history.configure(a.contains("options")?a.at("options"):ForgeFlowJson::parse(a.at("text").get<std::string>()));return false;}
    if(op=="undo"||op=="redo") {forge_flow_require(!f.dragging,"拖动期间不能撤销");if(f.history.restore(f.document,op=="redo")){const auto options=forge_flow_options(f);f.document["viewport"]["zoom"]=std::clamp(f.document["viewport"]["zoom"].get<double>(),options.value("min_zoom",.1),options.value("max_zoom",4.));forge_flow_reindex(f);}return true;}
    if(op=="view"){double z=forge_flow_number(a,"zoom",1);f.document["viewport"]={{"x",x},{"y",y},{"zoom",std::clamp(z,forge_flow_options(f).value("min_zoom",.1),forge_flow_options(f).value("max_zoom",4.))}};return false;}
    if(op=="select-next") {
        const bool edges=a.value("target",std::string())=="edge",reverse=a.value("flag",false);
        std::vector<std::string> ids;
        for(const auto& item:f.document[edges?"edges":"nodes"]) {
            if(!item.value("selectable",true))continue;
            if(edges) {if(item.value("hidden",false)||forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(item,"source_node")))||forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(item,"target_node"))))continue;}
            else if(forge_flow_hidden(f,item))continue;
            ids.push_back(forge_flow_id(item));
        }
        if(ids.empty())return false;
        const auto& selection=edges?f.selected_edges:f.selected_nodes;
        const auto current=!id.empty()?id:selection.empty()?std::string():*selection.begin();
        const auto found=std::find(ids.begin(),ids.end(),current);
        const auto index=found==ids.end()?(reverse?ids.size()-1:0):(std::distance(ids.begin(),found)+ids.size()+(reverse?-1:1))%ids.size();
        return forge_flow_apply(f,{{"op","select"},{"id",ids[index]}});
    }
    if(op=="nudge") {
        forge_flow_require(!f.dragging,"请先结束当前操作");
        const auto target=!id.empty()?id:f.selected_nodes.empty()?std::string():*f.selected_nodes.begin();
        if(target.empty()||(x==0&&y==0))return false;
        // 复用组合拖动的边界、局部坐标和手动路径规则，一次按键产生一步历史。
        try {
            forge_flow_apply(f,{{"op","drag-begin"},{"id",target},{"x",0},{"y",0}});
            forge_flow_apply(f,{{"op","drag-move"},{"x",x},{"y",y},{"flag",false}});
            forge_flow_validate(f);
            forge_flow_apply(f,{{"op","drag-end"}});
        } catch(...) {if(f.dragging)forge_flow_apply(f,{{"op","cancel"}});throw;}
        return false;
    }
    if(op=="select"||op=="select-all"||op=="select-rect"||op=="pick") {
        const bool add=a.value("additive",false);if(!add){f.selected_nodes.clear();f.selected_edges.clear();}
        if(op=="select"){
            if(f.indices.contains(id)){const auto& n=forge_flow_node(f,id);forge_flow_require(n.value("selectable",true)&&!forge_flow_hidden(f,n),"该节点不可选择");if(add&&f.selected_nodes.contains(id))f.selected_nodes.erase(id);else f.selected_nodes.insert(id);}
            else {const auto& e=forge_flow_edge(f,id);forge_flow_require(e.value("selectable",true)&&!e.value("hidden",false),"该连线不可选择");if(add&&f.selected_edges.contains(id))f.selected_edges.erase(id);else f.selected_edges.insert(id);}
        }
        if(op=="select-all"||op=="select-rect")for(const auto& n:f.document["nodes"]){if(forge_flow_hidden(f,n)||!n.value("selectable",true))continue;const auto& scene=forge_flow_scene_node(f,n);double nx=scene["x"],ny=scene["y"],w=scene["width"],h=scene["height"];if(op=="select-all"||(nx>=std::min(x,a.value("width",x))&&ny>=std::min(y,a.value("height",y))&&nx+w<=std::max(x,a.value("width",x))&&ny+h<=std::max(y,a.value("height",y))))f.selected_nodes.insert(forge_flow_id(n));}
        if(op=="pick"){auto edge=forge_flow_hit_edge(f,x,y,8./f.document["viewport"]["zoom"].get<double>());if(!edge.empty()){if(add&&f.selected_edges.contains(edge))f.selected_edges.erase(edge);else f.selected_edges.insert(edge);}}
        return false;
    }
    if(op=="drag-begin") {
        forge_flow_require(!f.dragging,"请先结束当前操作");f.resize_id.clear();
        const auto& node=forge_flow_node(f,id);forge_flow_require(node.value("draggable",true)&&!forge_flow_hidden(f,node),"该节点不可拖动");
        if(!f.selected_nodes.contains(id)){if(!a.value("additive",false))f.selected_nodes.clear();if(node.value("selectable",true))f.selected_nodes.insert(id);}f.selected_edges.clear();f.drag_before=f.document;f.drag_pointer={x,y};f.drag_origins.clear();f.dragging=true;
        f.drag_effective.clear();
        if(!tree)forge_flow_begin_group_drag(f,id);
        else for(const auto& n:f.document["nodes"]){auto nid=forge_flow_id(n);bool included=f.selected_nodes.contains(nid)||nid==id;auto parent=forge_flow_id(n,"parent");while(!included&&!parent.empty()){included=f.selected_nodes.contains(parent);parent=forge_flow_id(forge_flow_node(f,parent),"parent");}if(included&&n.value("draggable",true)&&!forge_flow_hidden(f,n)){f.drag_origins[nid]={n["x"],n["y"]};f.drag_effective.insert(nid);}}
        f.drag_scene_origins.clear();for(const auto& [nid,origin]:f.drag_origins){const auto& world=forge_flow_scene_node(f,forge_flow_node(f,nid));f.drag_scene_origins[nid]={world["x"],world["y"]};}return false;
    }
    if(op=="drag-move") {if(f.dragging&&f.resize_id.empty()){double dx=x-f.drag_pointer.x,dy=y-f.drag_pointer.y;forge_flow_drag_guides(f,a,dx,dy);for(const auto& [nid,p]:f.drag_origins){auto& n=forge_flow_node(f,nid);n["x"]=p.x+dx;n["y"]=p.y+dy;}forge_flow_translate_drag_edges(f,dx,dy);}return false;}
    if(op=="resize-begin") {
        forge_flow_require(!f.dragging,"请先结束当前操作");auto& n=forge_flow_node(f,id);
        forge_flow_require(n.value("resizable",true)&&!forge_flow_hidden(f,n),"该节点不可调整尺寸");
        forge_flow_require(!n.value("is_group",false)||!n.value("collapsed",false),"请先展开分组再调整尺寸");
        const int corner=forge_flow_integer(a,"number",3);forge_flow_require(corner>=0&&corner<=3,"缩放控制点无效");
        f.drag_before=f.document;f.drag_pointer={x,y};f.resize_id=id;f.resize_corner=corner;f.resize_width=n["width"];f.resize_height=n["height"];
        f.resize_min_width=std::max(forge_flow_number(a,"width",64),n.value("min_width",64.));f.resize_min_height=std::max({forge_flow_number(a,"height",48),n.value("min_height",48.),n.value("header_height",36.)});
        forge_flow_require(f.resize_min_width<=n.value("max_width",1e6)&&f.resize_min_height<=n.value("max_height",1e6),"缩放最小尺寸超出节点上限");
        // UI 的默认最小尺寸不能与文档的比例和上限形成反向区间。
        if(const double ratio=n.value("aspect_ratio",0.);ratio>0){const double maximum=std::min(n.value("max_width",1e6),n.value("max_height",1e6)*ratio);f.resize_min_width=std::min(f.resize_min_width,maximum);f.resize_min_height=std::min(f.resize_min_height,maximum/ratio);}
        f.drag_origins={{id,{n["x"],n["y"]}}};f.drag_effective=tree?std::set<std::string>{id}:forge_flow_descendants(f,{id});f.selected_nodes={id};f.selected_edges.clear();f.dragging=true;return false;
    }
    if(op=="resize-move") {
        if(f.dragging&&!f.resize_id.empty()) {
            auto& n=forge_flow_node(f,f.resize_id);const auto origin=f.drag_origins.at(f.resize_id);
            const bool right=(f.resize_corner%2)==1,bottom=f.resize_corner>=2;
            double w=f.resize_width+(right?1:-1)*(x-f.drag_pointer.x),h=f.resize_height+(bottom?1:-1)*(y-f.drag_pointer.y);
            if(a.value("flag",false)){const double grid=forge_flow_options(f).value("grid_size",20.);w=std::round(w/grid)*grid;h=std::round(h/grid)*grid;}
            double minimum_w=f.resize_min_width,minimum_h=f.resize_min_height,maximum_w=n.value("max_width",1e6),maximum_h=n.value("max_height",1e6);
            forge_flow_group_resize_limits(f,n,right,bottom,minimum_w,minimum_h,maximum_w,maximum_h);
            const double ratio=n.value("aspect_ratio",0.);
            if(ratio>0){if(std::abs(h-f.resize_height)>std::abs(w-f.resize_width)/ratio)w=h*ratio;const double low=std::max(minimum_w,minimum_h*ratio),high=std::min(maximum_w,maximum_h*ratio);forge_flow_require(low<=high,"固定比例与父级边界冲突");w=std::clamp(w,low,high);h=w/ratio;}
            else {w=std::clamp(w,minimum_w,maximum_w);h=std::clamp(h,minimum_h,maximum_h);}
            n["x"]=right?origin.x:origin.x+f.resize_width-w;n["y"]=bottom?origin.y:origin.y+f.resize_height-h;n["width"]=w;n["height"]=h;
            forge_flow_translate_drag_edges(f,n["x"].get<double>()-origin.x,n["y"].get<double>()-origin.y);
        }return false;
    }
    if(op=="bend-begin") {
        forge_flow_require(!f.dragging,"请先结束当前操作");const auto& e=forge_flow_edge(f,id);const int index=forge_flow_integer(a,"number");
        forge_flow_require(index>=0&&e.contains("control_points")&&index<static_cast<int>(e["control_points"].size()),"控制点不存在");
        f.drag_before=f.document;f.dragging=true;f.resize_id.clear();f.drag_origins.clear();f.bend_edge=id;f.bend_index=index;return false;
    }
    if(op=="bend-move") {if(f.dragging&&!f.bend_edge.empty())for(auto& e:f.document["edges"])if(forge_flow_id(e)==f.bend_edge)e["control_points"][f.bend_index]={{"x",x},{"y",y}};return false;}
    if(op=="drag-end"||op=="resize-end"||op=="bend-end"||op=="cancel") {if(f.dragging){if(op=="cancel"){const auto viewport=f.document["viewport"];f.document=f.drag_before;f.document["viewport"]=viewport;forge_flow_reindex(f);}else if(op=="drag-end"&&a.value("flag",false))forge_flow_drop_groups(f);forge_flow_clear_gesture(f);}return false;}
    forge_flow_require(!f.dragging,"请先结束当前拖动");
    if(op=="copy") {forge_flow_require(!f.batching,"事务内不能修改共享剪贴板");forge_flow_copy_selection(f);return false;}
    if(op=="connect-at"||op=="reconnect-at") {
        auto request=a;if(op=="reconnect-at")request["target"]="reconnect";
        auto proposal=forge_flow_resolve(f,request);forge_flow_require(proposal["op"]!="","请拖到有效端口");
        return forge_flow_apply(f,proposal);
    }
    if(op=="reveal") {
        forge_flow_reveal(f,id);
    }else if(op=="group") {
        forge_flow_group_selection(f,a);
    }else if(op=="ungroup") {
        forge_flow_ungroup(f,id.empty()?(f.selected_nodes.empty()?std::string():*f.selected_nodes.begin()):id);
    }else if(op=="options") {
        const auto patch=a.contains("options")?a["options"]:ForgeFlowJson::parse(a.at("text").get<std::string>());forge_flow_require(patch.is_object(),"画布选项必须为对象");
        auto options=forge_flow_options(f);options.update(patch);f.document["options"]=options;
        const double minz=forge_flow_number(options,"min_zoom",.1),maxz=forge_flow_number(options,"max_zoom",4.);
        forge_flow_require(minz>=.01&&maxz<=16&&minz<=maxz,"缩放范围无效");
        f.document["viewport"]["zoom"]=std::clamp(f.document["viewport"]["zoom"].get<double>(),minz,maxz);forge_flow_validate(f);
    }else if(op=="auto-size") {
        auto& n=forge_flow_node(f,id);forge_flow_require(n.value("resizable",true),"该节点不可调整尺寸");n["width"]=forge_flow_number(a,"width",n["width"]);n["height"]=forge_flow_number(a,"height",n["height"]);forge_flow_validate(f);if(tree)forge_flow_layout(f);
    }else if(op=="insert") {
        auto n=forge_flow_normalize_node(a.contains("node") ? a.at("node") : ForgeFlowJson::parse(a.at("text").get<std::string>()),tree);forge_flow_require(!f.indices.contains(forge_flow_id(n)),"节点 ID 重复");f.document["nodes"].push_back(n);forge_flow_reindex(f);if(tree)forge_flow_tree_edges(f);forge_flow_validate(f);if(tree)forge_flow_layout(f);
    }else if(op=="remove-edge") {
        forge_flow_require(!tree,"思维导图请删除分支或修改父级");auto& edges=f.document["edges"];auto it=std::find_if(edges.begin(),edges.end(),[&](const auto& e){return forge_flow_id(e)==id;});forge_flow_require(it!=edges.end(),"连线不存在");forge_flow_require(it->value("deletable",true),"该连线不可删除");edges.erase(it);
    }else if(op=="add"||op=="add-child"||op=="add-sibling") {
        std::string parent;
        if(tree&&!f.document["nodes"].empty()){forge_flow_require(!id.empty(),"请选择父主题");parent=op=="add-sibling"?forge_flow_id(forge_flow_node(f,id),"parent"):id;forge_flow_require(!parent.empty(),"根主题不能添加同级主题");}
        auto create=a;create["id"]=a.value("new_id","");if(!tree&&!a.value("target","").empty())create["kind"]=a["target"];auto n=forge_flow_make_node(f,create,parent);auto nid=forge_flow_id(n);f.document["nodes"].push_back(n);forge_flow_reindex(f);f.selected_nodes={nid};f.selected_edges.clear();if(tree)forge_flow_layout(f);
    } else if(op=="node"||op=="data"||op=="field"||op=="title"||op=="collapse"||op=="reparent") {
        auto& n=forge_flow_node(f,id);
        if(op=="node"){auto replacement=a.contains("node") ? a.at("node") : ForgeFlowJson::parse(a.at("text").get<std::string>());forge_flow_require(forge_flow_id(replacement)==id,"节点 ID 不可修改");n=forge_flow_normalize_node(replacement,tree);}
        if(op=="data"){auto data=ForgeFlowJson::parse(a.at("text").get<std::string>());forge_flow_require(data.is_object(),"业务数据必须为对象");n["data"]=data;}
        if(op=="field"){const auto key=a.value("target","");forge_flow_require(!key.empty(),"数据字段不能为空");n["data"][key]=a.contains("value")?a["value"]:ForgeFlowJson(a.value("text",""));}
        if(op=="title")n["title"]=a.value("text","");
        if(op=="collapse"){forge_flow_require(tree||n.value("is_group",false),"只有主题或分组支持折叠");n["collapsed"]=!n.value("collapsed",false);}
        if(op=="reparent"){if(tree){forge_flow_require(!forge_flow_id(n,"parent").empty(),"根节点不能修改父级");n["parent"]=a.value("target","");}else forge_flow_reparent_group_node(f,id,a.value("target",std::string()));}
        if(tree&&op=="reparent")forge_flow_tree_edges(f);forge_flow_validate(f);if(tree)forge_flow_layout(f);
    } else if(op=="reconnect") {
        forge_flow_require(!tree,"思维导图通过修改父级重连");
        bool found=false;for(auto& e:f.document["edges"])if(forge_flow_id(e)==a.value("text","")){forge_flow_require(e.value("reconnectable",true)&&!e.value("hidden",false),"该连线不可重连");auto edited=e;edited["source_node"]=id;edited["source_port"]=a.value("port","");edited["target_node"]=a.value("target","");edited["target_port"]=a.value("target_port","");forge_flow_validate_connection(f,edited,forge_flow_id(e),true,true);e=edited;found=true;}forge_flow_require(found,"连线不存在");
    } else if(op=="connect"||op=="connect-at"||op=="reconnect-at") {
        forge_flow_require(!tree,"思维导图通过父子主题建立连线");ForgeFlowJson e;
        if(op=="reconnect-at") {for(const auto& v:f.document["edges"])if(forge_flow_id(v)==id)e=v;forge_flow_require(!e.is_null(),"连线不存在");const bool start=a.value("number",1)==0;auto hit=forge_flow_hit_port(f,x,y,start?0:1,forge_flow_id(e,start?"target_node":"source_node"),16./f.document["viewport"]["zoom"].get<double>());forge_flow_require(!hit.first.empty(),"请拖到有效端口");e[start?"source_node":"target_node"]=hit.first;e[start?"source_port":"target_port"]=hit.second;forge_flow_validate_connection(f,e,id);for(auto& v:f.document["edges"])if(forge_flow_id(v)==id)v=e;
        } else {
            std::string target=a.value("target",""),target_port=a.value("target_port","");
            if(op=="connect-at"){auto hit=forge_flow_hit_port(f,x,y,1,id,16./f.document["viewport"]["zoom"].get<double>());target=hit.first;target_port=hit.second;forge_flow_require(!target.empty(),"请拖到有效输入端口");}
            e={{"id",forge_flow_new_id(f,"edge-")},{"source_node",id},{"source_port",a.value("port","")},{"target_node",target},{"target_port",target_port},{"label",a.value("text","")},{"style",forge_flow_integer(a,"number",1)},{"arrow",a.value("flag",true)}};forge_flow_validate_connection(f,e,"",true,true);f.document["edges"].push_back(e);
        }
    } else if(op=="node-properties") {
        forge_flow_edit_selected_nodes(f,a);
    } else if(op=="node-patch") {
        auto& node=forge_flow_node(f,id);const auto patch=a.contains("patch")?a["patch"]:ForgeFlowJson::parse(a.at("text").get<std::string>());forge_flow_require(patch.is_object()&&!patch.contains("id"),"节点补丁不能修改 ID");node.update(patch);forge_flow_validate(f);
    } else if(op=="edge-patch"||op=="edge-data"||op=="bend-insert"||op=="bend-remove"||op=="route-reset") {
        bool found=false;for(auto& e:f.document["edges"])if(forge_flow_id(e)==id) {
            if(op=="edge-patch"){const auto patch=a.contains("patch")?a["patch"]:ForgeFlowJson::parse(a.at("text").get<std::string>());forge_flow_require(patch.is_object()&&!patch.contains("id"),"连线补丁不能修改 ID");e.update(patch);}
            if(op=="edge-data")e["data"]=a.contains("data")?a["data"]:ForgeFlowJson::parse(a.at("text").get<std::string>());
            if(op=="route-reset"){e["path_kind"]="auto";e["control_points"]=ForgeFlowJson::array();e.erase("parallel_offset");}
            if(op=="bend-insert") {
                forge_flow_require(e.value("path_kind",std::string("auto"))!="cubic","三次曲线请成组设置控制点");
                if(!e.contains("control_points"))e["control_points"]=ForgeFlowJson::array();auto& points=e["control_points"];
                const int index=forge_flow_integer(a,"number",static_cast<int>(points.size()));forge_flow_require(index>=0&&index<=static_cast<int>(points.size()),"控制点索引越界");points.insert(points.begin()+index,ForgeFlowJson{{"x",x},{"y",y}});e["path_kind"]="polyline";
            }
            if(op=="bend-remove") {forge_flow_require(e.value("path_kind",std::string("auto"))=="polyline","仅折线路径支持删除单个折点");const int index=forge_flow_integer(a,"number");auto& points=e["control_points"];forge_flow_require(index>=0&&index<static_cast<int>(points.size()),"控制点索引越界");points.erase(points.begin()+index);}
            forge_flow_validate_connection(f,e,id);found=true;
        }forge_flow_require(found,"连线不存在");
        // 连线自身合法仍可能破坏父子关系；整树校验失败时由命令入口原子回滚。
        if(tree&&op=="edge-patch")forge_flow_validate(f);
    } else if(op=="edge-appearance") {
        ForgeFlowJson patch;
        if(a.contains("appearance"))patch=a.at("appearance");
        else {
            const auto key=a.value("target","");
            if(key=="style")patch={{key,forge_flow_integer(a,"number",1)}};
            else if(key=="arrow")patch={{key,a.value("flag",true)}};
            else if(key=="label")patch={{key,a.value("text","")}};
            else if(key=="stroke_width")patch={{key,forge_flow_number(a,"width",2)}};
            else if(key=="stroke_color")patch={{key,a.value("text","")}};
            else if(key=="dashed"||key=="animated")patch={{key,a.value("flag",false)}};
            else if(key=="marker_start"||key=="marker_end")patch={{key,a.value("text",std::string("none"))}};
            else if(key=="curvature"||key=="animation_speed"||key=="parallel_offset")patch={{key,forge_flow_number(a,"width")}};
            else if(key=="layer")patch={{key,forge_flow_integer(a,"number")}};
        }
        forge_flow_require(patch.is_object()&&!patch.empty(),"请指定连线样式属性");
        for(const auto& [key,value]:patch.items())forge_flow_require(key=="style"||key=="arrow"||key=="label"||key=="stroke_width"||key=="stroke_color"||key=="dashed"||key=="marker_start"||key=="marker_end"||key=="animated"||key=="animation_speed"||key=="curvature"||key=="layer"||key=="parallel_offset","不支持的连线样式属性");
        bool found=false;
        for(auto& e:f.document["edges"])if(id.empty()?f.selected_edges.contains(forge_flow_id(e)):forge_flow_id(e)==id){if(patch.contains("arrow")&&!patch.contains("marker_end"))e.erase("marker_end");e.update(patch);forge_flow_validate_connection(f,e,forge_flow_id(e),false);found=true;}
        forge_flow_require(found,"请先选中连线");
    } else if(op=="edge") {bool found=false;for(auto& e:f.document["edges"])if(forge_flow_id(e)==id){auto edit=a.contains("edge") ? a.at("edge") : ForgeFlowJson::parse(a.at("text").get<std::string>());forge_flow_require(forge_flow_id(edit)==id,"连线 ID 不可修改");e=edit;found=true;}forge_flow_require(found,"连线不存在");forge_flow_validate(f);
    } else if(op=="delete") {
        forge_flow_delete_selection(f);
    } else if(op=="cut") {
        forge_flow_require(!f.batching,"事务内不能修改共享剪贴板");
        const auto text=forge_flow_selection_fragment(f).dump();forge_flow_delete_selection(f);
        const std::lock_guard<std::mutex> lock(forge_flow_clipboard_mutex);forge_flow_clipboard_text=text;
    } else if(op=="paste") {
        forge_flow_paste_fragment(f,a);
    } else if(op=="duplicate") {
        forge_flow_duplicate_selection(f);
    } else if(op=="layout") {forge_flow_graph_layout(f,a);
    } else if(op=="align") {forge_flow_align_selection(f,a);
    } else if(op!="refresh") throw std::runtime_error("未知编辑操作");
    return op=="group"||op=="ungroup"||op=="delete"||op=="cut"||op=="paste"||op=="duplicate"||op=="add"||op=="add-child"||op=="add-sibling"||op=="layout"||op=="reparent";
}

extern "C" ForgeFlowCore* forge_flow_create(const char* mode) {
    try{auto* f=new ForgeFlowCore;f->document=forge_flow_empty(mode&&std::string(mode)=="mindmap"?"mindmap":"graph");f->saved_content=forge_flow_content(f->document);return f;}catch(...){return nullptr;}
}
extern "C" void forge_flow_destroy(ForgeFlowCore* f){delete f;}
extern "C" const char* forge_flow_command(ForgeFlowCore* f,const char* command) {
    if(!f)return "{\"ok\":false,\"error\":\"文档句柄无效\"}";
    ForgeFlowJson before;std::string operation,failure;bool failed=false;const auto old_nodes=f->selected_nodes,old_edges=f->selected_edges;
    const auto old_serial=f->serial;const auto old_guides=f->guides;
    try{const auto a=ForgeFlowJson::parse(command?command:"");
        const auto op=a.value("op","");operation=op;const bool motion=(op=="drag-move"||op=="resize-move"||op=="view")&&!f->summary.is_null();
        if(op=="resolve"||op=="preview"||op=="query") {
            try{f->result=(op=="resolve"?forge_flow_resolve(*f,a):op=="preview"?forge_flow_preview(*f,a):forge_flow_query(*f,a)).dump();}
            catch(const std::exception& e){auto result=a;result.update({{"op",""},{"ok",false},{"error",e.what()},{"result",nullptr}});f->result=result.dump();}
            return f->result.c_str();
        }
        if(!motion)before=f->dragging&&(op=="drag-end"||op=="resize-end"||op=="bend-end")?f->drag_before:f->document;
        if(op=="drag-begin"||op=="resize-begin"||op=="bend-begin")f->drag_was_dirty=f->dirty;
        if(!motion)f->parallel_dirty=true;
        const bool full=forge_flow_apply(*f,a);f->scene_nodes.clear();if(!motion)f->parallel_dirty=true;++f->revision;
        if(!motion) {
            if(!f->dragging&&op!="demo"&&op!="load"&&op!="undo"&&op!="redo"&&op!="mark-saved"&&op!="history-configure"&&op!="cancel"&&op!="auto-size"&&op!="view")f->history.record(before,f->document);
            f->dirty=!forge_flow_saved(*f);
        }
        // 手势之外的 drag-move/resize-move 不改动文档，也不能用上次手势的状态改写 dirty。
        if(motion)f->result=forge_flow_motion_view(*f,f->dragging&&(op=="drag-move"||op=="resize-move")).dump();
        else {f->adjacency_dirty=true;f->result=forge_flow_view(*f,full).dump();}}
    catch(const std::exception& e){failed=true;try{failure=e.what();}catch(...){}}
    catch(...){failed=true;}
    // 回滚与生成错误视图本身也可能抛出（如内存不足），不能让异常穿过 C ABI。
    if(failed) {
        try{if(failure.empty())failure="图编辑失败";if(f->dragging&&(operation=="drag-end"||operation=="resize-end"||operation=="bend-end")){const auto viewport=f->document["viewport"];f->document=f->drag_before;f->document["viewport"]=viewport;forge_flow_clear_gesture(*f);}else if(!before.is_null())f->document=std::move(before);f->serial=old_serial;f->guides=old_guides;f->selected_nodes=old_nodes;f->selected_edges=old_edges;f->parallel_dirty=true;f->dirty=!forge_flow_saved(*f);forge_flow_reindex(*f);f->result=forge_flow_view(*f,false,false,failure).dump();}
        catch(...){return R"({"ok":false,"error":"图编辑失败，且无法生成视图"})";}
    }
    return f->result.c_str();
}
extern "C" const char* forge_flow_snapshot(ForgeFlowCore* f){if(!f)return "{}";try{f->result=forge_flow_view(*f,true).dump();}catch(...){f->result="{}";}return f->result.c_str();}
extern "C" const char* forge_flow_export(ForgeFlowCore* f){if(!f)return "{}";try{f->result=f->document.dump(2);}catch(...){f->result="{}";}return f->result.c_str();}
