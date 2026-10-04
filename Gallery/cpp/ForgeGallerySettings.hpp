#pragma once

#include <string>

struct ForgeGallerySettings {
    std::string language = "zh";
    std::string font_family;
    float font_scale = 1.0f;
    bool pinned = false;
    bool rounded_corners = true;
    bool dark_mode = true;
    std::string placement = "left";
    std::string pane_state = "expanded";
};

ForgeGallerySettings forge_gallery_load_settings();
void forge_gallery_save_settings(const ForgeGallerySettings& settings);
