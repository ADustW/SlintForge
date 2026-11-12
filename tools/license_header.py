# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

"""生成器输出文件统一使用的版权与许可声明。"""

LICENSE_HEADER = '// Copyright © 2026 陕西禾又科技有限公司\n// SPDX-License-Identifier: GPL-3.0-only\n\n'


def with_license_header(text: str) -> str:
    return text if text.startswith(LICENSE_HEADER) else LICENSE_HEADER + text


# 版权声明不属于接口契约，计算 ABI 摘要前去除，避免只改声明就让已发布 DLL 失配。
def without_license_header(text: str) -> str:
    return text[len(LICENSE_HEADER):] if text.startswith(LICENSE_HEADER) else text
