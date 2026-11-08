// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct ForgeStaggeredInput {
    std::string id;
    float height = 0.0f;
};

struct ForgeStaggeredPlacement {
    std::string id;
    std::size_t column = 0;
    float top = 0.0f;
};

struct ForgeStaggeredLayout {
    std::vector<ForgeStaggeredPlacement> placements;
    float content_height = 0.0f;

    static ForgeStaggeredLayout arrange(
        const std::vector<ForgeStaggeredInput>& items,
        std::size_t columns,
        float gap,
        float padding);
};
