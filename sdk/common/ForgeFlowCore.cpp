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

using ForgeFlowJson = nlohmann::json;
struct ForgeFlowPoint { double x = 0, y = 0; };
struct ForgeFlowBox { double left, top, right, bottom; };
struct ForgeFlowObstacle { std::string id; ForgeFlowBox box; };
struct ForgeFlowEdgeGeometry {
    ForgeFlowJson source;
    ForgeFlowJson target;
    ForgeFlowJson edge;
    ForgeFlowJson view;
    std::vector<ForgeFlowPoint> points;
    bool valid = false;
};
struct ForgeFlowCore {
    ForgeFlowJson document;
    std::vector<ForgeFlowJson> undo, redo;
    std::set<std::string> selected_nodes, selected_edges;
    std::unordered_map<std::string, std::size_t> indices;
    std::unordered_map<std::string, ForgeFlowJson> last_nodes, last_edges;
    std::unordered_map<std::string, ForgeFlowEdgeGeometry> geometry;
    std::vector<ForgeFlowObstacle> obstacles;
    std::unordered_map<std::string, std::vector<std::size_t>> adjacency;
    bool adjacency_dirty = true;
    ForgeFlowJson summary;
    ForgeFlowJson drag_before;
    std::map<std::string, ForgeFlowPoint> drag_origins;
    ForgeFlowPoint drag_pointer;
    bool dragging = false;
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
    return {{"version", 1}, {"mode", mode}, {"nodes", ForgeFlowJson::array()}, {"edges", ForgeFlowJson::array()},
        {"viewport", {{"x", 40.}, {"y", 40.}, {"zoom", 1.}}}};
}
static void forge_flow_reindex(ForgeFlowCore& f) {
    f.indices.clear();
    for (std::size_t i = 0; i < f.document["nodes"].size(); ++i) f.indices.emplace(forge_flow_id(f.document["nodes"][i]), i);
}
static ForgeFlowJson& forge_flow_node(ForgeFlowCore& f, const std::string& id) {
    const auto it = f.indices.find(id);
    forge_flow_require(it != f.indices.end(), "节点不存在");
    return f.document["nodes"][it->second];
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
    auto id = forge_flow_id(n, "parent");
    while (!id.empty()) {
        const auto& p = forge_flow_node(f, id);
        if (p.value("collapsed", false)) return true;
        id = forge_flow_id(p, "parent");
    }
    return false;
}
static ForgeFlowJson forge_flow_normalize_node(ForgeFlowJson node, bool tree) {
    forge_flow_require(node.is_object() && !forge_flow_id(node).empty(), "节点 ID 不能为空");
    node["kind"] = node.value("kind", "text"); node["title"] = node.value("title", "节点");
    for (const auto* key : {"x", "y", "width", "height"}) node[key] = forge_flow_number(node, key, std::string(key) == "width" ? 220 : std::string(key) == "height" ? 132 : 0);
    forge_flow_require(node["width"].get<double>() >= 64 && node["height"].get<double>() >= 48, "节点尺寸不能小于 64×48");
    node["parent"] = node.value("parent", ""); node["order"] = forge_flow_integer(node, "order");
    node["side"] = forge_flow_integer(node, "side"); node["collapsed"] = node.value("collapsed", false);
    forge_flow_require(node["side"].get<int>() >= -1 && node["side"].get<int>() <= 1, "主题方向必须为左、自动或右");
    if (!node.contains("data")) node["data"] = ForgeFlowJson::object();
    forge_flow_require(node["data"].is_object(), "节点业务数据必须为 JSON 对象");
    if (!node.contains("ports")) node["ports"] = forge_flow_ports(tree);
    forge_flow_require(node["ports"].is_array(), "端口必须为数组");
    std::set<std::string> ids;
    for (auto& p : node["ports"]) {
        const auto id = forge_flow_id(p); forge_flow_require(!id.empty() && ids.insert(id).second, "端口 ID 为空或重复");
        p["direction"] = forge_flow_integer(p, "direction", 2); p["side"] = forge_flow_integer(p, "side", 1); p["offset"] = forge_flow_number(p, "offset", .5);
        forge_flow_require(p["direction"].get<int>() >= 0 && p["direction"].get<int>() <= 2 && p["side"].get<int>() >= 0 && p["side"].get<int>() <= 3 && p["offset"].get<double>() >= 0 && p["offset"].get<double>() <= 1, "端口方向或位置无效");
    }
    return node;
}
static void forge_flow_validate_connection(ForgeFlowCore& f, const ForgeFlowJson& e, const std::string& exclude = "", bool duplicates = true) {
    const auto style = forge_flow_integer(e, "style", 1);
    forge_flow_require(style >= 0 && style <= 2, "连线样式无效");
    const auto width = forge_flow_number(e, "stroke_width", 2);
    forge_flow_require(width >= 1 && width <= 8, "连线宽度必须位于 1 至 8");
    forge_flow_require(!e.contains("arrow") || e["arrow"].is_boolean(), "箭头状态必须为布尔值");
    forge_flow_require(!e.contains("label") || e["label"].is_string(), "连线标签必须为文本");
    const auto source = forge_flow_id(e, "source_node"), target = forge_flow_id(e, "target_node");
    forge_flow_require(source != target, "不能连接同一节点");
    const auto& a = forge_flow_port(forge_flow_node(f, source), forge_flow_id(e, "source_port"));
    const auto& b = forge_flow_port(forge_flow_node(f, target), forge_flow_id(e, "target_port"));
    forge_flow_require(a["direction"].get<int>() != 0 && b["direction"].get<int>() != 1, "端口方向不匹配");
    if (duplicates) for (const auto& other : f.document["edges"]) {
        if (forge_flow_id(other) == exclude) continue;
        forge_flow_require(!(other["source_node"] == e["source_node"] && other["source_port"] == e["source_port"] && other["target_node"] == e["target_node"] && other["target_port"] == e["target_port"]), "同一端口连接已存在");
    }
}
static void forge_flow_validate(ForgeFlowCore& f) {
    auto& d = f.document;
    forge_flow_require(d.is_object() && d.contains("version") && d["version"].is_number_integer() && d["version"] == 1, "不支持的文档版本");
    forge_flow_require(d.value("mode", "") == "graph" || d.value("mode", "") == "mindmap", "文档模式无效");
    forge_flow_require(d.contains("nodes") && d["nodes"].is_array() && d.contains("edges") && d["edges"].is_array(), "节点和连线必须为数组");
    const bool tree = d["mode"] == "mindmap";
    std::set<std::string> node_ids, edge_ids;
    for (auto& n : d["nodes"]) { n = forge_flow_normalize_node(n, tree); forge_flow_require(node_ids.insert(forge_flow_id(n)).second, "节点 ID 重复"); }
    forge_flow_reindex(f);
    int roots = 0;
    for (const auto& n : d["nodes"]) {
        auto parent = forge_flow_id(n, "parent");
        if (parent.empty()) ++roots;
        if (!tree) forge_flow_require(parent.empty(), "普通图不使用树父级");
        std::set<std::string> chain{forge_flow_id(n)};
        while (!parent.empty()) { forge_flow_require(chain.insert(parent).second, "思维导图存在父级循环"); parent = forge_flow_id(forge_flow_node(f, parent), "parent"); }
    }
    if (tree && !d["nodes"].empty()) forge_flow_require(roots == 1, "思维导图必须有且只有一个根节点");
    std::set<std::tuple<std::string,std::string,std::string,std::string>> connections;
    std::set<std::string> tree_targets;
    for (auto& e : d["edges"]) {
        forge_flow_require(!forge_flow_id(e).empty() && edge_ids.insert(forge_flow_id(e)).second, "连线 ID 为空或重复");
        e["style"] = forge_flow_integer(e, "style", 1); e["label"] = e.value("label", ""); e["arrow"] = e.value("arrow", !tree);
        forge_flow_require(e["style"].get<int>() >= 0 && e["style"].get<int>() <= 2, "连线样式无效");
        forge_flow_validate_connection(f, e, forge_flow_id(e), false);
        forge_flow_require(connections.emplace(forge_flow_id(e,"source_node"),forge_flow_id(e,"source_port"),forge_flow_id(e,"target_node"),forge_flow_id(e,"target_port")).second,"同一端口连接已存在");
        if(tree) {
            forge_flow_require(forge_flow_id(forge_flow_node(f,forge_flow_id(e,"target_node")),"parent")==forge_flow_id(e,"source_node"),"思维导图连线与父级关系不一致");
            forge_flow_require(tree_targets.insert(forge_flow_id(e,"target_node")).second,"主题只能有一条父级连线");
        }
    }
    if(tree)forge_flow_require(d["edges"].size()==(d["nodes"].empty()?0:d["nodes"].size()-1),"思维导图连线数量与树结构不一致");
    if (!d.contains("viewport")) d["viewport"] = {{"x", 40.}, {"y", 40.}, {"zoom", 1.}};
    for (const auto* key : {"x", "y", "zoom"}) d["viewport"][key] = forge_flow_number(d["viewport"], key, std::string(key) == "zoom" ? 1 : 0);
    forge_flow_require(d["viewport"]["zoom"].get<double>() >= .1 && d["viewport"]["zoom"].get<double>() <= 4., "缩放必须位于 10% 至 400%");
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
        const auto box = forge_flow_box(n,16);
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
    for (auto& [id,g] : f.geometry) if (g.valid && g.edge.value("style",1) != 0) {
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
static bool forge_flow_clear_route(const ForgeFlowCore& f, const std::vector<ForgeFlowPoint>& points) {
    for (const auto& obstacle : f.obstacles) {
        // 路由保持 16 像素间距，命中检查使用实际卡片边界。
        auto box=obstacle.box; box.left+=16;box.top+=16;box.right-=16;box.bottom-=16;
        for (std::size_t i=1;i<points.size();++i) if (forge_flow_segment_crosses(points[i-1],points[i],box)) return false;
    }
    return true;
}
static std::vector<ForgeFlowPoint> forge_flow_orthogonal_route(const ForgeFlowCore& f, ForgeFlowPoint a, ForgeFlowPoint b, ForgeFlowPoint da, ForgeFlowPoint db) {
    const auto escape = [&](ForgeFlowPoint p, ForgeFlowPoint direction) {
        double distance=24;
        for (const auto& obstacle:f.obstacles) {
            auto box=obstacle.box;box.left+=16;box.top+=16;box.right-=16;box.bottom-=16;
            double gap=1e9;
            if (direction.x>0 && p.y>box.top && p.y<box.bottom && box.left>p.x) gap=box.left-p.x;
            if (direction.x<0 && p.y>box.top && p.y<box.bottom && box.right<p.x) gap=p.x-box.right;
            if (direction.y>0 && p.x>box.left && p.x<box.right && box.top>p.y) gap=box.top-p.y;
            if (direction.y<0 && p.x>box.left && p.x<box.right && box.bottom<p.y) gap=p.y-box.bottom;
            distance=std::min(distance,gap/2);
        }
        return ForgeFlowPoint{p.x+direction.x*distance,p.y+direction.y*distance};
    };
    const auto s=escape(a,da),t=escape(b,db);
    std::vector<ForgeFlowPoint> best;
    double best_cost=1e100;
    const auto consider = [&](std::vector<ForgeFlowPoint> points) {
        points=forge_flow_simplify(std::move(points));
        if (points.size()<2) return;
        const auto start=points[1],end=points[points.size()-2];
        if ((start.x-a.x)*da.x+(start.y-a.y)*da.y<=0 || (end.x-b.x)*db.x+(end.y-b.y)*db.y<=0) return;
        double cost=16.*(points.size()-2);
        for (std::size_t i=1;i<points.size();++i) cost+=std::hypot(points[i].x-points[i-1].x,points[i].y-points[i-1].y);
        if (cost>=best_cost || !forge_flow_clear_route(f,points)) return;
        best_cost=cost;best=std::move(points);
    };
    consider({a,s,{t.x,s.y},t,b});consider({a,s,{s.x,t.y},t,b});
    const auto horizontal = [&](double y){consider({a,s,{s.x,y},{t.x,y},t,b});};
    const auto vertical = [&](double x){consider({a,s,{x,s.y},{x,t.y},t,b});};
    horizontal((a.y+b.y)/2);vertical((a.x+b.x)/2);
    if (!best.empty()) return best;
    // 回折、同侧及被其他卡片遮挡的连接，尝试卡片外侧的确定性通道。
    std::set<double> xs,ys;
    for (const auto& obstacle:f.obstacles) {
        xs.insert(obstacle.box.left);xs.insert(obstacle.box.right);
        ys.insert(obstacle.box.top);ys.insert(obstacle.box.bottom);
    }
    for (double y:ys) horizontal(y);
    for (double x:xs) vertical(x);
    if (!best.empty()) return best;
    // 重叠卡片可能封住端口；仍保留端口朝向，等待用户移开卡片。
    return forge_flow_simplify({a,s,{s.x,(s.y+t.y)/2},{t.x,(s.y+t.y)/2},t,b});
}
static void forge_flow_sample_cubic(std::vector<ForgeFlowPoint>& points, ForgeFlowPoint a, ForgeFlowPoint c, ForgeFlowPoint d, ForgeFlowPoint b, int steps=32) {
    if (points.empty()) points.push_back(a);
    for (int i=1;i<=steps;++i) {const double t=i/double(steps),u=1-t;points.push_back({u*u*u*a.x+3*u*u*t*c.x+3*u*t*t*d.x+t*t*t*b.x,u*u*u*a.y+3*u*u*t*c.y+3*u*t*t*d.y+t*t*t*b.y});}
}
static ForgeFlowEdgeGeometry& forge_flow_edge_geometry(ForgeFlowCore& f, const ForgeFlowJson& e) {
    const auto& n1 = forge_flow_node(f, forge_flow_id(e, "source_node")); const auto& n2 = forge_flow_node(f, forge_flow_id(e, "target_node"));
    const auto& p1 = forge_flow_port(n1, forge_flow_id(e, "source_port")); const auto& p2 = forge_flow_port(n2, forge_flow_id(e, "target_port"));
    auto a = forge_flow_anchor(n1, p1), b = forge_flow_anchor(n2, p2);
    ForgeFlowJson ak = {a.x,a.y,p1["side"],n1["width"],n1["height"]}, bk = {b.x,b.y,p2["side"],n2["width"],n2["height"]};
    auto& g = f.geometry[forge_flow_id(e)];
    if (g.valid && g.source == ak && g.target == bk && g.edge == e) return g;
    g.source = ak; g.target = bk; g.edge = e; g.points.clear();g.valid=true;
    std::ostringstream path; path.imbue(std::locale::classic()); path << std::fixed << std::setprecision(3) << "M " << a.x << ' ' << a.y;
    const int style = e.value("style", 1);
    const auto da=forge_flow_vector(p1["side"].get<int>()),db=forge_flow_vector(p2["side"].get<int>());
    const double distance=std::max(24.,std::hypot(b.x-a.x,b.y-a.y)*.4);
    const ForgeFlowPoint c{a.x+da.x*distance,a.y+da.y*distance},d{b.x+db.x*distance,b.y+db.y*distance};
    if(style==1)forge_flow_sample_cubic(g.points,a,c,d,b);
    if(std::hypot(b.x-a.x,b.y-a.y)<.001){
        g.points={a,b};path<<" L "<<b.x<<' '<<b.y;
    }else if(style==1 && forge_flow_clear_route(f,g.points)) {
        path << " C " << c.x << ' ' << c.y << ' ' << d.x << ' ' << d.y << ' ' << b.x << ' ' << b.y;
    }else if(style!=0){
        auto route=forge_flow_orthogonal_route(f,a,b,da,db);g.points={a};
        for(std::size_t i=1;i<route.size();++i){
            if(style==1 && i+1<route.size()){
                const auto p=route[i-1],q=route[i],r=route[i+1];const double l1=std::hypot(q.x-p.x,q.y-p.y),l2=std::hypot(r.x-q.x,r.y-q.y);
                const double radius=std::min({12.,l1/2,l2/2});
                const ForgeFlowPoint enter{q.x+(p.x-q.x)*radius/l1,q.y+(p.y-q.y)*radius/l1},leave{q.x+(r.x-q.x)*radius/l2,q.y+(r.y-q.y)*radius/l2};
                path<<" L "<<enter.x<<' '<<enter.y<<" C "<<q.x<<' '<<q.y<<' '<<q.x<<' '<<q.y<<' '<<leave.x<<' '<<leave.y;
                g.points.push_back(enter);forge_flow_sample_cubic(g.points,enter,q,q,leave,8);
            }else{path<<" L "<<route[i].x<<' '<<route[i].y;g.points.push_back(route[i]);}
        }
    }else {g.points={a,b};path<<" L "<<b.x<<' '<<b.y;}
    // 几乎重合的端口可能在去重后仅剩一点，仍保留终点并使用端口方向确定箭头。
    if(g.points.size()<2){g.points.push_back(b);path<<" L "<<b.x<<' '<<b.y;}
    auto before=g.points[g.points.size()-2];
    double angle=std::hypot(b.x-before.x,b.y-before.y)<.001?std::atan2(-db.y,-db.x):std::atan2(b.y-before.y,b.x-before.x);
    std::ostringstream arrow; arrow.imbue(std::locale::classic());
    if (e.value("arrow",true)) arrow << "M " << b.x << ' ' << b.y << " L " << b.x-11*std::cos(angle-.45) << ' ' << b.y-11*std::sin(angle-.45) << " L " << b.x-11*std::cos(angle+.45) << ' ' << b.y-11*std::sin(angle+.45) << " Z";
    double length=0;for(std::size_t i=1;i<g.points.size();++i)length+=std::hypot(g.points[i].x-g.points[i-1].x,g.points[i].y-g.points[i-1].y);
    auto middle=a;double remaining=length/2;for(std::size_t i=1;i<g.points.size();++i){const auto u=g.points[i-1],v=g.points[i];const auto segment=std::hypot(v.x-u.x,v.y-u.y);if(segment>=remaining&&segment>0){const double t=remaining/segment;middle={u.x+(v.x-u.x)*t,u.y+(v.y-u.y)*t};break;}remaining-=segment;}
    g.view={{"id",e["id"]},{"path",path.str()},{"arrow_path",arrow.str()},{"style",style},{"arrow",e.value("arrow",true)},{"stroke_width",e.value("stroke_width",2.)},{"label",e.value("label","")},{"label_x",middle.x},{"label_y",middle.y},{"source_x",a.x},{"source_y",a.y},{"target_x",b.x},{"target_y",b.y}};
    double minx=a.x,maxx=a.x,miny=a.y,maxy=a.y; for (auto p:g.points) {minx=std::min(minx,p.x);maxx=std::max(maxx,p.x);miny=std::min(miny,p.y);maxy=std::max(maxy,p.y);}
    g.view.update({{"min_x",minx-12},{"min_y",miny-12},{"max_x",maxx+12},{"max_y",maxy+12}});
    return g;
}
static double forge_flow_segment_distance(ForgeFlowPoint p, ForgeFlowPoint a, ForgeFlowPoint b) {
    const double dx=b.x-a.x,dy=b.y-a.y, length=dx*dx+dy*dy;
    const double t=length>0?std::clamp(((p.x-a.x)*dx+(p.y-a.y)*dy)/length,0.,1.):0.;
    return std::hypot(p.x-a.x-t*dx,p.y-a.y-t*dy);
}
static std::string forge_flow_hit_edge(ForgeFlowCore& f, double x, double y, double tolerance) {
    double best=1e100; std::string id;
    for (const auto& e:f.document["edges"]) {
        if (forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(e,"source_node"))) || forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(e,"target_node")))) continue;
        const auto& g=forge_flow_edge_geometry(f,e);
        const double hit_width=tolerance+e.value("stroke_width",2.)/2;
        for (std::size_t i=1;i<g.points.size();++i) {const double d=forge_flow_segment_distance({x,y},g.points[i-1],g.points[i]);if(d<=hit_width&&d<best){best=d;id=forge_flow_id(e);}}
    }
    return id;
}
static std::pair<std::string,std::string> forge_flow_hit_port(ForgeFlowCore& f,double x,double y,int direction,const std::string& exclude,double tolerance) {
    double best=tolerance; std::pair<std::string,std::string> hit;
    for (const auto& n:f.document["nodes"]) {
        if(forge_flow_id(n)==exclude||forge_flow_hidden(f,n))continue;
        for(const auto& p:n["ports"]) {if(p["direction"].get<int>()==direction)continue;auto a=forge_flow_anchor(n,p);double d=std::hypot(x-a.x,y-a.y);if(d<best){best=d;hit={forge_flow_id(n),forge_flow_id(p)};}}
    }
    return hit;
}
static ForgeFlowJson forge_flow_content(const ForgeFlowJson& d) { auto content=d;content.erase("viewport");return content; }
static void forge_flow_history(ForgeFlowCore& f,const ForgeFlowJson& before) {
    if(forge_flow_content(before)==forge_flow_content(f.document))return;
    f.undo.push_back(forge_flow_content(before)); if(f.undo.size()>100)f.undo.erase(f.undo.begin());f.redo.clear();
}
static void forge_flow_restore(ForgeFlowCore& f,ForgeFlowJson content) {content["viewport"]=f.document["viewport"];f.document=std::move(content);forge_flow_reindex(f);}
static ForgeFlowJson forge_flow_view(ForgeFlowCore& f,bool full,bool ok=true,const std::string& error="") {
    forge_flow_sync_obstacles(f);
    ForgeFlowJson out={{"ok",ok},{"error",error},{"full",full},{"revision",f.revision},{"mode",f.document["mode"]},{"viewport",f.document["viewport"]},{"can_undo",!f.undo.empty()},{"can_redo",!f.redo.empty()},{"nodes",ForgeFlowJson::array()},{"edges",ForgeFlowJson::array()},{"removed_nodes",ForgeFlowJson::array()},{"removed_edges",ForgeFlowJson::array()},{"selected_id",""},{"selected_title",""}};
    out["selected_edge"]=ForgeFlowJson::object();out["selected_edge_count"]=0;
    std::set<std::string> node_ids,edge_ids;double minx=0,miny=0,maxx=1,maxy=1;bool first=true;
    for(const auto& n:f.document["nodes"]) {
        const auto id=forge_flow_id(n);node_ids.insert(id);auto row=n;row["data_json"]=n["data"].dump();row.erase("data");row["selected"]=f.selected_nodes.contains(id);row["hidden"]=forge_flow_hidden(f,n);
        if(row["selected"]==true && out["selected_id"]==""){out["selected_id"]=id;out["selected_title"]=n["title"];}
        if(!row["hidden"].get<bool>()){double x=n["x"],y=n["y"],w=n["width"],h=n["height"];if(first){minx=x;miny=y;maxx=x+w;maxy=y+h;first=false;}else{minx=std::min(minx,x);miny=std::min(miny,y);maxx=std::max(maxx,x+w);maxy=std::max(maxy,y+h);}}
        if(full||!f.last_nodes.contains(id)||f.last_nodes[id]!=row)out["nodes"].push_back(row);f.last_nodes[id]=std::move(row);
    }
    for(const auto& e:f.document["edges"]) {
        auto row=forge_flow_edge_geometry(f,e).view;const auto id=forge_flow_id(e);edge_ids.insert(id);row["selected"]=f.selected_edges.contains(id);row["hidden"]=forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(e,"source_node")))||forge_flow_hidden(f,forge_flow_node(f,forge_flow_id(e,"target_node")));
        if(row["selected"]==true && row["hidden"]==false){if(out["selected_edge"].empty())out["selected_edge"]=row;out["selected_edge_count"]=out["selected_edge_count"].get<int>()+1;}
        if(row["hidden"]==false){minx=std::min(minx,row["min_x"].get<double>());miny=std::min(miny,row["min_y"].get<double>());maxx=std::max(maxx,row["max_x"].get<double>());maxy=std::max(maxy,row["max_y"].get<double>());}
        if(full||!f.last_edges.contains(id)||f.last_edges[id]!=row)out["edges"].push_back(row);f.last_edges[id]=std::move(row);
    }
    for(auto it=f.last_nodes.begin();it!=f.last_nodes.end();)if(!node_ids.contains(it->first)){out["removed_nodes"].push_back(it->first);f.selected_nodes.erase(it->first);it=f.last_nodes.erase(it);}else ++it;
    for(auto it=f.last_edges.begin();it!=f.last_edges.end();)if(!edge_ids.contains(it->first)){out["removed_edges"].push_back(it->first);f.selected_edges.erase(it->first);f.geometry.erase(it->first);it=f.last_edges.erase(it);}else ++it;
    out["bounds"]={{"x",minx},{"y",miny},{"width",maxx-minx},{"height",maxy-miny}};
    out["node_count"]=node_ids.size();out["edge_count"]=edge_ids.size();
    f.summary=out;for(const auto* key:{"nodes","edges","removed_nodes","removed_edges"})f.summary[key]=ForgeFlowJson::array();
    return out;
}
// 连续拖动只产生涉及节点及相邻连线的行更新；视口变化不重新生成任何模型行。
static ForgeFlowJson forge_flow_motion_view(ForgeFlowCore& f,bool moved) {
    auto out=f.summary;out["ok"]=true;out["error"]="";out["full"]=false;out["revision"]=f.revision;out["viewport"]=f.document["viewport"];
    if(!moved)return out;
    forge_flow_sync_obstacles(f);
    if(f.adjacency_dirty){f.adjacency.clear();for(std::size_t i=0;i<f.document["edges"].size();++i){const auto& e=f.document["edges"][i];f.adjacency[forge_flow_id(e,"source_node")].push_back(i);f.adjacency[forge_flow_id(e,"target_node")].push_back(i);}f.adjacency_dirty=false;}
    std::set<std::size_t> changed;
    for(const auto& [id,p]:f.drag_origins){const auto& n=forge_flow_node(f,id);auto& row=f.last_nodes.at(id);if(row["x"]==n["x"]&&row["y"]==n["y"])continue;row["x"]=n["x"];row["y"]=n["y"];out["nodes"].push_back(row);for(auto edge:f.adjacency[id])changed.insert(edge);}
    for(std::size_t i=0;i<f.document["edges"].size();++i)if(!f.geometry.at(forge_flow_id(f.document["edges"][i])).valid)changed.insert(i);
    for(auto i:changed){const auto& e=f.document["edges"][i];const auto id=forge_flow_id(e);auto& row=f.last_edges.at(id);auto next=forge_flow_edge_geometry(f,e).view;next["hidden"]=row["hidden"];next["selected"]=row["selected"];if(next!=row){row=std::move(next);out["edges"].push_back(row);}}
    bool first=true;double minx=0,miny=0,maxx=1,maxy=1;
    for(const auto& n:f.document["nodes"]){if(f.last_nodes.at(forge_flow_id(n))["hidden"]==true)continue;double x=n["x"],y=n["y"],w=n["width"],h=n["height"];if(first){minx=x;miny=y;maxx=x+w;maxy=y+h;first=false;}else{minx=std::min(minx,x);miny=std::min(miny,y);maxx=std::max(maxx,x+w);maxy=std::max(maxy,y+h);}}
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
    forge_flow_validate(f);f.undo.clear();f.redo.clear();
}
static bool forge_flow_apply(ForgeFlowCore& f,const ForgeFlowJson& a) {
    const auto op=a.value("op","");const auto id=forge_flow_id(a);const double x=forge_flow_number(a,"x"),y=forge_flow_number(a,"y");const bool tree=f.document["mode"]=="mindmap";
    if(op=="demo"){forge_flow_demo(f,a.value("number",0),a.value("count",0));return true;}
    if(op=="load"){auto parsed=ForgeFlowJson::parse(a.at("text").get<std::string>());f.document=std::move(parsed);forge_flow_validate(f);f.selected_nodes.clear();f.selected_edges.clear();f.undo.clear();f.redo.clear();return true;}
    if(op=="undo"||op=="redo") {forge_flow_require(!f.dragging,"拖动期间不能撤销");auto& from=op=="undo"?f.undo:f.redo;auto& to=op=="undo"?f.redo:f.undo;if(!from.empty()){to.push_back(forge_flow_content(f.document));auto d=from.back();from.pop_back();forge_flow_restore(f,d);}return true;}
    if(op=="view"){double z=forge_flow_number(a,"zoom",1);f.document["viewport"]={{"x",x},{"y",y},{"zoom",std::clamp(z,.1,4.)}};return false;}
    if(op=="select"||op=="select-all"||op=="select-rect"||op=="pick") {
        const bool add=a.value("additive",false);if(!add){f.selected_nodes.clear();f.selected_edges.clear();}
        if(op=="select"){forge_flow_node(f,id);if(add&&f.selected_nodes.contains(id))f.selected_nodes.erase(id);else f.selected_nodes.insert(id);}
        if(op=="select-all"||op=="select-rect")for(const auto& n:f.document["nodes"]){if(forge_flow_hidden(f,n))continue;double nx=n["x"],ny=n["y"],w=n["width"],h=n["height"];if(op=="select-all"||(nx>=std::min(x,a.value("width",x))&&ny>=std::min(y,a.value("height",y))&&nx+w<=std::max(x,a.value("width",x))&&ny+h<=std::max(y,a.value("height",y))))f.selected_nodes.insert(forge_flow_id(n));}
        if(op=="pick"){auto edge=forge_flow_hit_edge(f,x,y,8./f.document["viewport"]["zoom"].get<double>());if(!edge.empty())f.selected_edges.insert(edge);}
        return false;
    }
    if(op=="drag-begin") {
        forge_flow_node(f,id);if(!f.selected_nodes.contains(id)){if(!a.value("additive",false))f.selected_nodes.clear();f.selected_nodes.insert(id);}f.selected_edges.clear();f.drag_before=f.document;f.drag_pointer={x,y};f.drag_origins.clear();f.dragging=true;
        for(const auto& n:f.document["nodes"]){auto nid=forge_flow_id(n);bool included=f.selected_nodes.contains(nid);auto parent=forge_flow_id(n,"parent");while(!included&&!parent.empty()){included=f.selected_nodes.contains(parent);parent=forge_flow_id(forge_flow_node(f,parent),"parent");}if(included)f.drag_origins[nid]={n["x"],n["y"]};}return false;
    }
    if(op=="drag-move") {if(f.dragging){double dx=x-f.drag_pointer.x,dy=y-f.drag_pointer.y;if(a.value("flag",false)){dx=std::round(dx/20)*20;dy=std::round(dy/20)*20;}for(const auto& [nid,p]:f.drag_origins){auto& n=forge_flow_node(f,nid);n["x"]=p.x+dx;n["y"]=p.y+dy;}}return false;}
    if(op=="drag-end"||op=="cancel") {if(f.dragging){if(op=="cancel"){f.document=f.drag_before;forge_flow_reindex(f);}else forge_flow_history(f,f.drag_before);f.dragging=false;f.drag_origins.clear();}return false;}
    forge_flow_require(!f.dragging,"请先结束当前拖动");const auto before=f.document;
    if(op=="insert") {
        auto n=forge_flow_normalize_node(a.contains("node") ? a.at("node") : ForgeFlowJson::parse(a.at("text").get<std::string>()),tree);forge_flow_require(!f.indices.contains(forge_flow_id(n)),"节点 ID 重复");f.document["nodes"].push_back(n);forge_flow_reindex(f);if(tree)forge_flow_tree_edges(f);forge_flow_validate(f);if(tree)forge_flow_layout(f);
    }else if(op=="remove-edge") {
        forge_flow_require(!tree,"思维导图请删除分支或修改父级");auto& edges=f.document["edges"];auto it=std::find_if(edges.begin(),edges.end(),[&](const auto& e){return forge_flow_id(e)==id;});forge_flow_require(it!=edges.end(),"连线不存在");edges.erase(it);
    }else if(op=="add"||op=="add-child"||op=="add-sibling") {
        std::string parent;
        if(tree&&!f.document["nodes"].empty()){forge_flow_require(!id.empty(),"请选择父主题");parent=op=="add-sibling"?forge_flow_id(forge_flow_node(f,id),"parent"):id;forge_flow_require(!parent.empty(),"根主题不能添加同级主题");}
        auto create=a;create["id"]=a.value("new_id","");auto n=forge_flow_make_node(f,create,parent);auto nid=forge_flow_id(n);f.document["nodes"].push_back(n);forge_flow_reindex(f);f.selected_nodes={nid};f.selected_edges.clear();if(tree)forge_flow_layout(f);
    } else if(op=="node"||op=="data"||op=="field"||op=="title"||op=="collapse"||op=="reparent") {
        auto& n=forge_flow_node(f,id);
        if(op=="node"){auto replacement=a.contains("node") ? a.at("node") : ForgeFlowJson::parse(a.at("text").get<std::string>());forge_flow_require(forge_flow_id(replacement)==id,"节点 ID 不可修改");n=forge_flow_normalize_node(replacement,tree);}
        if(op=="data"){auto data=ForgeFlowJson::parse(a.at("text").get<std::string>());forge_flow_require(data.is_object(),"业务数据必须为对象");n["data"]=data;}
        if(op=="field"){const auto key=a.value("target","");forge_flow_require(!key.empty(),"数据字段不能为空");n["data"][key]=a.contains("value")?a["value"]:ForgeFlowJson(a.value("text",""));}
        if(op=="title")n["title"]=a.value("text","");
        if(op=="collapse"){forge_flow_require(tree,"仅思维导图支持分支折叠");n["collapsed"]=!n.value("collapsed",false);}
        if(op=="reparent"){forge_flow_require(tree&&!forge_flow_id(n,"parent").empty(),"根节点不能修改父级");n["parent"]=a.value("target","");}
        if(tree&&op=="reparent")forge_flow_tree_edges(f);forge_flow_validate(f);if(tree)forge_flow_layout(f);
    } else if(op=="reconnect") {
        forge_flow_require(!tree,"思维导图通过修改父级重连");
        bool found=false;for(auto& e:f.document["edges"])if(forge_flow_id(e)==a.value("text","")){auto edited=e;edited["source_node"]=id;edited["source_port"]=a.value("port","");edited["target_node"]=a.value("target","");edited["target_port"]=a.value("target_port","");forge_flow_validate_connection(f,edited,forge_flow_id(e));e=edited;found=true;}forge_flow_require(found,"连线不存在");
    } else if(op=="connect"||op=="connect-at"||op=="reconnect-at") {
        forge_flow_require(!tree,"思维导图通过父子主题建立连线");ForgeFlowJson e;
        if(op=="reconnect-at") {for(const auto& v:f.document["edges"])if(forge_flow_id(v)==id)e=v;forge_flow_require(!e.is_null(),"连线不存在");const bool start=a.value("number",1)==0;auto hit=forge_flow_hit_port(f,x,y,start?0:1,forge_flow_id(e,start?"target_node":"source_node"),16./f.document["viewport"]["zoom"].get<double>());forge_flow_require(!hit.first.empty(),"请拖到有效端口");e[start?"source_node":"target_node"]=hit.first;e[start?"source_port":"target_port"]=hit.second;forge_flow_validate_connection(f,e,id);for(auto& v:f.document["edges"])if(forge_flow_id(v)==id)v=e;
        } else {
            std::string target=a.value("target",""),target_port=a.value("target_port","");
            if(op=="connect-at"){auto hit=forge_flow_hit_port(f,x,y,1,id,16./f.document["viewport"]["zoom"].get<double>());target=hit.first;target_port=hit.second;forge_flow_require(!target.empty(),"请拖到有效输入端口");}
            e={{"id",forge_flow_new_id(f,"edge-")},{"source_node",id},{"source_port",a.value("port","")},{"target_node",target},{"target_port",target_port},{"label",a.value("text","")},{"style",forge_flow_integer(a,"number",1)},{"arrow",a.value("flag",true)}};forge_flow_validate_connection(f,e);f.document["edges"].push_back(e);
        }
    } else if(op=="edge-appearance") {
        ForgeFlowJson patch;
        if(a.contains("appearance"))patch=a.at("appearance");
        else {
            const auto key=a.value("target","");
            if(key=="style")patch={{key,forge_flow_integer(a,"number",1)}};
            else if(key=="arrow")patch={{key,a.value("flag",true)}};
            else if(key=="label")patch={{key,a.value("text","")}};
            else if(key=="stroke_width")patch={{key,forge_flow_number(a,"width",2)}};
        }
        forge_flow_require(patch.is_object()&&!patch.empty(),"请指定连线样式属性");
        for(const auto& [key,value]:patch.items())forge_flow_require(key=="style"||key=="arrow"||key=="label"||key=="stroke_width","不支持的连线样式属性");
        bool found=false;
        for(auto& e:f.document["edges"])if(id.empty()?f.selected_edges.contains(forge_flow_id(e)):forge_flow_id(e)==id){e.update(patch);forge_flow_validate_connection(f,e,forge_flow_id(e),false);found=true;}
        forge_flow_require(found,"请先选中连线");
    } else if(op=="edge") {bool found=false;for(auto& e:f.document["edges"])if(forge_flow_id(e)==id){auto edit=a.contains("edge") ? a.at("edge") : ForgeFlowJson::parse(a.at("text").get<std::string>());forge_flow_require(forge_flow_id(edit)==id,"连线 ID 不可修改");e=edit;found=true;}forge_flow_require(found,"连线不存在");forge_flow_validate(f);
    } else if(op=="delete") {
        auto removed=f.selected_nodes;bool again=true;while(again){again=false;for(const auto& n:f.document["nodes"])if(removed.contains(forge_flow_id(n,"parent"))&&removed.insert(forge_flow_id(n)).second)again=true;}
        auto& nodes=f.document["nodes"];nodes.erase(std::remove_if(nodes.begin(),nodes.end(),[&](const auto& n){return removed.contains(forge_flow_id(n));}),nodes.end());
        auto& edges=f.document["edges"];edges.erase(std::remove_if(edges.begin(),edges.end(),[&](const auto& e){return f.selected_edges.contains(forge_flow_id(e))||removed.contains(forge_flow_id(e,"source_node"))||removed.contains(forge_flow_id(e,"target_node"));}),edges.end());f.selected_nodes.clear();f.selected_edges.clear();forge_flow_reindex(f);if(tree)forge_flow_layout(f);
    } else if(op=="duplicate") {
        forge_flow_require(!tree,"思维导图请使用新增主题");std::map<std::string,std::string> map;auto original=f.document;
        for(const auto& n:original["nodes"])if(f.selected_nodes.contains(forge_flow_id(n))){auto c=n;auto nid=forge_flow_new_id(f,"node-");map[forge_flow_id(n)]=nid;c["id"]=nid;c["x"]=n["x"].get<double>()+32;c["y"]=n["y"].get<double>()+32;f.document["nodes"].push_back(c);forge_flow_reindex(f);}
        for(const auto& e:original["edges"])if(map.contains(forge_flow_id(e,"source_node"))&&map.contains(forge_flow_id(e,"target_node"))){auto c=e;c["id"]=forge_flow_new_id(f,"edge-");c["source_node"]=map[forge_flow_id(e,"source_node")];c["target_node"]=map[forge_flow_id(e,"target_node")];f.document["edges"].push_back(c);}f.selected_nodes.clear();for(const auto& [old,nid]:map)f.selected_nodes.insert(nid);
    } else if(op=="layout") {forge_flow_layout(f);
    } else if(op!="refresh") throw std::runtime_error("未知编辑操作");
    forge_flow_history(f,before);return op=="delete"||op=="duplicate"||op=="add"||op=="add-child"||op=="add-sibling"||op=="layout"||op=="reparent";
}

extern "C" ForgeFlowCore* forge_flow_create(const char* mode) {
    try{auto* f=new ForgeFlowCore;f->document=forge_flow_empty(mode&&std::string(mode)=="mindmap"?"mindmap":"graph");return f;}catch(...){return nullptr;}
}
extern "C" void forge_flow_destroy(ForgeFlowCore* f){delete f;}
extern "C" const char* forge_flow_command(ForgeFlowCore* f,const char* command) {
    if(!f)return "{\"ok\":false,\"error\":\"文档句柄无效\"}";
    ForgeFlowJson before;const auto old_nodes=f->selected_nodes,old_edges=f->selected_edges;
    try{const auto a=ForgeFlowJson::parse(command?command:"");
        const auto op=a.value("op","");const bool motion=(op=="drag-move"||op=="view")&&!f->summary.is_null();
        if(!motion)before=f->document;
        if(a.value("op","")=="resolve") {
            auto proposal=a; proposal["op"]="";
            if(a.value("target","")=="reconnect") {
                for(const auto& e:f->document["edges"])if(forge_flow_id(e)==forge_flow_id(a)) {
                    const bool start=a.value("number",1)==0;
                    auto hit=forge_flow_hit_port(*f,a.value("x",0.),a.value("y",0.),start?0:1,forge_flow_id(e,start?"target_node":"source_node"),16./f->document["viewport"]["zoom"].get<double>());
                    if(!hit.first.empty()){proposal.update({{"op","reconnect"},{"text",e["id"]},{"id",start?hit.first:forge_flow_id(e,"source_node")},{"port",start?hit.second:forge_flow_id(e,"source_port")},{"target",start?forge_flow_id(e,"target_node"):hit.first},{"target_port",start?forge_flow_id(e,"target_port"):hit.second}});}break;
                }
            }else{
                auto hit=forge_flow_hit_port(*f,a.value("x",0.),a.value("y",0.),1,forge_flow_id(a),16./f->document["viewport"]["zoom"].get<double>());
                if(!hit.first.empty())proposal.update({{"op","connect"},{"target",hit.first},{"target_port",hit.second}});
            }
            f->result=proposal.dump();return f->result.c_str();
        }
        const bool full=forge_flow_apply(*f,a);++f->revision;
        if(motion)f->result=forge_flow_motion_view(*f,op=="drag-move").dump();
        else {f->adjacency_dirty=true;f->result=forge_flow_view(*f,full).dump();}}
    catch(const std::exception& e){if(!before.is_null())f->document=before;f->selected_nodes=old_nodes;f->selected_edges=old_edges;forge_flow_reindex(*f);f->result=forge_flow_view(*f,false,false,e.what()).dump();}
    catch(...){if(!before.is_null())f->document=before;forge_flow_reindex(*f);f->result=forge_flow_view(*f,false,false,"图编辑失败").dump();}
    return f->result.c_str();
}
extern "C" const char* forge_flow_snapshot(ForgeFlowCore* f){if(!f)return "{}";try{f->result=forge_flow_view(*f,true).dump();}catch(...){f->result="{}";}return f->result.c_str();}
extern "C" const char* forge_flow_export(ForgeFlowCore* f){if(!f)return "{}";try{f->result=f->document.dump(2);}catch(...){f->result="{}";}return f->result.c_str();}
