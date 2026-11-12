// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeStaggeredLayout.hpp"

#include <algorithm>
#include <cmath>

static float forge_staggered_nonnegative(float value) {
    return std::isfinite(value) ? std::max(value, 0.0f) : 0.0f;
}

ForgeStaggeredLayout ForgeStaggeredLayout::arrange(
    const std::vector<ForgeStaggeredInput>& items,
    std::size_t columns,
    float gap,
    float padding) {
    columns = std::min(std::max<std::size_t>(columns, 1), std::max<std::size_t>(items.size(), 1));
    gap = forge_staggered_nonnegative(gap);
    padding = forge_staggered_nonnegative(padding);
    std::vector<float> heights(columns, padding);
    ForgeStaggeredLayout result;
    result.placements.reserve(items.size());
    for (const auto& item : items) {
        const auto cursor = std::min_element(heights.begin(), heights.end());
        const auto column = static_cast<std::size_t>(cursor - heights.begin());
        result.placements.push_back({item.id, column, *cursor});
        *cursor += forge_staggered_nonnegative(item.height) + gap;
    }
    result.content_height = items.empty()
        ? padding * 2.0f
        : *std::max_element(heights.begin(), heights.end()) - gap + padding;
    return result;
}
