// Copyright © 2026 陕西禾又科技有限公司
// SPDX-License-Identifier: GPL-3.0-only

// 仅编译 Gallery 的设置存储，避免为后台保存回归生成整套页面。
#[path = "../../../Gallery/rust/src/settings.rs"]
mod settings;
