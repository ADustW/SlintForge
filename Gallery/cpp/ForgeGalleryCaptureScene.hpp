#pragma once
#include "gallery.h"
#include <filesystem>

slint::ComponentHandle<ForgeCaptureSceneWindow> forge_gallery_create_capture_scene();
void forge_gallery_check_capture_scene(const slint::ComponentHandle<ForgeCaptureSceneWindow>& ui, int stage, const std::filesystem::path& directory);
