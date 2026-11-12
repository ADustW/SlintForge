// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <slint.h>

// 窗口句柄创建后调用；Windows 11 DWM 可应用圆角或直角偏好。
bool forge_apply_window_corners(const slint::Window& window, bool rounded);
