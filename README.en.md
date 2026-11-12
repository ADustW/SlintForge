[简体中文](README.md) | [English](README.en.md) | [Company website](https://shanxiheyoutech.com)

# SlintForge v0.1.2

**Developed and maintained by:** 陕西禾又科技有限公司 · [Visit the company website](https://shanxiheyoutech.com)

SlintForge is a desktop component library with a shared `.slint` implementation and separate Rust and C++ integration paths. It is an independent third-party project, not an official Slint project. The initial baseline is Windows x64, Slint **1.18.0**, and the `fluent` style. `ui/` is the single source for components and public UI types; `ui/slintforge.slint` is the public `@slintforge` export entry point. The two SDKs provide routing, navigation trees, the complete icon catalog, window and page capture with PNG saving, QR codes, syntax highlighting, background image handling, and Windows window-corner adaptation.

The Gallery uses categorized navigation, a top bar, sidebars, homepage sections, and example cards. The Slint code and visual assets in this project were written independently.

## Repository layout

| Path | Purpose |
|---|---|
| `ui/slintforge.slint` | Public `@slintforge` export entry point; component implementations live elsewhere |
| `ui/types` | Shared data types for navigation, content, and feedback |
| `ui/basic` | Basic extensions, one component per file |
| `ui/navigation` | Navigation components; rows and panels under `internal` are not exported |
| `ui/layouts` | Split, flow, adaptive grid, five-region dock, page stack, responsive form row, and staggered layouts |
| `ui/surfaces` | Capture, rounded images, icon-and-text, expanders, watermarks, and marquee surfaces |
| `ui/inputs`, `ui/form` | Validation, extended buttons, tags, colors, shortcuts, suggestions, multi-selection, and floating-point input |
| `ui/data`, `ui/feedback`, `ui/popups`, `ui/theme`, `ui/other` | Trees, timelines, QR codes, feedback, popups, materials, tours, syntax highlighting, and dials |
| `ui/window` | Borderless window components; internal title-bar buttons are not exported |
| `ui/typography`, `ui/icons`, `ui/assets/fonts` | Font configuration, icon-font components, and separately licensed font assets |
| `sdk/rust` | `Router`, `NavTree`, `StaggeredLayout`, `IconCatalog`, capture and PNG, window corners, and build paths |
| `sdk/cpp` | C++ counterparts, plus an installable CMake package |
| `Gallery/ui/screen`, `Gallery/ui/component`, `Gallery/ui/page` | Shared UI for the Rust and C++ Gallery hosts; `gallery.slint` is an export entry point |
| `Gallery/data/navigation.txt` | Shared category, hierarchy, page ID, and footer navigation catalog |
| `Gallery/data/component-status.txt` | Status of native controls, shared components, demos, and deferred pages |
| `Gallery/i18n` | Chinese and English Gallery translations and PO generation |
| `Gallery/ui/assets/brand` | Gallery icon source, PNG, and Windows ICO files |
| `Gallery/rust`, `Gallery/cpp` | Thin hosts for sampling, routing, and UI state synchronization |
| `examples/rust`, `examples/cpp` | Minimal consumers sharing `examples/ui/minimal.slint` |

Registered Gallery pages demonstrate native controls, shared components, navigation, basic charts, ForgePlot, grids, and ForgeFlow. The "Nodes and Edges" section follows "Charts" and contains basic editing, custom cards, flowcharts, topology diagrams, and mind maps. Navigation and example counts come from the catalogs. Both hosts use Skia by default and share settings, language, and UI contracts. See the [English API documentation](docs/site/en/index.md), [ForgeFlow integration](docs/FLOW.md), [component API](docs/COMPONENTS.md), [grid API](docs/GRIDS.md), [basic chart API](docs/CHARTS.md), and [ForgePlot API](docs/PLOT.md). The linked topic documents are currently in Chinese.

ForgePlot provides professional two-dimensional plotting features inspired by QCustomPlot: line, step and impulse series; scatter and error bars; fills between series; grouped and stacked bars; box plots; financial and color maps; polar plots; multiple axes and plot areas; and annotation items. It supports pan, zoom, selection, live data, and PNG/JPEG/BMP/SVG/PDF export. Bind `ForgePlotServices` once on either platform to use default coordinates and styles. Standalone demos are in `examples/plot-rust` and `examples/plot-cpp`; the Gallery's professional plotting page reuses the same component. See the [plot verification notes](docs/PLOT_VERIFICATION.md) for tested platforms and performance limits.

## Test and review scope

Test only the files, behavior, and direct dependencies affected by a change. Select Rust packages and test cases, C++ build targets and CTest names, and affected UI pages. Documentation-only changes do not require builds or functional tests. The project rules are in [AGENTS.md](AGENTS.md); [CLAUDE.md](CLAUDE.md) points to the same rules.

Workspace-wide tests, complete CTest runs, all-page Gallery captures, and full acceptance commands below are references to use when their scope is justified. Shared code changes require analysis of affected callers; they do not automatically require a full regression run.

## Build and run on Windows

The Rust workspace uses toolchain **1.98.0** for `x86_64-pc-windows-msvc` and needs a C++ compiler for the shared plotting core. C++ uses Visual Studio 2026 x64, CMake 3.21+, and the official Slint 1.18.0 Windows MSVC SDK. The configured build environment is loaded by `D:\.clion-env\qt-6.8.3-msvc.bat`. The standalone plotting example is part of the root Cargo workspace and uses the same `Cargo.lock`.

The current Windows packaging script produces one uncompressed directory and one installer with the API, Rust and C++ Gallery apps, components, and binary SDKs. It does **not** include the complete corresponding source. Its binary output is for internal verification only; public binary distribution requires the corresponding source for SlintForge and the bundled Slint SDK under GPLv3. See [Windows delivery](docs/RELEASE.md) and the [integration guide](docs/INTEGRATION.md).

```powershell
# Full acceptance entry points; do not run them by default for a small change.
pwsh -File tools/windows/Verify.ps1 -Mode Quick
pwsh -File tools/windows/Verify.ps1 -Mode Full -EvidenceDirectory build/acceptance
pwsh -File tools/windows/Package.ps1 -VerificationReport build/acceptance/result.json
```

Dependency paths can be overridden through script parameters. Existing caches default to `--offline --locked`; use `-Online` for first-time downloads. The scripts record commands, exit codes, logs, and JSON results, and tie acceptance and packaging to a source fingerprint.

```powershell
# Full-suite examples only. Use targeted commands from AGENTS.md for routine work.
cargo test --workspace
cargo run -p slintforge-gallery-rust
cargo run -p slintforge-example-rust

cmake -S . -B build/cpp -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_PREFIX_PATH="D:/path/to/slint-cpp-1.18.0" -DSLINT_STYLE=fluent
cmake --build build/cpp --config Debug
ctest --test-dir build/cpp -C Debug --output-on-failure

# Add the SDK's lib directory to PATH before launching C++ executables.
$env:PATH = "D:\path\to\slint-cpp-1.18.0\lib;$env:PATH"
& build/cpp/Gallery/cpp/Debug/slintforge_gallery_cpp.exe
& build/cpp/examples/cpp/Debug/slintforge_example_cpp.exe
```

`--smoke` checks Gallery settings and font routes, icon search and pagination, Chinese/English switching, fonts and text scale, always-on-top and corner settings, parameters, back navigation, expanded navigation, icons, title-bar states, panel positions, layout and surface routes, page-stack state, staggered reflow, and background sampling without showing a window. `--run-smoke` opens a real window, runs the event loop for about 850 ms, checks sampled values, and exits. Both flags work with the Rust and C++ Gallery hosts.

Use `--preview-top` and `--preview-right` to inspect the top dropdown and right-side navigation manually. `--preview-route=forge-window` opens a page by its stable route ID.

## Install the C++ package for another project

```powershell
cmake --install build/cpp --config Debug --prefix build/install
cmake -S examples/cpp -B build/external-cpp -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_PREFIX_PATH="D:/path/to/slint-cpp-1.18.0;D:/path/to/SlintForge/build/install"
cmake --build build/external-cpp --config Debug
```

External projects use `find_package(SlintForge 0.1.2 CONFIG REQUIRED)` to obtain `SlintForge::Sdk`, `slintforge_target_sources()`, and the Slint dependency. The helper registers the `@slintforge` path for a target. The current package installs Debug and Release libraries; the development environment still needs the official Slint 1.18.0 SDK separately.

The icon font is Microsoft Fluent System Icons Regular. Its asset and MIT license are in `ui/assets/fonts`; the C++ package includes the font, codepoint map, and license. Original SlintForge code is GPL-3.0-only, while Slint retains its own license notices. See the [license explanation](LICENSE.md). QR codes use the unmodified MIT-licensed Project Nayuki v1.8.0 source in `third_party/qrcodegen`, including its license; the C++ package includes that license.

## Surface and capture verification

The broader commands in this section are references. For a component change, run only checks for the affected component and capture path.

The eight surface components are `ForgePage`, `ForgeRoundImageView`, `ForgeIconLabel`, `ForgeExpander`, `ForgeWatermark`, `ForgeMarquee`, `ForgeCard`, and `ForgeWindow`. Native rounding and clipping use `Rectangle` directly. See the [API](docs/API.md) and [English capture guide](docs/CAPTURE.en.md).

```powershell
cargo test --workspace --locked
cargo run --locked -p slintforge-gallery-rust -- --capture-smoke
cargo run --locked -p slintforge-example-rust -- --capture-smoke
# After a C++ build and CTest, --capture-smoke can also run on the C++ hosts.
```

`--capture-smoke` opens a real window and checks whole-window and nested-page captures, physical dimensions after resizing and maximizing, error paths, and PNG writing and decoding. The hosts print their capture output paths. C++ CTest covers the SDK, pixel clipping and PNG, Gallery smoke checks, and real-window capture. Coordinate unit tests cover 100%, 125%, 150%, and 200% scaling; interaction tests cover expansion and keyboard use, retained state, watermark clicks, animation pause/resume, and text wrapping. Actual captures still need visual review.

## New components and page-level verification

For a new or changed component, verify that component and directly affected pages. The following broad entry points are for explicitly justified wider checks, not the default for each new component.

`--components-smoke` checks registered routes in dark Chinese, light English with 125% text scale, and a narrow Chinese window. It preserves popup, busy, chart, and grid states while capturing component examples. Each host writes route and scenario PNGs under `build/component-qa/rust` or `build/component-qa/cpp` and exits without changing user settings. The Gallery also offers side-by-side cards, scroll regions, and tabs for review; see the [example organization notes](docs/GALLERY_PRESENTATION.md). `--widgets-smoke` checks the minimal consumers' QR, highlighting, and material services; `--charts-smoke` checks seven basic chart types; `--grids-smoke` checks seven public grid states. The professional plotting examples support `--smoke` and `--benchmark`; see the [ForgePlot API](docs/PLOT.md).

```powershell
cargo test --workspace --offline --locked
cargo run -p slintforge-gallery-rust --offline --locked -- --components-smoke
cargo run -p slintforge-example-rust --offline --locked -- --widgets-smoke
cargo run -p slintforge-example-rust --offline --locked -- --grids-smoke
ctest --test-dir build/cpp -C Debug --output-on-failure
```

`tools/verify_components.py --compiler <path-to-slint-compiler>` compiles usage examples for new pages and checks the status catalog, translations, and first-party C++ namespace policy. `--qr` additionally decodes and compares QR pixels from both hosts using a test-only zxing-cpp dependency.

The latest topic-specific test, capture, and installed-consumer results and reproduction commands are in the [grid review](docs/GRID_REVIEW.md) and [plot verification](docs/PLOT_VERIFICATION.md). Earlier grid evidence remains in the [grid verification notes](docs/GRIDS_VERIFICATION.md). These reports describe their component development stages; current acceptance for v0.1.2 is determined by `ACCEPTANCE.md`, `verification`, and `manifest.json` in a timestamped release directory. The [October 4, 2026 component review](docs/COMPONENT_CODE_REVIEW_20261004.md) covers the public component boundary, navigation tree performance, and ForgePlot incremental updates.

## Rust DLL SDK

The Rust computation core is delivered as a DLL. Slint objects and service bindings remain in a native Rust interface. The supported baseline is Rust 1.98.0: clients build the public wrapper and Slint 1.18.0 with that toolchain, then call the core DLL through a versioned C ABI. Initial builds download and compile dependencies. The app bundles the DLL at runtime, and the tooling can copy it automatically. See [Rust DLL integration](docs/RUST_DLL.md) for delivery size and cache measurements. The current binary packaging limitation above still applies.

## License

Copyright © 2026 陕西禾又科技有限公司. Original SlintForge code is licensed under [GPL-3.0-only](LICENSE), which is compatible with Slint's open-source option. Third-party content retains its respective licenses. See the [license explanation](LICENSE.md) for Slint's three license options, proprietary-use conditions, and third-party inventory, and [REUSE.toml](REUSE.toml) for path-specific declarations.
