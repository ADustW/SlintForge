# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

"""生成连线窗口回归场景，并检查线身是否连续接近箭头。"""
import argparse
import json
from pathlib import Path


def forge_flow_routing_scenarios(root):
    def node(id, x, y, **fields):
        return dict(id=id, x=x, y=y, width=180, height=130, title=id,
                    data={"text": "连接检查"}, **fields)

    def edge(id, source, target, width=8, style=2, **fields):
        return dict(id=id, source_node=source, target_node=target, source_port="out",
                    target_port="in", stroke_color="#DD2244", stroke_width=width,
                    style=style, **fields)

    steps = [dict(kind="size", width=1100, height=760), dict(kind="theme", dark=False)]

    def scene(name, nodes, edges, zoom=1, dark=False, options=None):
        steps.extend([dict(kind="theme", dark=dark),
                      dict(kind="edit", op="load", text=json.dumps(dict(version=2, mode="graph", nodes=nodes, edges=edges, options=options or {}), ensure_ascii=False)),
                      dict(kind="view", x=40, y=20, zoom=zoom), dict(kind="snapshot", name=name)])

    long_nodes = [node("A", 20, 80), node("B", 800, 80), node("C", 20, 320), node("D", 800, 320)]
    long_edges = [edge("thick", "A", "B", style=0), edge("thin", "C", "D", width=2, style=1, marker_end="open-arrow")]
    scene("routing-long-light", long_nodes, long_edges)
    steps.extend([dict(kind="click", x=570, y=265),
                  dict(kind="query-expect", request=dict(op="query", target="selection"), path="/result/edges/0", value="thick"),
                  dict(kind="snapshot", name="routing-long-selected")])
    scene("routing-long-dark", long_nodes, long_edges, dark=True)
    scene("routing-long-zoom", long_nodes, long_edges, zoom=.75)

    for incoming in (False, True):
        nodes = [node("center", 700 if incoming else 0, 140)]
        nodes.extend(node(f"branch-{i}", 0 if incoming else 700, i*220) for i in range(3))
        edges = [edge(f"fan-{i}", f"branch-{i}" if incoming else "center", "center" if incoming else f"branch-{i}") for i in range(3)]
        scene("routing-fan-in" if incoming else "routing-fan-out", nodes, edges)
        # 第二分支的竖线位于端口向外 48 像素处，点击实际窗口再读取选择状态。
        steps.extend([dict(kind="click", x=700-48+40 if incoming else 180+48+40, y=365),
                      dict(kind="query-expect", request=dict(op="query", target="selection"), path="/result/edges/0", value="fan-1")])
        for e in edges:
            e["style"] = 3
        scene("routing-fan-in-rounded" if incoming else "routing-fan-out-rounded", nodes, edges, dark=True)

    scene("routing-close-parallel", [node("A",80,40), node("B",280,40)],
          [edge("close-1","A","B",width=2), edge("close-2","A","B",width=2)],
          zoom=2, options={"allow_parallel_connections":True})
    dense = json.loads((root/"tests/fixtures/flow-routing-dense.json").read_text(encoding="utf-8"))
    scene("routing-dense", dense["nodes"], dense["edges"], zoom=.8)
    steps.extend([dict(kind="edit", op="demo", number=1), dict(kind="view", x=40, y=40, zoom=1),
                  dict(kind="snapshot", name="routing-cross-row"), dict(kind="theme", dark=False),
                  dict(kind="size", width=760, height=600), dict(kind="view", x=30, y=10, zoom=.7),
                  dict(kind="snapshot", name="routing-cross-row-narrow")])
    steps.append(dict(kind="finish"))
    return steps


def forge_flow_check_routing_pixels(folder, scale):
    from PIL import Image
    import numpy as np
    checks = []
    for name in ("routing-long-light", "routing-long-dark", "routing-long-selected", "routing-long-zoom"):
        pixels = np.asarray(Image.open(folder/(name+".png")).convert("RGB"))
        zoom = .75 if name.endswith("zoom") else 1
        for label, y in (("thick", 145), ("thin", 385)):
            # 两端避开端口覆盖区域，其余线身每一列都必须可见。
            left, right = round((200*zoom+40+12)*scale), round((800*zoom+40-12)*scale)
            center = round((y*zoom+120)*scale)
            radius = max(2, round(5*zoom*scale))
            crop = pixels[center-radius:center+radius+1, left:right]
            ink = (crop[:,:,0]>140)&(crop[:,:,1]<100)&(crop[:,:,2]<140)
            missing = int((~ink.any(axis=0)).sum())
            checks.append(dict(image=name, edge=label, missing_columns=missing, passed=missing==0))
    result = dict(passed=all(c["passed"] for c in checks), scale=scale, checks=checks)
    (folder/"routing-pixels.json").write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding="utf-8")
    print(json.dumps(result,ensure_ascii=False))
    return result["passed"]


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--generate", type=Path)
    parser.add_argument("--check", type=Path)
    parser.add_argument("--scale", type=float, default=1)
    parser.add_argument("--baseline", action="store_true")
    args = parser.parse_args()
    if args.generate:
        steps = forge_flow_routing_scenarios(Path(__file__).resolve().parents[2])
        if args.baseline:
            steps = [step for step in steps if step["kind"] not in ("click", "query-expect")]
        args.generate.parent.mkdir(parents=True,exist_ok=True)
        args.generate.write_text(json.dumps(steps,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    if args.check:
        raise SystemExit(0 if forge_flow_check_routing_pixels(args.check,args.scale) else 1)
