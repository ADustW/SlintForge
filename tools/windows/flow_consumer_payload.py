# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""准备独立的 Flow 接入项目，只复制 SDK 接口和示例，不复制共用核心源码。"""
import argparse
import json
from pathlib import Path
import shutil
from release_payload import ROOT, interface_files, interface_text


def prepare(destination):
    destination.mkdir(parents=True, exist_ok=True)
    # 显式隔离消费项目，放在仓库 build 目录时也不能误用上层 Cargo 工作区。
    (destination / "Cargo.toml").write_text(
        '[workspace]\nresolver = "2"\nmembers = ["examples/flow-rust", "sdk/rust", "sdk/runtime-api", "sdk/rust-build"]\n',
        encoding="utf-8")
    shutil.copy2(ROOT / "rust-toolchain.toml", destination / "rust-toolchain.toml")
    for source, relative in interface_files():
        target = destination / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(interface_text(source), encoding="utf-8")
    runtime = destination / "sdk/rust/runtime/slintforge_runtime.dll"
    runtime.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(ROOT / "target/release/slintforge_runtime.dll", runtime)
    for name in ("flow-cpp", "flow-rust"):
        shutil.copytree(ROOT / "examples" / name, destination / "examples" / name)
    ui = destination / "examples/ui"
    ui.mkdir(parents=True, exist_ok=True)
    for name in ("flow.slint", "flow-showcase.slint", "flow-device.svg", "flow-qa.json", "flow-edge-qa.json", "flow-refinement-qa.json", "flow-batch1-qa.json", "flow-batch2-qa.json", "flow-batch3-qa.json", "flow-transform.slint"):
        shutil.copy2(ROOT / "examples/ui" / name, ui / name)
    manifest = destination / "examples/flow-rust/Cargo.toml"
    manifest.write_text(manifest.read_text(encoding="utf-8")
                        .replace('name = "slintforge-flow-example"', 'name = "forgeflow-sdk-consumer"')
                        .replace("version.workspace = true", 'version = "0.1.2"')
                        .replace("edition.workspace = true", 'edition = "2021"')
                        .replace("publish.workspace = true", "publish = false"), encoding="utf-8")
    shutil.copy2(ROOT / "Cargo.lock", destination / "Cargo.lock")
    # 独立项目编译后把已安装 SDK 的 DLL 部署到应用目录。
    build = manifest.parent / "build.rs"
    build.write_text(build.read_text(encoding="utf-8").replace(
        'fn main() {', 'fn main() {\n    let output = std::path::PathBuf::from(std::env::var_os("OUT_DIR").unwrap());\n    slintforge_build::deploy_runtime(output.ancestors().nth(3).unwrap()).unwrap();'), encoding="utf-8")
    assert not (destination / "sdk/common").exists()
    print(json.dumps({"consumer": str(destination), "core_source_copied": False}, ensure_ascii=False))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("destination", type=Path)
    prepare(parser.parse_args().destination)
