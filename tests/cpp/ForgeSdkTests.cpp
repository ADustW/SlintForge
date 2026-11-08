// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeSdk.hpp"
#include "ForgeIconCatalog.hpp"
#include "ForgeStaggeredLayout.hpp"

#include <cassert>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>

int main() {
    ForgeIconCatalog icons;
    assert(icons.size() == 9767);
    const auto* home = icons.find("home_20_regular");
    assert(home && home->codepoint == 0xF480 && home->glyph_utf8() == "\xEF\x92\x80");
    assert(icons.count("home") > 8);
    assert(icons.search("home", 0, 8).size() == 8);
    assert(icons.search("home", 8, 8).front().name != icons.search("home", 0, 8).front().name);
    ForgeRouter router;
    assert(ForgeRouter::create({"home", "detail"}, "home", router).ok());
    assert(!router.go("home").changed);
    assert(router.go("missing").code == ForgeErrorCode::UnknownRoute);
    assert(router.current().id == "home");
    assert(router.go("detail", {{"id", "7"}}).changed);
    assert(!router.go("detail", {{"id", "7"}}).changed);
    assert(router.history_len() == 1);
    assert(router.back().ok() && router.current().id == "home");
    assert(router.back().code == ForgeErrorCode::NoHistory);
    assert(router.go("detail", {{"id", "8"}}).ok());
    assert(router.replace("home").ok());
    assert(router.history_len() == 1 && router.current().id == "home");
    assert(router.go("").code == ForgeErrorCode::EmptyId);
    assert(router.current().id == "home");
    ForgeRouter duplicate_router;
    assert(ForgeRouter::create({"home", "home"}, "home", duplicate_router).code == ForgeErrorCode::DuplicateId);

    ForgeNavTree tree;
    assert(ForgeNavTree::create({
        {"root", "", "Root", "R", ForgeTreeItemKind::Group},
        {"sub", "root", "Sub", "S", ForgeTreeItemKind::Group},
        {"leaf", "sub", "Leaf", "L", ForgeTreeItemKind::Page},
    }, tree).ok());
    assert(tree.visible_items().size() == 1);
    assert(tree.toggle("root").ok() && tree.visible_items().size() == 2);
    assert(tree.toggle("sub").ok() && tree.visible_items()[2].depth == 2);
    assert(ForgeNavTree::create({{"x", "x", "X", "", ForgeTreeItemKind::Group}}, tree).code == ForgeErrorCode::Cycle);
    assert(tree.visible_items().size() == 3);
    // 深层导航必须迭代展开，且不能因输入顺序改变层级。
    constexpr std::size_t forge_review_depth = 20000;
    std::vector<ForgeNavItem> forge_review_items;
    for (std::size_t i = 0; i < forge_review_depth; ++i)
        forge_review_items.push_back({"node-" + std::to_string(i), i == 0 ? "" : "node-" + std::to_string(i - 1), "节点", "", ForgeTreeItemKind::Group});
    const auto forge_review_start = std::chrono::steady_clock::now();
    ForgeNavTree forge_review_tree;
    assert(ForgeNavTree::create(forge_review_items, forge_review_tree).ok());
    const auto forge_review_creation = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < forge_review_depth; ++i)
        assert(forge_review_tree.set_expanded("node-" + std::to_string(i), true).ok());
    const auto forge_review_flatten = std::chrono::steady_clock::now();
    const auto forge_review_visible = forge_review_tree.visible_items();
    std::cout << "NAVIGATION_REVIEW nodes=" << forge_review_depth
        << " creation_ms=" << std::chrono::duration<double, std::milli>(forge_review_creation - forge_review_start).count()
        << " flatten_ms=" << std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - forge_review_flatten).count() << '\n';
    assert(forge_review_visible.size() == forge_review_depth && forge_review_visible.back().depth == forge_review_depth - 1);
    assert(forge_review_visible.back().item.id == "node-19999");
    assert(forge_review_tree.set_expanded("node-1", false).ok() && forge_review_tree.visible_items().size() == 2);
    std::reverse(forge_review_items.begin(), forge_review_items.end());
    assert(ForgeNavTree::create(std::move(forge_review_items), forge_review_tree).ok());
    assert(forge_review_tree.visible_items().front().item.id == "node-0");
    assert(ForgeNavTree::create({
        {"root", "", "根", "", ForgeTreeItemKind::Group},
        {"a", "b", "A", "", ForgeTreeItemKind::Group},
        {"b", "a", "B", "", ForgeTreeItemKind::Group},
    }, forge_review_tree).code == ForgeErrorCode::Cycle);
    assert(forge_review_tree.visible_items().front().item.id == "node-0");
    const auto staggered = ForgeStaggeredLayout::arrange({
        {"card-0", 100.0f}, {"card-1", 60.0f}, {"card-2", 80.0f},
        {"card-3", 50.0f}, {"card-4", 70.0f},
    }, 2, 10.0f, 12.0f);
    assert(staggered.placements.size() == 5);
    assert(staggered.placements[0].id == "card-0" && staggered.placements[0].column == 0 && staggered.placements[0].top == 12.0f);
    assert(staggered.placements[1].column == 1 && staggered.placements[1].top == 12.0f);
    assert(staggered.placements[2].column == 1 && staggered.placements[2].top == 82.0f);
    assert(staggered.placements[3].column == 0 && staggered.placements[3].top == 122.0f);
    assert(staggered.placements[4].column == 1 && staggered.placements[4].top == 172.0f);
    assert(staggered.content_height == 254.0f);
    const auto bounded = ForgeStaggeredLayout::arrange({
        {"bad", std::numeric_limits<float>::quiet_NaN()},
        {"good", 30.0f},
        {"infinite", std::numeric_limits<float>::infinity()},
    }, std::numeric_limits<std::size_t>::max(), std::numeric_limits<float>::quiet_NaN(), -5.0f);
    assert(bounded.placements.size() == 3);
    assert(bounded.placements[0].column == 0 && bounded.placements[1].column == 0 && bounded.placements[2].column == 1);
    assert(bounded.content_height == 30.0f);
    assert(ForgeStaggeredLayout::arrange({}, std::numeric_limits<std::size_t>::max(), 12.0f, 5.0f).content_height == 10.0f);
    assert(ForgeNavTree::create({{"child", "absent", "C", "", ForgeTreeItemKind::Page}}, tree).code == ForgeErrorCode::UnknownParent);
    assert(ForgeNavTree::create({
        {"parent", "", "P", "", ForgeTreeItemKind::Page},
        {"child", "parent", "C", "", ForgeTreeItemKind::Page},
    }, tree).code == ForgeErrorCode::InvalidParent);
    assert(tree.toggle("leaf").code == ForgeErrorCode::NotGroup);
    assert(tree.visible_items().size() == 3);
    return 0;
}
