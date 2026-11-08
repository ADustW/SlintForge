// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

fn main() {
    println!("cargo:rustc-env=SLINT_ENABLE_EXPERIMENTAL_FEATURES=1");
    println!("cargo:rerun-if-changed=runtime/slintforge_runtime.dll");
}
