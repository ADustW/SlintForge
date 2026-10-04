# Window and page capture

The baseline is Windows x64, Slint 1.18.0, and the `fluent` style. Rust and C++ use the same Slint components and return straight RGBA8 pixels. PNG export preserves the pixel dimensions and alpha channel.

`ForgeWindow.capture()` captures the displayed client area, including the custom title bar. `ForgePage.capture()` captures its current rectangle in the owning window. This includes the current scroll position, background, and composited overlays. It does not render an isolated subtree or content outside the visible window. Place a `ScrollView` inside a `ForgePage` to capture its viewport.

`ForgePage` provides `@children`, `content-padding` (default zero), inherited `clip` (default true), and `active` (default true). Deactivating a page keeps its child instances and input state. Bind `active` to the routing state when a parent controls page visibility.

## Shared Slint contract

Import and re-export `ForgeCaptureService`, `ForgeCaptureRect`, and `ForgeCaptureResult` from `@slintforge` in the consumer's Slint entry file. The generated host types are then available to the binding helpers.

`ForgeCaptureRect` uses logical pixels relative to the window client area. A successful result contains `ok`, `image`, `pixel-width`, and `pixel-height`; a failed result contains `error`. The service has `capture-window()` and `capture-region(rect)` callbacks. Bind it separately for each top-level window.

```slint
export { ForgeCaptureService, ForgeCaptureRect, ForgeCaptureResult } from "@slintforge";
```

## Rust

After creating the consumer window:

```rust
slintforge::bind_capture_service!(&ui, ForgeCaptureService, ForgeCaptureResult);
```

Call capture on the UI thread after showing the window and completing layout, for example from a button callback or timer:

```rust
let pixels = slintforge::capture_window(ui.window())?;
slintforge::save_capture_png(&pixels, std::path::Path::new("capture.png"))?;
let image = slint::Image::from_rgba8(pixels);
```

`capture_region(window, CaptureRect)` crops the current window snapshot. `crop_capture(pixels, rect, scale)` performs the same operation on an existing buffer. PNG encoding uses `png 0.18.1`.

## C++

Include `ForgeCapture.hpp` and bind the consumer's generated types:

```cpp
forge_bind_capture_service<ForgeCaptureService, ForgeCaptureResult>(ui);
auto capture = forge_capture_window(ui->window());
if (capture.ok()) {
    const auto error = forge_save_capture_png(*capture.pixels, std::filesystem::path(L"capture.png"));
    const auto image = slint::Image(*capture.pixels);
}
```

`forge_capture_region(window, ForgeLogicalCaptureRect)` and `forge_crop_capture(pixels, rect, scale)` provide region capture. `ForgeCapturedImage` contains the optional RGBA8 buffer and an error string. `forge_save_capture_png` returns an empty string on success. Windows PNG encoding uses WIC; the installed `SlintForge::Sdk` CMake target propagates `windowscodecs` and `ole32` to consumers.

## Bounds and failures

Both hosts use `Window.take_snapshot()`. Multiply logical bounds by `window.scale_factor()`, floor the starting coordinates, ceil the ending coordinates, and intersect the resulting rectangle with the snapshot bounds. Partially outside regions return their intersection. Nonfinite, nonpositive, and fully outside regions fail.

An unbound service, hidden window, inactive page, empty region, or unsupported renderer returns an explicit error. PNG export reports creation, encoding, and write failures. The caller chooses the output path and creates parent directories.

## Examples and verification

The shared Gallery has page/window capture buttons, image previews, physical pixel dimensions, and timestamped PNG export. Both minimal consumers demonstrate the same API. Run `--capture-smoke` on either Gallery host or consumer to exercise a real window. Gallery checks nested offsets, scrolling, external overlays, image fitting, resize/maximize, and export. Coordinate tests cover 100%, 125%, 150%, and 200% scaling; PNG tests independently decode exact RGBA bytes, including transparent pixels and Chinese file paths.

Native scaling can be checked with `SLINT_SCALE_FACTOR=1`, `1.25`, `1.5`, or `2`. Use a fresh installation prefix for the C++ SDK so that previously removed component files do not remain in an older installation.

See [the Chinese API reference](API.md) and [the verification record](CONTAINER_VERIFICATION.md) for the detailed contract and recorded results.
