// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string_view>
bool forge_gallery_is_deferred(std::string_view id);

#include "ForgeSdk.hpp"

#include <string>
#include <string_view>
#include <vector>

enum class ForgeGalleryArea { Main, Footer, Hidden };

struct ForgeGalleryEntry {
    ForgeNavItem item;
    ForgeGalleryArea area = ForgeGalleryArea::Main;
    std::string zh_title;
};

std::vector<ForgeGalleryEntry> forge_gallery_entries();
std::string forge_gallery_title(const ForgeGalleryEntry& entry, std::string_view language);
std::string forge_gallery_category(const std::vector<ForgeGalleryEntry>& entries, const std::string& id, std::string_view language);
