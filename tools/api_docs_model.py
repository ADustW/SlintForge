#!/usr/bin/env python3
# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""SlintForge 公共 Slint 契约提取器，供文档生成和一致性检查共用。"""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[1]
ENTRY = ROOT / "ui" / "slintforge.slint"


@dataclass(frozen=True)
class Member:
    kind: str
    name: str
    direction: str = ""
    value_type: str = ""
    signature: str = ""
    default: str = ""


@dataclass
class Symbol:
    name: str
    source: str
    kind: str
    line: int
    inherits: str = ""
    body: str = ""
    members: list[Member] = field(default_factory=list)


def kebab(name: str) -> str:
    value = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1-\2", name)
    value = re.sub(r"([a-z0-9])([A-Z])", r"\1-\2", value)
    return value.replace("_", "-").lower()


def _matching_brace(text: str, opening: int) -> int:
    depth = 0
    quote = ""
    escaped = False
    for index in range(opening, len(text)):
        char = text[index]
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = ""
            continue
        if char in {'"', "'"}:
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return index
    raise ValueError(f"未闭合的大括号，偏移 {opening}")


def _is_top_level(body: str, offset: int) -> bool:
    depth = 0
    quote = ""
    escaped = False
    for char in body[:offset]:
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = ""
            continue
        if char in {'"', "'"}:
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
    return depth == 0


def _clean(value: str) -> str:
    return re.sub(r"\s+", " ", value.strip())


def _members(body: str) -> list[Member]:
    found: list[tuple[int, Member]] = []
    property_pattern = re.compile(
        r"(?m)^\s*(in-out|in|out)\s+property<([^>]+)>\s+([A-Za-z_][\w-]*)"
        r"(?:\s*(?::|<=>)\s*([^;]+))?\s*;"
    )
    callback_pattern = re.compile(
        r"(?m)^\s*callback\s+([A-Za-z_][\w-]*)\s*\(([^;]*)\)"
        r"(?:\s*->\s*([^;]+))?\s*;"
    )
    function_pattern = re.compile(
        r"(?m)^\s*public\s+function\s+([A-Za-z_][\w-]*)\s*\(([^)]*)\)"
        r"(?:\s*->\s*([^\s{]+))?\s*\{"
    )
    for match in property_pattern.finditer(body):
        if _is_top_level(body, match.start()):
            found.append(
                (
                    match.start(),
                    Member(
                        kind="property",
                        name=match.group(3),
                        direction=match.group(1),
                        value_type=_clean(match.group(2)),
                        default=_clean(match.group(4) or ""),
                    ),
                )
            )
    for match in callback_pattern.finditer(body):
        if _is_top_level(body, match.start()):
            result = _clean(match.group(3) or "")
            args = _clean(match.group(2))
            signature = f"({args})" + (f" -> {result}" if result else "")
            found.append((match.start(), Member("callback", match.group(1), signature=signature)))
    for match in function_pattern.finditer(body):
        if _is_top_level(body, match.start()):
            result = _clean(match.group(3) or "")
            args = _clean(match.group(2))
            signature = f"({args})" + (f" -> {result}" if result else "")
            found.append((match.start(), Member("function", match.group(1), signature=signature)))
    if "@children" in body:
        found.append((body.index("@children"), Member("slot", "default", signature="@children")))
    return [member for _, member in sorted(found, key=lambda item: item[0])]


def _locate_symbol(name: str, source: str) -> Symbol:
    path = ROOT / "ui" / source
    text = path.read_text(encoding="utf-8")
    patterns = [
        ("component", re.compile(rf"\bexport\s+component\s+{re.escape(name)}\s+inherits\s+([\w-]+)\s*\{{")),
        ("struct", re.compile(rf"\bexport\s+struct\s+{re.escape(name)}\s*\{{")),
        ("enum", re.compile(rf"\bexport\s+enum\s+{re.escape(name)}\s*\{{")),
        ("global", re.compile(rf"\bexport\s+global\s+{re.escape(name)}\s*\{{")),
    ]
    for kind, pattern in patterns:
        match = pattern.search(text)
        if not match:
            continue
        opening = text.index("{", match.start())
        closing = _matching_brace(text, opening)
        body = text[opening + 1 : closing]
        line = text.count("\n", 0, match.start()) + 1
        inherits = match.group(1) if kind == "component" else ""
        members = _members(body) if kind in {"component", "global"} else []
        return Symbol(name, f"ui/{source}", kind, line, inherits, body, members)
    raise ValueError(f"公共入口导出了 {name}，但 {path} 中找不到声明")


def load_contract() -> list[Symbol]:
    text = ENTRY.read_text(encoding="utf-8")
    result: list[Symbol] = []
    for match in re.finditer(r"export\s*\{(.*?)\}\s*from\s*\"([^\"]+)\"\s*;", text, re.S):
        source = match.group(2)
        names = [name.strip() for name in match.group(1).replace("\n", " ").split(",")]
        for name in filter(None, names):
            result.append(_locate_symbol(name, source))
    duplicates = {symbol.name for symbol in result if sum(item.name == symbol.name for item in result) > 1}
    if duplicates:
        raise ValueError(f"公共入口存在重复导出: {sorted(duplicates)}")
    return result


def category(symbol: Symbol) -> str:
    part = Path(symbol.source).parts[1]
    return {"charts": "plot", "typography": "theme"}.get(part, part)


def service_dependencies(symbol: Symbol) -> list[str]:
    text = (ROOT / symbol.source).read_text(encoding="utf-8")
    names = sorted(set(re.findall(r"\b(Forge[A-Za-z0-9]+Services?|ForgeCaptureService)\b", text)))
    return [name for name in names if name.endswith("Service") or name.endswith("Services")]


def capabilities(symbol: Symbol) -> dict[str, bool]:
    text = (ROOT / symbol.source).read_text(encoding="utf-8")
    return {
        "keyboard": any(token in text for token in ("KeyEvent", "FocusScope", "focus()")),
        "pointer": any(token in text for token in ("TouchArea", "PointerEvent", "MouseCursor")),
        "accessible": "accessible-" in text,
        "localized": "@tr(" in text or "language" in text,
        "themed": any(token in text for token in ("ForgeTheme", "ForgeTypography", "Palette.")),
    }


def kind_counts(symbols: list[Symbol]) -> dict[str, int]:
    return {kind: sum(symbol.kind == kind for symbol in symbols) for kind in ("component", "struct", "enum", "global")}
