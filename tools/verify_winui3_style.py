# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

"""核对主题依赖、未改动的标准控件来源和旧图表接口清理。"""
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
STYLE = ROOT / "ui/styles/slintforge-winui3"


def main():
    manifest = json.loads((STYLE / "upstream-sha256.json").read_text(encoding="utf-8"))
    for relative, expected in manifest.items():
        assert hashlib.sha256((STYLE / relative).read_bytes()).hexdigest() == expected, relative
    errors = []
    for folder in ("ui", "sdk", "Gallery", "examples", "tests"):
        for path in (ROOT / folder).rglob("*"):
            if path.suffix not in (".slint", ".rs", ".cpp", ".hpp", ".h") or "upstream" in path.parts:
                continue
            text = path.read_text(encoding="utf-8-sig")
            if re.search(r"\bForge(?:Chart|BarChart|LineChart|PieChart|PolarAreaChart|BubbleChart|ScatterChart|RadarChart)\w*\b|bind_chart_services", text):
                errors.append(str(path.relative_to(ROOT)))
            if path.suffix == ".slint" and re.search(r"opacity:\s*root\.enabled\s*\?", text):
                errors.append("整块禁用透明度：" + str(path.relative_to(ROOT)))
    assert not errors, errors
    assert "ABI_VERSION: u32 = 2" in (ROOT / "sdk/runtime-api/src/lib.rs").read_text(encoding="utf-8")
    models = (ROOT / "sdk/runtime-api/src/models.rs").read_text(encoding="utf-8")
    for name, value in (("RESERVED21", 21), ("RESERVED22", 22), ("RESERVED23", 23), ("GRIDPREPARE", 24)):
        assert f"OP_{name}: u32 = {value};" in models
    print(f"WINUI3_STYLE_OK upstream_files={len(manifest)} abi=2 retired_ops=21,22,23")


if __name__ == "__main__":
    main()
