# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

"""在同一后端记录三轮波形性能及峰值内存，供图表迁移前后比较。"""
import argparse
import ctypes
import json
import os
from pathlib import Path
import subprocess
import time


class MemoryCounters(ctypes.Structure):
    _fields_ = [("cb", ctypes.c_ulong), ("PageFaultCount", ctypes.c_ulong)] + [
        (name, ctypes.c_size_t) for name in (
            "PeakWorkingSetSize", "WorkingSetSize", "QuotaPeakPagedPoolUsage",
            "QuotaPagedPoolUsage", "QuotaPeakNonPagedPoolUsage", "QuotaNonPagedPoolUsage",
            "PagefileUsage", "PeakPagefileUsage",
        )
    ]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--phase", choices=("before", "after"), required=True)
    parser.add_argument("--root", type=Path, default=Path("build/winui3/performance"))
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    env = dict(os.environ, SLINT_BACKEND="winit-software")
    env["PATH"] = str(root.parent / "slint-sdk-1.18.0/lib") + os.pathsep + env["PATH"]
    hosts = {
        "rust": root / "target/debug/slintforge-plot-example.exe",
        "cpp": root / "build/cpp/examples/plot-cpp/Release/slintforge_plot_example.exe",
    }
    get_memory = ctypes.windll.psapi.GetProcessMemoryInfo
    get_memory.argtypes = [ctypes.c_void_p, ctypes.POINTER(MemoryCounters), ctypes.c_ulong]
    for host, exe in hosts.items():
        for trial in range(3):
            for mode in ("benchmark", "interaction-benchmark"):
                directory = (args.root / args.phase / host / str(trial) / mode).resolve()
                directory.mkdir(parents=True, exist_ok=True)
                env["SLINTFORGE_PLOT_QA_DIR"] = str(directory)
                with (directory / "run.log").open("w", encoding="utf-8") as log:
                    started = time.monotonic()
                    process = subprocess.Popen([str(exe), "--" + mode], cwd=root, env=env,
                                               stdout=log, stderr=subprocess.STDOUT,
                                               creationflags=subprocess.CREATE_NO_WINDOW)
                    peak = 0
                    while process.poll() is None:
                        memory = MemoryCounters()
                        memory.cb = ctypes.sizeof(memory)
                        if get_memory(int(process._handle), ctypes.byref(memory), memory.cb):
                            peak = max(peak, memory.PeakWorkingSetSize)
                        if time.monotonic() - started > 180:
                            process.kill()
                            raise RuntimeError(f"基准超时：{host} {mode}")
                        time.sleep(.05)
                result = {"host": host, "trial": trial, "mode": mode,
                          "exit_code": process.returncode, "peak_working_set": peak,
                          "elapsed_seconds": time.monotonic() - started}
                (directory / "process.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
                print(json.dumps(result), flush=True)
                if process.returncode:
                    raise RuntimeError(f"基准失败，见 {directory / 'run.log'}")


if __name__ == "__main__":
    main()
