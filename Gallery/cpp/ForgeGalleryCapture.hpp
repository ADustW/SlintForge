// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "gallery.h"

void forge_gallery_connect_capture(const slint::ComponentHandle<GalleryWindow>& ui);
void forge_gallery_start_capture_verification(const slint::ComponentHandle<GalleryWindow>& ui, slint::Timer& timer, bool preview = false);
