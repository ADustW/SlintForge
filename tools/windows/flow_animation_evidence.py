"""比较真实窗口连续截图，分别确认连线流动和禁用动画后的静止画面。"""
import json
from pathlib import Path
import sys

from PIL import Image, ImageChops

folder = Path(sys.argv[1])


def difference(name):
    with Image.open(folder / f"{name}-a.png") as a, Image.open(folder / f"{name}-b.png") as b:
        if a.size != b.size:
            raise RuntimeError("动画截图尺寸不一致")
        diff = ImageChops.difference(a.convert("RGB"), b.convert("RGB"))
        box = diff.getbbox()
        count = sum(1 for value in diff.getdata() if any(value))
        return {"changed_pixels": count, "bounds": box, "size": a.size}


moving = difference("animation")
paused = difference("animation-paused")
scale = moving["size"][0] / 1100
if moving["changed_pixels"] < 4 or moving["changed_pixels"] > 2000 * scale * scale:
    raise RuntimeError(f"流动效果未出现或画面变化超出连线范围: {moving}")
if paused["changed_pixels"] != 0:
    raise RuntimeError(f"关闭动画后画面仍在改变: {paused}")
record = {"passed": True, "moving": moving, "paused": paused, "source": "Slint window captures"}
(folder / "animation-result.json").write_text(json.dumps(record, indent=2), encoding="utf-8")
print("FLOW_ANIMATION_OK", json.dumps(record))
