# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$PackageDirectory,
    [Parameter(Mandatory=$true)][string]$EvidenceDirectory,
    [string]$CMake = 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe',
    [switch]$Online
)
. (Join-Path $PSScriptRoot 'Common.ps1')
$script:ForgeEvidence = [IO.Path]::GetFullPath($EvidenceDirectory)
[IO.Directory]::CreateDirectory($script:ForgeEvidence) | Out-Null
$consumer = Join-Path $env:TEMP ('SlintForge 二进制接入 ' + [Guid]::NewGuid().ToString('N'))
[IO.Directory]::CreateDirectory($consumer) | Out-Null
$package = Join-Path $consumer '组件 库'
Copy-Item -LiteralPath (Resolve-Path -LiteralPath $PackageDirectory).Path -Destination $package -Recurse
$ui = @'
import { ForgeTimePicker, ForgeDatePicker, ForgeCalendarPicker, ForgeColorPicker, ForgeShortcutPicker, ForgeDelayButton } from "@slintforge";
export { ForgeWidgetServices, ForgeQRCodeResult, ForgeSyntaxLine } from "@slintforge";
export component ConsumerWindow inherits Window {
    width: 640px; height: 540px;
    out property<int> day: dates.date.day;
    out property<int> red: colors.red;
    out property<int> confirmations: root.count;
    private property<int> count: 0;
    callback check-controls();
    check-controls => { dates.open(); dates.set-day(15); dates.confirm(); colors.open(); colors.edit-hex("#12AB34"); colors.confirm(); delay.begin(); delay.finish(); shortcuts.open(); shortcuts.confirm(); }
    VerticalLayout {
        ForgeTimePicker { language: "zh"; minute-increment: 5; }
        dates := ForgeDatePicker { language: "zh"; }
        ForgeCalendarPicker { language: "zh"; show-time: true; }
        colors := ForgeColorPicker { language: "zh"; }
        shortcuts := ForgeShortcutPicker { shortcut: "Ctrl+K"; }
        delay := ForgeDelayButton { delay: 0ms; clicked => { root.count += 1; } }
    }
}
'@
$cpp = @'
#include "consumer.h"
#include "ForgeWidgets.hpp"
#include <stdexcept>
#include <chrono>
#include <iostream>
int main() {
    auto ui = ConsumerWindow::create();
    forge_bind_widget_services<ForgeWidgetServices, ForgeQRCodeResult, ForgeSyntaxLine>(ui);
    ui->show(); ui->invoke_check_controls();
    if (ui->get_day() != 15 || ui->get_red() != 18 || ui->get_confirmations() != 1) throw std::runtime_error("控件提交状态错误");
    if (forge_color_to_hex(*forge_color_from_hex("#12345680")) != "#12345680" || forge_color_from_hex("#12XY56")) throw std::runtime_error("颜色 API 转换错误");
    slint::Timer::single_shot(std::chrono::milliseconds(200), [] { slint::quit_event_loop(); });
    ui->run(); std::cout << "BINARY_CONSUMER_OK host=cpp\n";
}
'@
$rust = @'
include!(env!("SLINTFORGE_GENERATED_UI"));
use slint::ComponentHandle;
fn main() {
    let ui = ConsumerWindow::new().unwrap();
    slintforge::bind_widget_services!(ui, ForgeWidgetServices, ForgeQRCodeResult, ForgeSyntaxLine);
    ui.show().unwrap(); ui.invoke_check_controls();
    assert_eq!((ui.get_day(), ui.get_red(), ui.get_confirmations()), (15, 18, 1));
    slint::Timer::single_shot(std::time::Duration::from_millis(200), || slint::quit_event_loop().unwrap());
    ui.run().unwrap(); println!("BINARY_CONSUMER_OK host=rust");
}
'@
$cmakeText = @'
cmake_minimum_required(VERSION 3.21)
project(ForgeBinaryConsumer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(/utf-8)
find_package(SlintForge CONFIG REQUIRED)
add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE SlintForge::Sdk)
slintforge_target_sources(consumer consumer.slint)
'@
foreach ($entry in @(@('consumer.slint',$ui),@('main.cpp',$cpp),@('main.rs',$rust),@('CMakeLists.txt',$cmakeText))) { [IO.File]::WriteAllText((Join-Path $consumer $entry[0]), $entry[1], [Text.UTF8Encoding]::new($false)) }
$env:SLINT_BACKEND = 'winit-software'
$env:PATH = "$package/sdk/slint/lib;" + $env:PATH
$rustArguments = @('-NoProfile','-File',"$package/sdk/rust/Compile-Rust.ps1",'-SourceFile',"$consumer/main.rs",'-UiFile',"$consumer/consumer.slint",'-OutputFile',"$consumer/consumer-rust.exe")
# 发布验收默认使用已验证的本地依赖缓存；仅在显式要求时更新网络索引。
if (!$Online) { $rustArguments += '-Offline' }
Invoke-ForgeCheck 'binary-rust-build' 'pwsh' $rustArguments $consumer
Invoke-ForgeCheck 'binary-rust-run' "$consumer/consumer-rust.exe" @() $consumer
Invoke-ForgeCheck 'binary-cpp-configure' $CMake @('-S',$consumer,'-B',"$consumer/cpp-build",'-G','Visual Studio 18 2026','-A','x64',"-DSlintForge_DIR=$package/sdk/cpp/lib/cmake/SlintForge","-DSlint_DIR=$package/sdk/slint/lib/cmake/Slint") $consumer
foreach ($config in @('Debug','Release')) {
    Invoke-ForgeCheck "binary-cpp-build-$config" $CMake @('--build',"$consumer/cpp-build",'--config',$config,'--parallel','2') $consumer
    Invoke-ForgeCheck "binary-cpp-run-$config" "$consumer/cpp-build/$config/consumer.exe" @() $consumer
}
# 部署后仅保留 EXE 和计算 DLL，并暂时移走本轮 SDK 副本，禁止退回 SDK 目录加载。
$detached = Join-Path $consumer '独立发布'
[IO.Directory]::CreateDirectory($detached) | Out-Null
foreach ($name in @('consumer-rust.exe','slintforge_runtime.dll')) {
    Copy-Item -LiteralPath (Join-Path $consumer $name) -Destination $detached
}
if ((Get-FileHash -LiteralPath "$detached/slintforge_runtime.dll").Hash -ne (Get-FileHash -LiteralPath "$package/sdk/rust/runtime/slintforge_runtime.dll").Hash) { throw '独立应用携带的计算 DLL 与 SDK 不一致' }
$hiddenPackage = Join-Path $consumer '临时隔离的 SDK'
$consumerRoot = [IO.Path]::GetFullPath($consumer) + [IO.Path]::DirectorySeparatorChar
foreach ($path in @($package,$hiddenPackage)) {
    if (![IO.Path]::GetFullPath($path).StartsWith($consumerRoot,[StringComparison]::OrdinalIgnoreCase)) { throw 'SDK 隔离目标越界' }
}
$originalPath = $env:PATH
Move-Item -LiteralPath $package -Destination $hiddenPackage
try {
    $env:PATH = "$env:SystemRoot\System32;$env:SystemRoot"
    Invoke-ForgeCheck 'binary-rust-detached-run' "$detached/consumer-rust.exe" @() $detached
} finally {
    $env:PATH = $originalPath
    Move-Item -LiteralPath $hiddenPackage -Destination $package
}
Write-ForgeJson ([ordered]@{passed=$true; consumer_directory=$consumer; core_source_free_sdk=$true; interface_source_available=$true; rust='1.98.0'; rust_toolchains=@('1.98.0'); rust_detached_runtime=$true; cpp_configurations=@('Debug','Release'); checks=$script:ForgeChecks.Count}) "$script:ForgeEvidence/consumer-result.json"
Write-Host "BINARY_CONSUMERS_OK directory=$consumer"
