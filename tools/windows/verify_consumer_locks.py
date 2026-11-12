# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

"""独立消费者只能使用交付锁文件中已有的 registry 依赖版本。"""
from pathlib import Path
import sys
import tomllib


def registry_packages(path):
    data = tomllib.loads(Path(path).read_text(encoding="utf-8"))
    return {(p["name"], p["version"], p["source"], p.get("checksum"))
            for p in data["package"] if p.get("source", "").startswith("registry+")}


expected, actual = (registry_packages(p) for p in sys.argv[1:3])
extra = actual - expected
if extra:
    raise SystemExit(f"消费者依赖偏离交付锁文件：{sorted(extra)}")
print(f"CONSUMER_LOCK_OK registry_packages={len(actual)}")
