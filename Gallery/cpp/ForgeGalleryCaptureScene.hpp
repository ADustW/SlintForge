// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "gallery.h"
#include <filesystem>

slint::ComponentHandle<ForgeCaptureSceneWindow> forge_gallery_create_capture_scene();
void forge_gallery_check_capture_scene(const slint::ComponentHandle<ForgeCaptureSceneWindow>& ui, int stage, const std::filesystem::path& directory);
