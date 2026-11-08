# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""核对真实窗口中的背景密度、锚点和图案切换，防止只验证交互步骤。"""
import json
import math
import sys
from pathlib import Path

import numpy as np
from PIL import Image


def clusters(profile):
    profile = profile - profile.min()
    active = np.flatnonzero(profile > profile.max() * 0.1)
    groups = np.split(active, np.where(np.diff(active) > 1)[0] + 1)
    return [(float(np.average(g, weights=profile[g])), len(g)) for g in groups if len(g)]


def check_image(path, variant, view):
    image = np.asarray(Image.open(path).convert("RGB"), dtype=float)
    scale = image.shape[1] / 1100
    x0, y0 = round(100 * scale), round(200 * scale)
    pixels = image[y0:round(450 * scale), x0:round(800 * scale)]
    background = np.median(pixels.reshape(-1, 3), axis=0)
    contrast = np.abs(pixels - background).mean(axis=2)
    result = {"image": path.name, "scale": scale}
    if variant == "plain":
        result["passed"] = bool(not contrast.any())
        return result
    x, y, zoom, gap = view
    step = math.ceil(16 / (zoom * gap)) * zoom * gap
    center_offset = 1 if variant == "dots" else 0.5
    axes = []
    for axis, origin, coordinate, canvas_start in [(0, x0, x, 0), (1, y0, y, 100)]:
        profile = contrast.sum(axis=axis)
        # 剔除裁剪边缘的不完整图案；剩余中心应与视口锚点一致。
        actual = [(c, w) for c, w in clusters(profile) if 5 < c < len(profile) - 5]
        offset = (canvas_start + coordinate % step + center_offset) * scale - origin - 0.5
        expected = [offset + i * step * scale for i in range(-200, 201)
                    if 5 < offset + i * step * scale < len(profile) - 5]
        displacement = max((abs(c - e) for (c, _), e in zip(actual, expected)), default=999)
        widths_ok = all(w <= math.ceil((2 if variant == "dots" else 1) * scale) + 2 for _, w in actual)
        passed = len(actual) == len(expected) and bool(actual) and displacement <= 1.01 and widths_ok
        axes.append({"axis": axis, "expected_count": len(expected), "actual_count": len(actual),
                     "max_displacement_physical_px": displacement, "widths_ok": widths_ok, "passed": passed})
    result.update(axes=axes, passed=all(a["passed"] for a in axes))
    return result


directory = Path(sys.argv[1])
views = [(17.25, -11.5, 1, 20), (-37.4, 12.7, 0.75, 20), (125.5, 24.25, 1.5, 32), (13, -25, 0.1, 20)]
checks = [check_image(directory / f"{variant}-{theme}-{index}.png", variant, view)
          for variant in ["dots", "grid", "plain"] for theme in range(2) for index, view in enumerate(views)]
report = {"passed": all(c["passed"] for c in checks), "checks": checks}
(directory / "background-evidence.json").write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
print(json.dumps({"passed": report["passed"], "count": len(checks), "failed": [c for c in checks if not c["passed"]]}, ensure_ascii=False))
sys.exit(0 if report["passed"] else 1)
