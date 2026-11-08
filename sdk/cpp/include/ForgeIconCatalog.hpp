// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct ForgeIconGlyph {
    std::string name;
    std::uint32_t codepoint = 0;

    [[nodiscard]] std::string glyph_utf8() const;
    [[nodiscard]] std::string codepoint_hex() const;
};

class ForgeIconCatalog {
public:
    ForgeIconCatalog();
    [[nodiscard]] std::size_t size() const { return items_.size(); }
    [[nodiscard]] const ForgeIconGlyph* find(std::string_view name) const;
    [[nodiscard]] std::size_t count(std::string_view query) const;
    [[nodiscard]] std::vector<ForgeIconGlyph> search(
        std::string_view query, std::size_t offset, std::size_t limit) const;

private:
    std::vector<ForgeIconGlyph> items_;
    std::unordered_map<std::string, std::size_t> by_name_;
};
