# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""生成可迁移源码 SDK、源码指纹和 ZIP 清单，不复制构建缓存。"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import zipfile

DIRECTORIES = {"ui", "sdk", "Gallery", "examples", "tests", "tools", "third_party", "docs"}
ROOT_FILES = {"Cargo.toml", "Cargo.lock", "CMakeLists.txt", "rust-toolchain.toml", "README.md", ".gitignore"}
IGNORED = {".git", "build", "target", "dist", "__pycache__", ".cache", ".pytest_cache"}
VERIFICATION_SCRIPTS = {"tools/windows/Common.ps1", "tools/windows/Verify.ps1", "tools/windows/source_snapshot.py"}


def files(root):
    for directory, folders, names in os.walk(root):
        folders[:] = sorted(p for p in folders if p not in IGNORED and not p.startswith(("build-", "cmake-build-"))
                            and (Path(directory) != root or p in DIRECTORIES))
        for name in sorted(names):
            path = Path(directory) / name
            if Path(directory) == root and name not in ROOT_FILES:
                continue
            if path.suffix.lower() in {".pyc", ".log", ".tmp", ".exe", ".dll", ".lib", ".pdb", ".obj"}:
                continue
            yield path


def records(root):
    return {p.relative_to(root).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in files(root)}


def fingerprint(root, verification=False):
    # 文档可在验收完成后补充实际结果；源码、脚本、契约、资源和锁文件必须保持一致。
    data = {key: value for key, value in records(root).items() if not key.endswith(".md")}
    if verification:
        # 打包与安装脚本由发布阶段独立验证，应用验收仅绑定构建和测试的输入。
        data = {key: value for key, value in data.items()
                if not key.startswith("tools/windows/") or key in VERIFICATION_SCRIPTS}
    return hashlib.sha256(json.dumps(data, sort_keys=True, ensure_ascii=False).encode()).hexdigest()


def archive(directory, destination):
    directory, destination = Path(directory), Path(destination)
    with zipfile.ZipFile(destination, "w", zipfile.ZIP_DEFLATED, compresslevel=6) as output:
        for path in sorted(directory.rglob("*")):
            if path.is_file():
                output.write(path, path.relative_to(directory.parent))
    with zipfile.ZipFile(destination) as output:
        if output.testzip() is not None:
            raise RuntimeError(f"ZIP 校验失败：{destination}")
    print(f"ZIP_OK {destination.name} sha256={hashlib.sha256(destination.read_bytes()).hexdigest()}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=str(Path(__file__).resolve().parents[2]))
    parser.add_argument("--fingerprint", action="store_true")
    parser.add_argument("--verification-fingerprint", action="store_true")
    parser.add_argument("--copy-to")
    parser.add_argument("--zip", nargs=2, metavar=("DIRECTORY", "OUTPUT"))
    args = parser.parse_args()
    root = Path(args.root).resolve()
    if args.fingerprint:
        print(fingerprint(root))
    if args.verification_fingerprint:
        print(fingerprint(root, verification=True))
    if args.copy_to:
        destination = Path(args.copy_to)
        destination.mkdir(parents=True, exist_ok=False)
        for path in files(root):
            target = destination / path.relative_to(root)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, target)
        receipt = {"fingerprint": fingerprint(root), "files": records(root)}
        try:
            receipt["git_commit"] = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=root, text=True).strip()
            receipt["git_dirty"] = bool(subprocess.check_output(["git", "status", "--porcelain"], cwd=root))
        except subprocess.CalledProcessError:
            receipt["git_commit"] = None
        (destination / "source-sha256.json").write_text(json.dumps(receipt, ensure_ascii=False, indent=2), encoding="utf-8")
        assert records(destination) == receipt["files"]
        print(f"SOURCE_SNAPSHOT_OK files={len(receipt['files'])} fingerprint={receipt['fingerprint']}")
    if args.zip:
        archive(*args.zip)
