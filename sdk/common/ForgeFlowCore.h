// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <stdint.h>

// 共用引擎持有文档、事务和历史；返回文本有效至同一对象的下一次调用。
// Flow 契约 14：增量撤销、实例级历史预算及历史用量/边界统计。
// 节点和连线扩展字段有兼容默认值；操作号 36 至 40 保持不变。
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
