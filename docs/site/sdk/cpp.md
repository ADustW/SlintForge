# C++ SDK

头文件 SDK 覆盖相同的宿主服务族，并链接共享运行库。创建 UI 组件后再绑定 Slint 生成的全局对象。

## 头文件

- `sdk/cpp/include/ForgeCapture.hpp`
- `sdk/cpp/include/ForgeFlow.hpp`
- `sdk/cpp/include/ForgeFlowBindings.hpp`
- `sdk/cpp/include/ForgeFlowImageExport.hpp`
- `sdk/cpp/include/ForgeGrids.hpp`
- `sdk/cpp/include/ForgeIconCatalog.hpp`
- `sdk/cpp/include/ForgePlot.hpp`
- `sdk/cpp/include/ForgePlotBindings.hpp`
- `sdk/cpp/include/ForgeSdk.hpp`
- `sdk/cpp/include/ForgeStaggeredLayout.hpp`
- `sdk/cpp/include/ForgeWidgets.hpp`
- `sdk/cpp/include/ForgeWindowCorners.hpp`

## 构建集成

使用仓库 CMake 目标，使 Slint 生成头、运行库和 SDK 包含目录保持同一版本。
