# SlintForge 许可说明

Copyright © 2026 陕西禾又科技有限责任公司

除下文“第三方组件”列出的内容外，本仓库的源代码、`.slint` 界面文件、脚本、文档及 Gallery 资源均由陕西禾又科技有限责任公司享有版权，并按 **GNU General Public License v3.0 only**（SPDX：`GPL-3.0-only`）授权。许可全文见 [`LICENSE`](LICENSE)，另有一份副本位于 [`LICENSES/GPL-3.0-only.txt`](LICENSES/GPL-3.0-only.txt)。

```
SPDX-FileCopyrightText: 2026 陕西禾又科技有限责任公司
SPDX-License-Identifier: GPL-3.0-only
```

逐路径的版权与许可声明记录在 [`REUSE.toml`](REUSE.toml) 中，格式遵循 [REUSE 规范](https://reuse.software/spec/)，与 Slint 官方仓库相同。

## 与 Slint 许可的关系

SlintForge 基于 [Slint](https://slint.dev) 开发。Slint 由 SixtyFPS GmbH 以三选一方式授权：

```
GPL-3.0-only OR LicenseRef-Slint-Royalty-free-2.0 OR LicenseRef-Slint-Software-3.0
```

本仓库分发 Slint 时选择其中的 GPL-3.0-only；SlintForge 自有代码也由权利人选择 GPL-3.0-only。两项选择分别适用于各自的代码：

- Slint 的 Royalty-free 和商业许可只授予对 Slint 的权利，不会改变 SlintForge 自有代码的 GPL 许可。
- Slint 官方说明允许开发者把自己的代码按与 GPL 兼容的宽松许可开源；本项目权利人选择 GPL-3.0-only，并非 Slint 要求所有独立源文件必须使用 GPL。
- Royalty-free 许可不允许单独分发 Slint，也不允许分发暴露 Slint API 的应用。SlintForge 作为组件库和 SDK 提供，因此本仓库选择 Slint 的 GPL 路径公开。

**对使用者的影响**

- 按 GPL-3.0 使用 SlintForge 时，分发的衍生作品须整体遵守 GPL-3.0：提供完整对应源代码，保留版权和许可声明。
- 如需闭源分发基于 SlintForge 的产品，须先取得陕西禾又科技有限责任公司对 SlintForge 的另行书面授权，并根据实际分发方式另行满足 Slint 的适用许可。符合官方条件的桌面、移动或 Web 终端应用可能使用 Slint Royalty-free 许可并履行署名要求；单独分发 SDK 或暴露 Slint API 的产品不适用该许可，需选择 Slint 允许的其他路径。详见 [Slint 官方许可 FAQ](https://github.com/slint-ui/slint/blob/master/FAQ.md#licensing)。

**二进制打包现状：**当前 `tools/windows/Package.ps1` 生成的目录和安装包只含公共接口源码，不含 SlintForge 核心实现及包内 Slint SDK 的对应源码。该打包产物仅供内部验证，尚不能直接作为 GPLv3 对外发行包；公开二进制发行还需满足上文的对应源码和声明要求。参见 [`docs/RELEASE.md`](docs/RELEASE.md)。

## 第三方组件

以下内容不属于陕西禾又科技有限责任公司的版权作品，按各自上游许可提供，原始许可文件随附在对应目录：

| 路径 | 组件 | 版权人 | 许可 |
|---|---|---|---|
| `third_party/nlohmann/` | JSON for Modern C++ 3.12.0 | Niels Lohmann | MIT，见 `third_party/nlohmann/LICENSE.MIT` |
| `third_party/qrcodegen/` | QR-Code-generator 1.8.0 | Project Nayuki | MIT，见 `third_party/qrcodegen/LICENSE.txt` |
| `ui/assets/fonts/FluentSystemIcons-*` | Fluent System Icons 1.1.343 | Microsoft Corporation | MIT，见 `ui/assets/fonts/LICENSE-FluentSystemIcons.txt` |
| `ui/styles/slintforge-winui3/upstream/` 中的 `.slint` | Slint 1.18.0 标准控件 | SixtyFPS GmbH；个别文件另有 KDAB 署名 | `GPL-3.0-only OR LicenseRef-Slint-Royalty-free-2.0 OR LicenseRef-Slint-Software-3.0`，以各文件头为准 |
| `ui/styles/slintforge-winui3/upstream/fluent/_*.svg` | Slint 随附的 Fluent 图标 | Microsoft Corporation | MIT |
| `ui/styles/slintforge-winui3/upstream/common/MadeWithSlint-logo-*.svg` | Made with Slint 徽标 | SixtyFPS GmbH | CC-BY-ND-4.0 |

`ui/styles/slintforge-winui3/upstream/` 保持上游原文，不添加本公司版权声明；上游许可全文见同目录的 `LICENSE-*` 和 [`LICENSES/`](LICENSES)。

通过 Cargo、CMake 或 npm 获取的依赖（包括 Slint、Skia 等）不随本仓库分发，其许可以各依赖自身声明为准。

## 免责声明

本程序按“原样”提供，不附带任何明示或暗示的担保。具体条款见 GPL-3.0 第 15、16 条。
