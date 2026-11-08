# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""从固定版本的上游码位表生成 Rust/C++ 共用目录。"""

import json
import sys
from pathlib import Path

root = Path(__file__).resolve().parent
source = root.parent / "assets" / "fonts" / "FluentSystemIcons-Regular.json"
target = root / "catalog.tsv"
mapping = json.loads(source.read_text(encoding="utf-8"))
lines = []
for name, codepoint in sorted(mapping.items()):
    if not name.startswith("ic_fluent_") or not isinstance(codepoint, int):
        raise SystemExit(f"无效码位项：{name}")
    lines.append(f"{name.removeprefix('ic_fluent_')}\t{codepoint:X}")
content = "\n".join(lines) + "\n"
if "--check" in sys.argv:
    if not target.exists() or target.read_text(encoding="utf-8") != content:
        raise SystemExit("图标目录未同步")
else:
    target.write_text(content, encoding="utf-8")
print(f"ICON_CATALOG_OK {len(lines)}")
