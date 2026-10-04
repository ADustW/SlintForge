#include "ForgeGrids.hpp"
#include <algorithm>
#include <cmath>
#include <charconv>
#include <limits>
#include <numeric>
#include <unordered_map>
#include <unordered_set>

static double forge_grid_bounded(double value, double fallback, double minimum, double maximum) {
    return std::isfinite(value) && value > 0 ? std::clamp(value, minimum, maximum) : fallback;
}
static std::string forge_grid_lower(std::string text) {
    for (auto& c : text) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    return text;
}
static std::string forge_grid_trim(const std::string& text) {
    const auto begin = text.find_first_not_of(" \t\n\r\f\v");
    return begin == std::string::npos ? std::string{} : text.substr(begin, text.find_last_not_of(" \t\n\r\f\v") - begin + 1);
}
static int forge_grid_compare_text(const std::string& left, const std::string& right) {
    // 小兄弟集合直接比较折叠后的字节，遇到差异即返回，不分配临时字符串。
    const auto count = std::min(left.size(), right.size());
    for (std::size_t index = 0; index < count; ++index) {
        auto a = static_cast<unsigned char>(left[index]);
        auto b = static_cast<unsigned char>(right[index]);
        if (a >= 'A' && a <= 'Z') a += 'a' - 'A';
        if (b >= 'A' && b <= 'Z') b += 'a' - 'A';
        if (a != b) return a < b ? -1 : 1;
    }
    return left.size() < right.size() ? -1 : left.size() > right.size() ? 1 : 0;
}
static int forge_grid_compare(const ForgeGridRowData& a, const ForgeGridRowData& b, std::size_t column, bool numeric, bool descending,const std::string& left_key,const std::string& right_key,bool use_keys) {
    const auto left = column < a.cells.size() ? &a.cells[column] : nullptr;
    const auto right = column < b.cells.size() ? &b.cells[column] : nullptr;
    const auto valid = [numeric](const ForgeGridCellData* c) { return c && (!numeric || std::isfinite(c->value)); };
    if (valid(left) != valid(right)) return valid(left) ? -1 : 1;
    int result = 0;
    if (left && right) {
        if (numeric) result = left->value < right->value ? -1 : left->value > right->value ? 1 : 0;
        else if(use_keys){ result = left_key < right_key ? -1 : left_key > right_key ? 1 : 0; }
        else result = forge_grid_compare_text(left->text, right->text);
    }
    return descending ? -result : result;
}
ForgeGridSceneData forge_prepare_grid(const std::vector<ForgeGridRowData>& rows,
    const std::vector<ForgeGridColumnData>& columns, bool tree, int sort_column, bool descending, const std::string& filter) {
    ForgeGridSceneData scene{};
    scene.selected_count = static_cast<int>(std::count_if(rows.begin(), rows.end(), [](const auto& r) { return r.selected; }));
    std::unordered_set<std::string> column_ids;
    for (const auto& c : columns) if (c.id.empty() || !column_ids.insert(c.id).second) { scene.status = 2; return scene; }
    for (bool frozen : {true, false}) {
        for (std::size_t index = 0; index < columns.size(); ++index) {
            const auto& c = columns[index];
            if (c.hidden || c.frozen != frozen) continue;
            const auto minimum = forge_grid_bounded(c.min_width, 40, 32, 2048);
            const auto maximum = std::max(minimum, forge_grid_bounded(c.max_width, 4096, minimum, 4096));
            const auto width = std::clamp(forge_grid_bounded(c.width, 140, minimum, maximum), minimum, maximum);
            scene.columns.push_back({c, static_cast<int>(index), scene.total_width, width});
            scene.total_width += width;
        }
        if (frozen) scene.frozen_width = scene.total_width;
    }
    std::unordered_map<std::string, std::size_t> ids;
    for (std::size_t i = 0; i < rows.size(); ++i) if (rows[i].id.empty() || !ids.emplace(rows[i].id, i).second) { scene.status = 2; return scene; }
    std::vector<std::vector<std::size_t>> children(rows.size());
    std::vector<std::size_t> roots;
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (tree && !rows[i].parent_id.empty()) {
            const auto parent = ids.find(rows[i].parent_id);
            if (parent == ids.end()) { scene.status = 2; return scene; }
            children[parent->second].push_back(i);
        } else roots.push_back(i);
    }
    // 迭代拓扑遍历同时检查循环，深层树不使用递归。
    auto order = roots;
    for (std::size_t cursor = 0; cursor < order.size(); ++cursor) {
        const auto& descendants = children[order[cursor]]; order.insert(order.end(), descendants.begin(), descendants.end());
    }
    if (order.size() != rows.size()) { scene.status = 2; return scene; }
    const auto query = forge_grid_lower(forge_grid_trim(filter));
    std::vector<bool> included(rows.size());
    for (std::size_t i = 0; i < rows.size(); ++i) included[i] = query.empty() || std::any_of(rows[i].cells.begin(), rows[i].cells.end(),
        [&](const auto& c) { return forge_grid_lower(c.text).find(query) != std::string::npos; });
    if (tree && !query.empty()) for (auto it = order.rbegin(); it != order.rend(); ++it) {
        const auto i = *it; if (included[i] && !rows[i].parent_id.empty()) included[ids.at(rows[i].parent_id)] = true;
    }
    if (sort_column >= 0 && static_cast<std::size_t>(sort_column) < columns.size() && columns[sort_column].sortable) {
        // 每行只折叠一次，避免每次比较重复分配字符串。
        const bool use_keys=!columns[sort_column].numeric&&(roots.size()>=32||std::any_of(children.begin(),children.end(),[](const auto& group){return group.size()>=32;}));
        std::vector<std::string> text_keys;if(use_keys){text_keys.reserve(rows.size());for(const auto& row:rows)text_keys.push_back(static_cast<size_t>(sort_column)<row.cells.size()?forge_grid_lower(row.cells[sort_column].text):std::string{});}const std::string empty_key;
        const auto less = [&](std::size_t a, std::size_t b) {
            const auto order = forge_grid_compare(rows[a], rows[b], sort_column, columns[sort_column].numeric, descending,text_keys.empty()?empty_key:text_keys[a],text_keys.empty()?empty_key:text_keys[b],use_keys);
            return order == 0 ? a < b : order < 0;
        };
        std::sort(roots.begin(), roots.end(), less);
        for (auto& siblings : children) std::sort(siblings.begin(), siblings.end(), less);
    }
    std::vector<std::pair<std::size_t, int>> stack;
    for (auto it = roots.rbegin(); it != roots.rend(); ++it) stack.emplace_back(*it, 0);
    while (!stack.empty()) {
        const auto [index, depth] = stack.back(); stack.pop_back();
        if (!included[index]) continue;
        const bool expanded = rows[index].expanded || !query.empty();
        auto row = rows[index]; row.cells.resize(std::max(columns.size(), row.cells.size()));
        scene.rows.push_back({std::move(row), static_cast<int>(index), depth, !children[index].empty(), expanded});
        if (expanded) for (auto it = children[index].rbegin(); it != children[index].rend(); ++it) stack.emplace_back(*it, depth + 1);
    }
    const auto selectable = std::count_if(scene.rows.begin(), scene.rows.end(), [](const auto& v) { return !v.row.disabled; });
    scene.all_visible_selected = selectable > 0 && std::all_of(scene.rows.begin(), scene.rows.end(), [](const auto& v) { return v.row.disabled || v.row.selected; });
    if (scene.rows.empty() || scene.columns.empty()) scene.status = 1;
    return scene;
}
std::vector<ForgeGridRowData> forge_update_grid(const std::vector<ForgeGridRowData>& rows,
    const std::vector<int>& visible, int operation, int target, int anchor) {
    auto result = rows;
    const auto valid = [&](int i) { return i >= 0 && static_cast<std::size_t>(i) < rows.size(); };
    if (operation == 11) {
        std::unordered_set<std::string> removed;
        for (const auto& r : rows) if (r.selected) removed.insert(r.id);
        if (target == 1) {
            std::unordered_map<std::string, std::vector<std::string>> children;
            for (const auto& r : rows) children[r.parent_id].push_back(r.id);
            std::vector<std::string> queue(removed.begin(), removed.end());
            for (std::size_t cursor = 0; cursor < queue.size(); ++cursor) {
                for (const auto& id : children[queue[cursor]]) if (removed.insert(id).second) queue.push_back(id);
            }
        }
        std::erase_if(result, [&](const auto& r) { return removed.contains(r.id); }); return result;
    }
    if (operation == 5) { for (auto& r : result) r.selected = false; return result; }
    if (operation == 9 || operation == 10) { for (auto& r : result) if (!r.disabled) r.expanded = operation == 9; return result; }
    if (operation == 4 || operation == 12) {
        const bool all = std::all_of(visible.begin(), visible.end(), [&](int i) { return !valid(i) || rows[i].disabled || rows[i].selected; });
        for (int i : visible) if (valid(i) && !result[i].disabled) result[i].selected = operation == 12 || !all;
        return result;
    }
    if (!valid(target) || rows[target].disabled) return result;
    if (operation == 0) { for (auto& r : result) r.selected = false; result[target].selected = true; }
    else if (operation == 1) result[target].selected = !result[target].selected;
    else if (operation == 2 || operation == 3) {
        const auto end = std::find(visible.begin(), visible.end(), target);
        if (end == visible.end()) return result;
        auto begin = std::find(visible.begin(), visible.end(), anchor); if (begin == visible.end()) begin = end;
        if (operation == 2) for (auto& r : result) r.selected = false;
        for (auto it = std::min(begin, end); it != std::max(begin, end) + 1; ++it) if (valid(*it) && !result[*it].disabled) result[*it].selected = true;
    } else if (operation == 6) result[target].expanded = !result[target].expanded;
    else if (operation == 7) result[target].expanded = true;
    else if (operation == 8) result[target].expanded = false;
    return result;
}
ForgeGridEditData forge_commit_grid(const std::vector<ForgeGridRowData>& rows,
    const std::vector<ForgeGridColumnData>& columns, int row, int column, const std::string& text) {
    ForgeGridEditData result{1, rows};
    if (row < 0 || column < 0 || static_cast<std::size_t>(row) >= rows.size() || static_cast<std::size_t>(column) >= columns.size()
        || rows[row].disabled || !columns[column].editable || columns[column].hidden) return result;
    double value = static_cast<std::size_t>(column) < rows[row].cells.size() ? rows[row].cells[column].value : 0;
    if (columns[column].numeric) {
        const auto input = forge_grid_trim(text);
        if (input.size() >= 2 && input[0] == '+' && (input[1] == '+' || input[1] == '-')) { result.status = 2; return result; }
        float number = 0;
        // from_chars 不接收前导加号，先处理以保持与 Rust 浮点解析一致。
        const auto first = input.data() + (!input.empty() && input[0] == '+' ? 1 : 0);
        const auto conversion = std::from_chars(first, input.data() + input.size(), number);
        if (conversion.ec != std::errc{} || conversion.ptr != input.data() + input.size() || !std::isfinite(number)) { result.status = 2; return result; }
        value = number;
    }
    result.rows[row].cells.resize(std::max(columns.size(), result.rows[row].cells.size()));
    result.rows[row].cells[column] = {text, value}; result.status = 0; return result;
}
int forge_step_grid(const std::vector<bool>& disabled, int current, int delta) {
    if (disabled.empty()) return -1;
    const int direction = delta < 0 ? -1 : 1;
    const auto candidate = static_cast<long long>(current) + delta;
    int index = current < 0 ? (direction > 0 ? 0 : static_cast<int>(disabled.size()) - 1)
        : static_cast<int>(std::clamp(candidate, 0LL, static_cast<long long>(disabled.size()) - 1));
    while (index >= 0 && static_cast<std::size_t>(index) < disabled.size()) { if (!disabled[index]) return index; index += direction; }
    return current >= 0 && static_cast<std::size_t>(current) < disabled.size() && !disabled[current] ? current : -1;
}
