#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

enum class ForgeErrorCode {
    None,
    EmptyId,
    DuplicateId,
    UnknownRoute,
    UnknownParent,
    Cycle,
    InvalidParent,
    NotGroup,
    NoHistory,
};

struct ForgeStatus {
    ForgeErrorCode code = ForgeErrorCode::None;
    std::string message;
    bool changed = false;

    [[nodiscard]] bool ok() const { return code == ForgeErrorCode::None; }
    static ForgeStatus success(bool changed = false) { return {ForgeErrorCode::None, {}, changed}; }
    static ForgeStatus failure(ForgeErrorCode code, std::string message) {
        return {code, std::move(message), false};
    }
};

struct ForgeRouteArgument {
    std::string key;
    std::string value;
    bool operator==(const ForgeRouteArgument&) const = default;
};

struct ForgeRouteState {
    std::string id;
    std::vector<ForgeRouteArgument> params;
    bool operator==(const ForgeRouteState&) const = default;
};

class ForgeRouter {
public:
    static ForgeStatus create(const std::vector<std::string>& ids, const std::string& initial, ForgeRouter& out);
    ForgeStatus go(const std::string& id, std::vector<ForgeRouteArgument> params = {});
    ForgeStatus replace(const std::string& id, std::vector<ForgeRouteArgument> params = {});
    ForgeStatus back();
    [[nodiscard]] const ForgeRouteState& current() const { return current_; }
    [[nodiscard]] bool can_go_back() const { return !history_.empty(); }
    [[nodiscard]] std::size_t history_len() const { return history_.size(); }

private:
    ForgeStatus ensure_route(const std::string& id) const;
    std::unordered_set<std::string> registered_;
    ForgeRouteState current_;
    std::vector<ForgeRouteState> history_;
};

enum class ForgeTreeItemKind { Page, Group, Header, Separator };

struct ForgeNavItem {
    std::string id;
    std::string parent_id;
    std::string title;
    std::string icon;
    ForgeTreeItemKind kind = ForgeTreeItemKind::Page;
    bool enabled = true;
};

struct ForgeVisibleNavItem {
    ForgeNavItem item;
    std::size_t depth = 0;
    bool expanded = false;
    bool has_children = false;
};

class ForgeNavTree {
public:
    static ForgeStatus create(std::vector<ForgeNavItem> items, ForgeNavTree& out);
    ForgeStatus toggle(const std::string& id);
    ForgeStatus set_expanded(const std::string& id, bool expanded);
    [[nodiscard]] std::vector<ForgeVisibleNavItem> visible_items() const;

private:
    std::vector<ForgeNavItem> items_;
    std::unordered_map<std::string, std::size_t> by_id_;
    std::unordered_map<std::string, std::vector<std::size_t>> children_;
    std::vector<std::size_t> roots_;
    std::unordered_set<std::string> expanded_;
};
