# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

"""生成大图持续编辑、历史边界和反复释放的真实窗口场景。"""
import json
from pathlib import Path
import sys

steps = [{"kind":"edit","op":"demo","number":1,"count":500},
         {"kind":"count","nodes":500,"edges":1000},
         {"kind":"edit","op":"node-patch","id":"node-500","text":json.dumps({"is_group":True,"kind":"group","title":"生命周期分组"})}]
steps += [{"kind":"edit","op":"reparent","id":f"node-{i}","target":"node-500"} for i in range(5,21)]
steps += [{"kind":"edit","op":"layout","text":json.dumps({"algorithm":"layered","direction":"LR","parent":"node-500"})},
          {"kind":"count","nodes":500,"edges":1000},
          {"kind":"lifecycle-marker","name":"editing"}]
for i in range(900):
    steps.append({"kind":"edit","op":"field","id":"node-1","target":"text","text":f"持续编辑 {i}"})
    if i in (100,400,899):
        steps += [{"kind":"query-expect","request":{"target":"document-status"},"path":"/result/undo_count","value":100},
                  {"kind":"lifecycle-marker","name":f"history-{i}"}]
steps += [{"kind":"edit","op":"undo"} for _ in range(100)]
steps += [{"kind":"query-expect","request":{"target":"document-status"},"path":"/result/redo_count","value":100}]
steps += [{"kind":"edit","op":"redo"} for _ in range(100)]
steps += [{"kind":"expect","path":"/nodes/0/data/text","value":"持续编辑 899"},
          {"kind":"snapshot","name":"large-history"},
          {"kind":"edit","op":"demo","number":1},
          {"kind":"lifecycle-marker","name":"recreating"}]
for _ in range(60):
    steps += [{"kind":"dispose"},{"kind":"controllers","value":0},{"kind":"reopen"},{"kind":"controllers","value":1}]
steps += [{"kind":"lifecycle"},{"kind":"controllers","value":1},{"kind":"count","nodes":6,"edges":5},
          {"kind":"lifecycle-marker","name":"released"},{"kind":"snapshot","name":"lifecycle-finished"},{"kind":"finish"}]
Path(sys.argv[1]).write_text(json.dumps(steps,ensure_ascii=False,indent=2),encoding="utf-8")
print("FLOW_LIFECYCLE_SCENARIO",len(steps))
