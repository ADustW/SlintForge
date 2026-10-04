#pragma once
#include "gallery.h"

void forge_gallery_connect_capture(const slint::ComponentHandle<GalleryWindow>& ui);
void forge_gallery_start_capture_verification(const slint::ComponentHandle<GalleryWindow>& ui, slint::Timer& timer, bool preview = false);
