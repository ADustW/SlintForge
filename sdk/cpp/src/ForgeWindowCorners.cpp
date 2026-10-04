#include "ForgeWindowCorners.hpp"

#ifdef _WIN32
#include <dwmapi.h>
#include <windows.h>
#endif

bool forge_apply_window_corners(const slint::Window& window, bool rounded) {
#ifdef _WIN32
    const HWND hwnd = window.win32_hwnd();
    if (!hwnd) return false;
    const auto preference = rounded ? DWMWCP_ROUND : DWMWCP_DONOTROUND;
    return SUCCEEDED(DwmSetWindowAttribute(
        hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference)));
#else
    (void)window;
    (void)rounded;
    return false;
#endif
}
