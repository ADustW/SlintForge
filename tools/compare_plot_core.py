# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""使用当前同一 MSVC 环境构建 HEAD/工作区内核，比较三轮中位数和峰值内存。"""
import csv
import ctypes
import io
import json
import os
from pathlib import Path
import statistics
import subprocess
import time
from measure_plot_migration import MemoryCounters

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "build/winui3/core-performance"


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    reference = OUTPUT / "reference"
    reference.mkdir(exist_ok=True)
    for name in ("ForgePlotCore.cpp", "ForgePlotCore.h"):
        (reference / name).write_bytes(subprocess.check_output(["git", "show", f"HEAD:sdk/common/{name}"], cwd=ROOT))
    compiler = subprocess.run(["cl"], capture_output=True, text=True).stderr
    data = {"reference": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(), "compiler": compiler, "runs": {}}
    get_memory = ctypes.windll.psapi.GetProcessMemoryInfo
    get_memory.argtypes = [ctypes.c_void_p, ctypes.POINTER(MemoryCounters), ctypes.c_ulong]
    for phase, source in (("before", reference), ("after", ROOT / "sdk/common")):
        directory = OUTPUT / phase
        directory.mkdir(exist_ok=True)
        exe = directory / "plot-performance.exe"
        command = ["cl", "/nologo", "/std:c++20", "/EHsc", "/O2", "/DNDEBUG", "/utf-8", "/I" + str(source), str(ROOT / "tests/cpp/ForgePlotPerformance.cpp"), str(source / "ForgePlotCore.cpp"), "/Fe:" + str(exe), "/link", "windowscodecs.lib", "ole32.lib", "gdi32.lib", "user32.lib"]
        build = subprocess.run(command, cwd=directory, capture_output=True)
        (directory / "build.log").write_bytes(build.stdout + build.stderr)
        if build.returncode:
            raise RuntimeError(f"编译失败：{directory / 'build.log'}")
        runs = []
        for trial in range(3):
            process = subprocess.Popen([str(exe)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, creationflags=subprocess.CREATE_NO_WINDOW)
            peak = 0
            while process.poll() is None:
                memory = MemoryCounters(); memory.cb = ctypes.sizeof(memory)
                if get_memory(int(process._handle), ctypes.byref(memory), memory.cb):
                    peak = max(peak, memory.PeakWorkingSetSize)
                time.sleep(.01)
            stdout, stderr = process.communicate()
            if process.returncode:
                raise RuntimeError(stderr.decode(errors="replace"))
            (directory / f"{trial}.csv").write_bytes(stdout)
            runs.append({"peak_bytes": peak, "rows": list(csv.DictReader(io.StringIO(stdout.decode())))})
        data["runs"][phase] = runs
    comparisons = []
    for index, count in enumerate((100000, 1000000)):
        for metric in ("prepare_ms", "interaction_ms"):
            before = statistics.median(float(v["rows"][index][metric]) for v in data["runs"]["before"])
            after = statistics.median(float(v["rows"][index][metric]) for v in data["runs"]["after"])
            comparisons.append({"points": count, "metric": metric, "before": before, "after": after, "regression": after / before - 1})
    before = statistics.median(v["peak_bytes"] for v in data["runs"]["before"])
    after = statistics.median(v["peak_bytes"] for v in data["runs"]["after"])
    comparisons.append({"metric": "peak_bytes", "before": before, "after": after, "regression": after / before - 1})
    data["comparisons"] = comparisons
    data["passed"] = all(v["regression"] <= .10 for v in comparisons)
    (OUTPUT / "comparison.json").write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")
    print(json.dumps({"passed": data["passed"], "comparisons": comparisons}), flush=True)
    if not data["passed"]:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
