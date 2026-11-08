# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""生成双端窗口的预算裁剪、重做分支和超大编辑边界场景。"""
import json
from pathlib import Path
import sys


def query(field, value):
    return {"kind": "query-expect", "request": {"target": "history"}, "path": f"/result/{field}", "value": value}


def configure(steps, size):
    return {"kind": "edit", "op": "history-configure", "text": json.dumps({"max_steps": steps, "max_bytes": size})}


def field(value, target="text"):
    return {"kind": "edit", "op": "field", "id": "node-1", "target": target, "text": value}


def repeat(op, count):
    return [{"kind": "edit", "op": op} for _ in range(count)]


steps = [{"kind": "size", "width": 1100, "height": 760},
         {"kind": "theme", "dark": False},
         {"kind": "edit", "op": "demo", "number": 1},
         configure(3, 4096)]
steps += [field(f"版本 {i}") for i in range(5)]
steps += [query("undo_count", 3), query("history_evicted", 2)]
steps += repeat("undo", 4)
steps += [{"kind": "expect", "path": "/nodes/0/data/text", "value": "版本 1"},
          query("redo_count", 3), {"kind": "snapshot", "name": "history-undo-boundary"}]
steps += repeat("redo", 4)
steps += [{"kind": "expect", "path": "/nodes/0/data/text", "value": "版本 4"}, configure(1, 4096)]
steps += repeat("undo", 1)
steps += [{"kind": "expect", "path": "/nodes/0/data/text", "value": "版本 3"}]
steps += repeat("redo", 1)
steps += [configure(0, 4096), field("关闭历史后编辑")]
steps += repeat("undo", 1)
steps += [{"kind": "expect", "path": "/nodes/0/data/text", "value": "关闭历史后编辑"},
          query("history_bytes", 0), configure(2, 1024), field("x" * 4096, "payload"),
          query("history_bytes", 0), query("history_barriers", 2),
          {"kind": "edit", "op": "title", "id": "node-1", "text": "大数据之后的小编辑"}, query("undo_count", 1)]
steps += repeat("undo", 1)
steps += [{"kind": "expect", "path": "/nodes/0/title", "value": "节点 1"},
          {"kind": "expect", "path": "/nodes/0/data/payload", "value": "x" * 4096}]
steps += repeat("redo", 1)
steps += [{"kind": "expect", "path": "/nodes/0/title", "value": "大数据之后的小编辑"},
          {"kind": "theme", "dark": True}, {"kind": "snapshot", "name": "history-redo-after-barrier"},
          {"kind": "edit", "op": "demo", "number": 1}, query("history_limit", 2),
          query("history_budget_bytes", 1024), query("history_bytes", 0), query("history_barriers", 0),
          {"kind": "finish"}]
Path(sys.argv[1]).write_text(json.dumps(steps, ensure_ascii=False, indent=2), encoding="utf-8")
print("FLOW_HISTORY_SCENARIO", len(steps))
