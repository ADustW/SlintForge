#!/usr/bin/env python3
# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""验证 SlintForge 双语 API 文档、示例、链接和截图资产。"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import struct
import subprocess
import tempfile
import zlib

from api_docs_model import ROOT, kebab, kind_counts, load_contract


MANIFEST = ROOT / "docs" / "api" / "components.json"
SITE = ROOT / "docs" / "site"
EXPECTED = {"component": 79, "struct": 45, "enum": 21, "global": 12}
ZH_HEADINGS = [
    "## 用途与适用场景", "## 导入方式", "## 属性", "## 回调、公开函数与插槽",
    "## 键盘、鼠标、可访问性、本地化、主题与平台限制", "## 可编译最小示例",
    "## 宿主接入", "## 相关 API",
]
EN_HEADINGS = [
    "## Purpose and usage", "## Import", "## Properties", "## Callbacks, public functions, and slots",
    "## Interaction, accessibility, localization, and platform notes", "## Minimal compilable example",
    "## Host integration", "## Related API",
]


class VerificationError(RuntimeError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise VerificationError(message)


def load_manifest() -> list[dict]:
    require(MANIFEST.is_file(), f"缺少清单: {MANIFEST}")
    value = json.loads(MANIFEST.read_text(encoding="utf-8"))
    require(isinstance(value, list), "components.json 顶层必须是数组")
    return value


def verify_contract(entries: list[dict]) -> tuple[list, dict[str, object]]:
    symbols = load_contract()
    counts = kind_counts(symbols)
    require(counts == EXPECTED, f"公共契约数量不匹配: {counts}，期望 {EXPECTED}")
    components = [symbol for symbol in symbols if symbol.kind == "component"]
    by_name = {symbol.name: symbol for symbol in components}
    names = [entry.get("name") for entry in entries]
    require(len(entries) == 79, f"组件清单数量不是 79: {len(entries)}")
    require(len(names) == len(set(names)), "组件清单存在重复 API 名")
    require(set(names) == set(by_name), f"组件清单与公共入口不一致: 缺少={sorted(set(by_name) - set(names))} 多余={sorted(set(names) - set(by_name))}")
    for entry in entries:
        symbol = by_name[entry["name"]]
        expected_slug = kebab(symbol.name)
        require(entry.get("slug") == expected_slug, f"{symbol.name} slug 不正确")
        require(entry.get("source") == symbol.source, f"{symbol.name} 源码路径不正确")
        require(entry.get("line") == symbol.line, f"{symbol.name} 源码行号不正确")
        require(entry.get("page") == f"/components/{entry['category']}/{expected_slug}", f"{symbol.name} 页面路径不正确")
        require(entry.get("example") == f"examples/api-components/{expected_slug}.slint", f"{symbol.name} 示例路径不正确")
        screenshot = entry.get("screenshot")
        require(isinstance(screenshot, dict) and screenshot.get("route") and screenshot.get("focus") == symbol.name, f"{symbol.name} 截图场景不完整")
    return symbols, by_name


def verify_pages(entries: list[dict], by_name: dict[str, object]) -> None:
    component_route_set = set()
    for entry in entries:
        symbol = by_name[entry["name"]]
        paths = [
            SITE / "components" / entry["category"] / f"{entry['slug']}.md",
            SITE / "en" / "components" / entry["category"] / f"{entry['slug']}.md",
        ]
        for index, path in enumerate(paths):
            require(path.is_file(), f"缺少组件页面: {path}")
            text = path.read_text(encoding="utf-8")
            for heading in (EN_HEADINGS if index else ZH_HEADINGS):
                require(heading in text, f"{path} 缺少章节 {heading}")
            require(f'<ApiScreenshot slug="{entry["slug"]}" name="{entry["name"]}" />' in text, f"{path} 未引用对应截图")
            require(entry["source"] in text, f"{path} 未记录源码位置")
            require(entry["example"] in text, f"{path} 未引用共享示例")
            for member in symbol.members:
                require(f"`{member.name}`" in text, f"{path} 未覆盖公开成员 {member.name}")
        component_route_set.add(entry["page"])
        component_route_set.add("/en" + entry["page"])
    zh_files = set(path.relative_to(SITE).as_posix() for path in (SITE / "components").rglob("*.md"))
    en_files = set(path.relative_to(SITE / "en").as_posix() for path in (SITE / "en" / "components").rglob("*.md"))
    require(zh_files == en_files, f"中英文组件页面集合不对称: zh-only={sorted(zh_files - en_files)} en-only={sorted(en_files - zh_files)}")
    require(len(zh_files) == 79, f"中文组件页数量不是 79: {len(zh_files)}")


def verify_type_indexes(symbols: list) -> None:
    types = [symbol for symbol in symbols if symbol.kind != "component"]
    zh = "\n".join(path.read_text(encoding="utf-8") for path in (SITE / "types").glob("*.md"))
    en = "\n".join(path.read_text(encoding="utf-8") for path in (SITE / "en" / "types").glob("*.md"))
    for symbol in types:
        require(f"`{symbol.name}`" in zh, f"中文类型参考缺少 {symbol.name}")
        require(f"`{symbol.name}`" in en, f"英文类型参考缺少 {symbol.name}")
    for locale in (SITE, SITE / "en"):
        for relative in ("guide/index.md", "guide/integration.md", "guide/migration.md", "sdk/rust.md", "sdk/cpp.md"):
            require((locale / relative).is_file(), f"缺少双语指南/SDK 页面: {locale / relative}")


def verify_links_and_includes() -> None:
    markdown_files = list(SITE.rglob("*.md"))
    route_files = {"/" + path.relative_to(SITE).with_suffix("").as_posix().removesuffix("/index") for path in markdown_files}
    route_files.add("/")
    for path in markdown_files:
        text = path.read_text(encoding="utf-8")
        for include in re.findall(r"^<<<\s+([^\s{]+)", text, re.M):
            target = (path.parent / include).resolve()
            require(target.is_file(), f"{path} 的代码引用不存在: {include}")
        for link in re.findall(r"\[[^\]]*\]\((/[^)#?]*)(?:#[^)]*)?\)", text):
            if "." in Path(link).name:
                continue
            normalized = link.rstrip("/") or "/"
            require(normalized in route_files, f"{path} 的站内链接不存在: {link}")


def find_compiler() -> Path:
    candidates = [
        Path(value) for value in [
            str(ROOT.parent / "slint-sdk-1.18.0" / "bin" / "slint-compiler.exe"),
            shutil.which("slint-compiler") or "",
        ] if value
    ]
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    raise VerificationError("找不到 Slint 1.18.0 编译器；设置 PATH 或放置 ../slint-sdk-1.18.0/bin/slint-compiler.exe")


def verify_examples(entries: list[dict]) -> None:
    compiler = find_compiler()
    version = subprocess.run(
        [str(compiler), "--version"], text=True, encoding="utf-8", errors="replace",
        capture_output=True, check=True,
    ).stdout
    require("1.18.0" in version, f"示例必须使用 Slint 1.18.0，当前为 {version.strip()}")
    style = ROOT / "ui" / "styles" / "slintforge-winui3"
    include_paths = [style.parent, style, style / "upstream" / "fluent", style / "upstream" / "common"]
    (ROOT / "build").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(
        prefix="slintforge-api-examples-", dir=ROOT / "build", ignore_cleanup_errors=True
    ) as directory:
        output = Path(directory) / "example.hpp"
        for index, entry in enumerate(entries, start=1):
            source = ROOT / entry["example"]
            require(source.is_file(), f"缺少最小示例: {source}")
            command = [str(compiler), "--style", "slintforge-winui3"]
            for include in include_paths:
                command.extend(["-I", str(include.resolve())])
            command.extend(["-f", "cpp", "-o", str(output), str(source)])
            result = subprocess.run(
                command, cwd=ROOT, text=True, encoding="utf-8", errors="replace",
                capture_output=True,
                env={**__import__("os").environ, "SLINT_ENABLE_EXPERIMENTAL_FEATURES": "1"},
            )
            if result.returncode:
                raise VerificationError(f"示例编译失败 ({index}/79) {source}:\n{result.stderr}")


def png_pixels(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    require(data.startswith(b"\x89PNG\r\n\x1a\n"), f"不是 PNG: {path}")
    offset = 8
    width = height = color_type = bit_depth = 0
    payload = bytearray()
    while offset < len(data):
        length = struct.unpack(">I", data[offset:offset + 4])[0]
        kind = data[offset + 4:offset + 8]
        chunk = data[offset + 8:offset + 8 + length]
        offset += 12 + length
        if kind == b"IHDR":
            width, height, bit_depth, color_type = struct.unpack(">IIBB", chunk[:10])
        elif kind == b"IDAT":
            payload.extend(chunk)
        elif kind == b"IEND":
            break
    require(bit_depth == 8 and color_type in {2, 6}, f"截图 PNG 必须是 8 位 RGB/RGBA: {path}")
    channels = 4 if color_type == 6 else 3
    raw = zlib.decompress(bytes(payload))
    stride = width * channels
    require(len(raw) == height * (stride + 1), f"PNG 扫描线长度异常: {path}")
    filters = raw[::stride + 1]
    require(all(value <= 4 for value in filters), f"PNG 扫描线过滤器异常: {path}")
    # zlib 解压和 SHA-256 均由原生代码执行；过滤后的扫描线仍足以证明像素载荷
    # 非空并检测逐文件重复，避免对 158 张大图做数亿次 Python 像素循环。
    return width, height, raw


def verify_images(entries: list[dict]) -> None:
    hashes: dict[str, Path] = {}
    for locale in ("zh", "en"):
        directory = SITE / "public" / "images" / "components" / locale
        files = list(directory.glob("*.png")) if directory.is_dir() else []
        require(len(files) == 79, f"{locale} 截图数量不是 79: {len(files)}")
        for entry in entries:
            path = directory / f"{entry['slug']}.png"
            require(path.is_file() and path.stat().st_size > 4096, f"截图缺失或过小: {path}")
            width, height, pixels = png_pixels(path)
            require((width, height) == (1280, 900), f"截图尺寸必须为 1280x900: {path} 是 {width}x{height}")
            sample = pixels[::max(1, len(pixels) // 8192)]
            require(len(set(sample)) > 8, f"截图像素近似为空: {path}")
            digest = hashlib.sha256(pixels).hexdigest()
            require(digest not in hashes, f"截图像素重复: {path} 与 {hashes.get(digest)}")
            hashes[digest] = path


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--skip-images", action="store_true", help="仅用于截图生成前的中间检查")
    parser.add_argument("--skip-examples", action="store_true", help="仅用于快速内容检查")
    args = parser.parse_args()
    entries = load_manifest()
    symbols, by_name = verify_contract(entries)
    verify_pages(entries, by_name)
    verify_type_indexes(symbols)
    verify_links_and_includes()
    if not args.skip_examples:
        verify_examples(entries)
    if not args.skip_images:
        verify_images(entries)
    print(f"API_DOCS_OK components={len(entries)} structs=45 enums=21 globals=12 examples={'skipped' if args.skip_examples else 79} images={'skipped' if args.skip_images else 158}")


if __name__ == "__main__":
    try:
        main()
    except VerificationError as error:
        raise SystemExit(f"API_DOCS_ERROR: {error}")
