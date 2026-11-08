# C++ SDK

The header SDK covers the same host-service families and links to the shared runtime. Bind generated Slint globals only after the UI component is created.

## Headers

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

## Build integration

Use the repository CMake targets so the generated Slint headers, runtime library, and SDK include directory stay version-aligned.
