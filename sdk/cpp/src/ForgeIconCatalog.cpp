#include "ForgeIconCatalog.hpp"
#include "ForgeIconCatalogData.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cstdio>
#include <string>
#include <string_view>

static std::string forge_icon_query_lower(std::string_view text) {
    std::string result(text);
    for (char& ch : result) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    const auto start = result.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return {};
    const auto end = result.find_last_not_of(" \t\r\n");
    return result.substr(start, end - start + 1);
}

ForgeIconCatalog::ForgeIconCatalog() {
    const std::string_view data(forge_icon_catalog_data);
    std::size_t cursor = 0;
    while (cursor < data.size()) {
        const auto end = data.find('\n', cursor);
        const auto line = data.substr(cursor, end == std::string_view::npos ? end : end - cursor);
        const auto tab = line.find('\t');
        if (tab != std::string_view::npos) {
            std::uint32_t codepoint = 0;
            const auto hex = line.substr(tab + 1);
            const auto result = std::from_chars(hex.data(), hex.data() + hex.size(), codepoint, 16);
            if (result.ec == std::errc{} && result.ptr == hex.data() + hex.size()) {
                const auto name = std::string(line.substr(0, tab));
                by_name_.emplace(name, items_.size());
                items_.push_back({name, codepoint});
            }
        }
        if (end == std::string_view::npos) break;
        cursor = end + 1;
    }
}

const ForgeIconGlyph* ForgeIconCatalog::find(std::string_view name) const {
    const auto found = by_name_.find(std::string(name));
    return found == by_name_.end() ? nullptr : &items_[found->second];
}

std::size_t ForgeIconCatalog::count(std::string_view query) const {
    const auto lower = forge_icon_query_lower(query);
    return static_cast<std::size_t>(std::count_if(items_.begin(), items_.end(), [&](const auto& item) {
        return item.name.find(lower) != std::string::npos;
    }));
}

std::vector<ForgeIconGlyph> ForgeIconCatalog::search(
    std::string_view query, std::size_t offset, std::size_t limit) const {
    const auto lower = forge_icon_query_lower(query);
    std::vector<ForgeIconGlyph> page;
    page.reserve(std::min(limit, std::size_t{100}));
    std::size_t matched = 0;
    for (const auto& item : items_) {
        if (item.name.find(lower) == std::string::npos) continue;
        if (matched++ < offset) continue;
        if (page.size() < limit) page.push_back(item);
        if (page.size() == limit) break;
    }
    return page;
}

std::string ForgeIconGlyph::glyph_utf8() const {
    std::string result;
    if (codepoint <= 0x7F) result.push_back(static_cast<char>(codepoint));
    else if (codepoint <= 0x7FF) {
        result.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0xFFFF) {
        result.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        result.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    return result;
}

std::string ForgeIconGlyph::codepoint_hex() const {
    char text[9]{};
    std::snprintf(text, sizeof(text), "%X", codepoint);
    return text;
}
