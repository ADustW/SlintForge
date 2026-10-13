#pragma once
#include <stdint.h>

// 共用引擎持有文档、事务和历史；返回文本有效至同一对象的下一次调用。
// Flow 契约 2：增量行增加 style/arrow/stroke_width，状态增加 selected_edge/selected_edge_count。
// edge-appearance 仅更新外观，可撤销且不会修改端点；旧 JSON 默认线宽为 2。
#ifdef __cplusplus
extern "C" {
#endif
typedef struct ForgeFlowCore ForgeFlowCore;
ForgeFlowCore* forge_flow_create(const char* mode);
void forge_flow_destroy(ForgeFlowCore* flow);
const char* forge_flow_command(ForgeFlowCore* flow, const char* command);
const char* forge_flow_snapshot(ForgeFlowCore* flow);
const char* forge_flow_export(ForgeFlowCore* flow);
#ifdef __cplusplus
}
#endif
