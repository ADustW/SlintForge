// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeSdk.hpp"

#include <utility>

ForgeStatus ForgeRouter::create(const std::vector<std::string>& ids, const std::string& initial, ForgeRouter& out) {
    ForgeRouter candidate;
    for (const auto& id : ids) {
        if (id.empty()) return ForgeStatus::failure(ForgeErrorCode::EmptyId, "路由 ID 不能为空");
        if (!candidate.registered_.insert(id).second)
            return ForgeStatus::failure(ForgeErrorCode::DuplicateId, "重复 ID: " + id);
    }
    if (initial.empty()) return ForgeStatus::failure(ForgeErrorCode::EmptyId, "初始路由 ID 不能为空");
    if (!candidate.registered_.contains(initial))
        return ForgeStatus::failure(ForgeErrorCode::UnknownRoute, "未知路由: " + initial);
    candidate.current_.id = initial;
    out = std::move(candidate);
    return ForgeStatus::success();
}

ForgeStatus ForgeRouter::ensure_route(const std::string& id) const {
    if (id.empty()) return ForgeStatus::failure(ForgeErrorCode::EmptyId, "路由 ID 不能为空");
    if (!registered_.contains(id)) return ForgeStatus::failure(ForgeErrorCode::UnknownRoute, "未知路由: " + id);
    return ForgeStatus::success();
}

ForgeStatus ForgeRouter::go(const std::string& id, std::vector<ForgeRouteArgument> params) {
    auto status = ensure_route(id);
    if (!status.ok()) return status;
    ForgeRouteState next{id, std::move(params)};
    if (next == current_) return ForgeStatus::success(false);
    history_.push_back(std::move(current_));
    current_ = std::move(next);
    return ForgeStatus::success(true);
}

ForgeStatus ForgeRouter::replace(const std::string& id, std::vector<ForgeRouteArgument> params) {
    auto status = ensure_route(id);
    if (!status.ok()) return status;
    ForgeRouteState next{id, std::move(params)};
    if (next == current_) return ForgeStatus::success(false);
    current_ = std::move(next);
    return ForgeStatus::success(true);
}

ForgeStatus ForgeRouter::back() {
    if (history_.empty()) return ForgeStatus::failure(ForgeErrorCode::NoHistory, "没有可返回的历史页面");
    current_ = std::move(history_.back());
    history_.pop_back();
    return ForgeStatus::success(true);
}

ForgeStatus ForgeNavTree::create(std::vector<ForgeNavItem> items, ForgeNavTree& out) {
    ForgeNavTree candidate;
    candidate.items_ = std::move(items);
    for (std::size_t i = 0; i < candidate.items_.size(); ++i) {
        const auto& item = candidate.items_[i];
        if (item.id.empty()) return ForgeStatus::failure(ForgeErrorCode::EmptyId, "导航 ID 不能为空");
        if (!candidate.by_id_.emplace(item.id, i).second)
            return ForgeStatus::failure(ForgeErrorCode::DuplicateId, "重复 ID: " + item.id);
    }
    std::vector<std::size_t> parents(candidate.items_.size(), candidate.items_.size());
    for (std::size_t i = 0; i < candidate.items_.size(); ++i) {
        const auto& item = candidate.items_[i];
        if (item.parent_id.empty()) {
            candidate.roots_.push_back(i);
            continue;
        }
        const auto parent = candidate.by_id_.find(item.parent_id);
        if (parent == candidate.by_id_.end())
            return ForgeStatus::failure(ForgeErrorCode::UnknownParent, "未知父节点: " + item.parent_id);
        if (candidate.items_[parent->second].kind != ForgeTreeItemKind::Group)
            return ForgeStatus::failure(ForgeErrorCode::InvalidParent, "父节点不是分组: " + item.parent_id);
        parents[i] = parent->second;
        candidate.children_[item.parent_id].push_back(i);
    }
    // 三色标记保证每个父链节点只检查一次，避免深层树的平方级开销。
    std::vector<unsigned char> colors(candidate.items_.size(), 0);
    std::vector<std::size_t> path;
    for (std::size_t i = 0; i < candidate.items_.size(); ++i) {
        auto cursor = i;
        while (cursor < candidate.items_.size()) {
            if (colors[cursor] == 2) break;
            if (colors[cursor] == 1)
                return ForgeStatus::failure(ForgeErrorCode::Cycle, "导航树存在环: " + candidate.items_[cursor].id);
            colors[cursor] = 1;
            path.push_back(cursor);
            cursor = parents[cursor];
        }
        for (const auto node : path) colors[node] = 2;
        path.clear();
    }
    out = std::move(candidate);
    return ForgeStatus::success();
}

ForgeStatus ForgeNavTree::set_expanded(const std::string& id, bool expanded) {
    if (id.empty()) return ForgeStatus::failure(ForgeErrorCode::EmptyId, "导航 ID 不能为空");
    const auto found = by_id_.find(id);
    if (found == by_id_.end()) return ForgeStatus::failure(ForgeErrorCode::UnknownRoute, "未知导航 ID: " + id);
    if (items_[found->second].kind != ForgeTreeItemKind::Group)
        return ForgeStatus::failure(ForgeErrorCode::NotGroup, "节点不是分组: " + id);
    const bool before = expanded_.contains(id);
    if (expanded) expanded_.insert(id); else expanded_.erase(id);
    return ForgeStatus::success(before != expanded);
}

ForgeStatus ForgeNavTree::toggle(const std::string& id) {
    return set_expanded(id, !expanded_.contains(id));
}

std::vector<ForgeVisibleNavItem> ForgeNavTree::visible_items() const {
    std::vector<ForgeVisibleNavItem> out;
    std::vector<std::pair<std::size_t, std::size_t>> stack;
    for (auto it = roots_.rbegin(); it != roots_.rend(); ++it) stack.emplace_back(*it, 0);
    // 显式栈保留先序顺序，不受系统线程栈大小限制。
    while (!stack.empty()) {
        const auto [index, depth] = stack.back();
        stack.pop_back();
        const auto& item = items_[index];
        const auto children = children_.find(item.id);
        const bool expanded = expanded_.contains(item.id);
        out.push_back({item, depth, expanded, children != children_.end() && !children->second.empty()});
        if (expanded && children != children_.end()) {
            for (auto it = children->second.rbegin(); it != children->second.rend(); ++it)
                stack.emplace_back(*it, depth + 1);
        }
    }
    return out;
}
