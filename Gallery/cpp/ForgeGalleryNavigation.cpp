// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeGalleryNavigation.hpp"
#include "ForgeGalleryNavigationData.hpp"

bool forge_gallery_is_deferred(std::string_view id) {
    const std::string_view data(ForgeGalleryComponentStatus);
    std::size_t start = 0;
    while (start < data.size()) {
        const auto end = data.find('\n', start);
        const auto line = data.substr(start, end == std::string_view::npos ? data.size() - start : end - start);
        if (line.starts_with(std::string(id) + "|deferred|")) return true;
        if (end == std::string_view::npos) break;
        start = end + 1;
    }
    return false;
}
#include "ForgeIconCatalog.hpp"

#include <algorithm>
#include <array>
#include <sstream>
#include <stdexcept>
#include <string_view>

static std::array<std::string, 7> forge_gallery_split(const std::string& line, int line_number) {
    std::array<std::string, 7> fields;
    std::size_t start = 0;
    for (std::size_t i = 0; i < 5; ++i) {
        const auto end = line.find('|', start);
        if (end == std::string::npos) throw std::runtime_error("导航目录字段数错误: " + std::to_string(line_number));
        fields[i] = line.substr(start, end - start);
        start = end + 1;
    }
    const auto optional = line.find('|', start);
    if (optional == std::string::npos) fields[5] = line.substr(start);
    else {
        fields[5] = line.substr(start, optional - start);
        if (line.find('|', optional + 1) != std::string::npos)
            throw std::runtime_error("导航目录字段数错误: " + std::to_string(line_number));
        fields[6] = line.substr(optional + 1);
    }
    for (auto& field : fields) {
        const auto first = field.find_first_not_of(" \t\r");
        if (first == std::string::npos) { field.clear(); continue; }
        const auto last = field.find_last_not_of(" \t\r");
        field = field.substr(first, last - first + 1);
    }
    return fields;
}

std::vector<ForgeGalleryEntry> forge_gallery_entries() {
    std::vector<ForgeGalleryEntry> result;
    ForgeIconCatalog icons;
    std::istringstream stream(ForgeGalleryNavigationData);
    std::string line;
    int line_number = 0;
    while (std::getline(stream, line)) {
        ++line_number;
        if (line.empty() || line.front() == '#') continue;
        const auto fields = forge_gallery_split(line, line_number);
        ForgeTreeItemKind kind;
        if (fields[0] == "P") kind = ForgeTreeItemKind::Page;
        else if (fields[0] == "G") kind = ForgeTreeItemKind::Group;
        else if (fields[0] == "H") kind = ForgeTreeItemKind::Header;
        else if (fields[0] == "S") kind = ForgeTreeItemKind::Separator;
        else throw std::runtime_error("导航目录条目类型错误: " + std::to_string(line_number));
        ForgeGalleryArea area;
        if (fields[5] == "main") area = ForgeGalleryArea::Main;
        else if (fields[5] == "footer") area = ForgeGalleryArea::Footer;
        else if (fields[5] == "hidden") area = ForgeGalleryArea::Hidden;
        else throw std::runtime_error("导航目录区域错误: " + std::to_string(line_number));
        std::string glyph;
        if (kind == ForgeTreeItemKind::Page || kind == ForgeTreeItemKind::Group) {
            const auto icon = icons.find(fields[4]);
            if (!icon) throw std::runtime_error("导航目录图标不存在: " + fields[4]);
            glyph = icon->glyph_utf8();
        }
        result.push_back({{fields[1], fields[2], fields[3], glyph, kind, true}, area, fields[6]});
    }
    return result;
}

std::string forge_gallery_title(const ForgeGalleryEntry& entry, std::string_view language) {
    return language == "zh" && !entry.zh_title.empty() ? entry.zh_title : entry.item.title;
}

std::string forge_gallery_category(const std::vector<ForgeGalleryEntry>& entries, const std::string& id, std::string_view language) {
    auto found = std::find_if(entries.begin(), entries.end(), [&](const auto& entry) { return entry.item.id == id; });
    if (found == entries.end()) throw std::runtime_error("未知页面 ID: " + id);
    std::vector<std::string> parts;
    std::string parent_id = found->item.parent_id;
    while (!parent_id.empty()) {
        found = std::find_if(entries.begin(), entries.end(), [&](const auto& entry) { return entry.item.id == parent_id; });
        if (found == entries.end()) throw std::runtime_error("未知父节点: " + parent_id);
        parts.push_back(forge_gallery_title(*found, language));
        parent_id = found->item.parent_id;
    }
    std::reverse(parts.begin(), parts.end());
    if (parts.empty()) return "Gallery";
    std::string category;
    for (const auto& part : parts) {
        if (!category.empty()) category += " / ";
        category += part;
    }
    return category;
}
