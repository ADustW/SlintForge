# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

"""检查由真实窗口入口导出的 SVG/PNG、选区范围和卡片适配内容。"""
import json
from pathlib import Path
import sys
from xml.etree import ElementTree
from PIL import Image

folder = Path(sys.argv[1])
records = {}
for name, count in (("whole", 6), ("selected", 2)):
    svg = ElementTree.parse(folder / f"{name}.svg").getroot()
    ids = [item.attrib["data-node-id"] for item in svg.iter() if "data-node-id" in item.attrib]
    edges = [item.attrib["data-edge-id"] for item in svg.iter() if "data-edge-id" in item.attrib]
    if len(ids) != count or (name == "selected" and (ids != ["node-1", "node-2"] or len(edges) != 1)):
        raise RuntimeError(f"导出节点或内部连线不符: {name} {ids} {edges}")
    size = (round(float(svg.attrib["width"])), round(float(svg.attrib["height"])))
    with Image.open(folder / f"{name}.png") as image:
        image.load()
        if image.size != size or len(image.convert("RGB").getcolors(1000000) or []) < 20:
            raise RuntimeError(f"PNG 尺寸或内容异常: {name} {image.size}")
        records[name] = {"size": size, "nodes": ids, "edges": edges, "png_bytes": (folder / f"{name}.png").stat().st_size}
    content = "".join(svg.itertext())
    if "已提交 <参数> & 中文" not in content or "设备在线" not in content:
        raise RuntimeError(f"导出没有保留业务卡片内容: {name}")
    if name == "whole" and not any(item.attrib.get("fill") == "#405a72" for item in svg.iter()):
        raise RuntimeError("图片卡片适配内容缺失")
if (folder / "rejected.bmp").exists():
    raise RuntimeError("不支持格式产生了输出文件")
record = {"passed": True, "exports": records, "source": "public SDK and live window export callbacks"}
(folder / "export-result.json").write_text(json.dumps(record, indent=2, ensure_ascii=False), encoding="utf-8")
print("FLOW_EXPORT_OK", json.dumps(record, ensure_ascii=False))
