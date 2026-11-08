# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""检查真实窗口交互基准；阈值针对本机 Windows 软件渲染验收，不能替代跨平台测量。"""
import argparse
import csv
import json
import math
from pathlib import Path


def measurements(path):
    groups = {}
    with path.open(encoding="utf-8", newline="") as stream:
        for row in csv.DictReader(stream):
            key = (int(row["points"]), row["operation"])
            value = float(row["total_ms"])
            assert math.isfinite(value) and value > 0, f"无效耗时：{path} {row}"
            groups.setdefault(key, []).append((int(row["frame"]), value))
    assert set(groups) == {(points, operation) for points in (12, 2000) for operation in ("hover", "drag", "wheel", "live")}, f"缺少工作负载：{path}"
    result = {}
    for key, values in groups.items():
        assert sorted(frame for frame, _ in values) == list(range(24)), f"帧序列缺失或重复：{path} {key}"
        ordered = sorted(value for _, value in values)
        result[key] = {"median_ms": ordered[12], "p95_ms": ordered[22]}
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--qa-root", type=Path, default=Path("build/plot-performance-qa"))
    parser.add_argument("--hover-p95-ms", type=float, default=15.)
    parser.add_argument("--small-p95-ms", type=float, default=35.)
    parser.add_argument("--dense-p95-ms", type=float, default=50.)
    args = parser.parse_args()
    report = []
    for host, configuration in (("rust", "debug"), ("cpp", "release")):
        before = measurements(args.qa_root / f"before-{host}-{configuration}" / "interaction-performance.csv")
        after = measurements(args.qa_root / f"final-{host}-{configuration}" / "interaction-performance.csv")
        for (points, operation), measured in after.items():
            maximum = args.hover_p95_ms if operation == "hover" else args.small_p95_ms if points == 12 else args.dense_p95_ms
            assert measured["p95_ms"] <= maximum, f"帧预算超限：{host} {points} {operation} {measured}"
            baseline = before[(points, operation)]["median_ms"]
            improvement = 100. * (1. - measured["median_ms"] / baseline)
            if host == "rust":
                assert improvement >= 80., f"Rust 优化收益不足：{points} {operation} {improvement:.1f}%"
            elif points == 2000:
                assert improvement >= 50., f"密集数据优化收益不足：{points} {operation} {improvement:.1f}%"
            else:
                assert measured["median_ms"] <= baseline * 1.25, f"C++ 小数据明显回退：{operation}"
            report.append({"host": host, "configuration": configuration, "points": points, "operation": operation, "before_median_ms": baseline, **measured, "improvement_percent": improvement})
    (args.qa_root / "performance-summary.json").write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"PLOT_INTERACTION_PERFORMANCE_OK cases={len(report)} frames={len(report) * 24}")


if __name__ == "__main__":
    main()
