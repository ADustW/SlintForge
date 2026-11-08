# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$SourceArchive,
    [Parameter(Mandatory=$true)][string]$CppArchive,
    [Parameter(Mandatory=$true)][string]$EvidenceDirectory,
    [string]$ConsumerDirectory,
    [string]$SlintSdk = (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0'),
    [string]$Python = 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe',
    [string]$CMake = 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe',
    [string]$ToolchainEnvironment = 'D:/.clion-env/qt-6.8.3-msvc.bat',
    [int]$Jobs = 2,
    [switch]$Online
)
. (Join-Path $PSScriptRoot 'Common.ps1')
$script:ForgeEvidence = [IO.Path]::GetFullPath($EvidenceDirectory)
[IO.Directory]::CreateDirectory($script:ForgeEvidence) | Out-Null
Initialize-ForgeEnvironment $SlintSdk $Python $CMake $ToolchainEnvironment
if (!$ConsumerDirectory) {
    $ConsumerDirectory = Join-Path (Split-Path $script:ForgeRoot) ('交付 验收/' + (Get-Date -Format 'yyyyMMdd_HHmmss'))
}
$consumerRoot = [IO.Path]::GetFullPath($ConsumerDirectory)
if ($consumerRoot.StartsWith($script:ForgeRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw '消费者必须位于仓库外' }
if (Test-Path -LiteralPath $consumerRoot) { throw "消费者目录已存在：$consumerRoot" }
if ($consumerRoot -notmatch '[\u4e00-\u9fff]' -or $consumerRoot -notmatch ' ') { throw '消费者验收路径必须含中文和空格' }
[IO.Directory]::CreateDirectory($consumerRoot) | Out-Null
try {
    foreach ($archive in @($SourceArchive,$CppArchive)) {
        Expand-Archive -LiteralPath $archive -DestinationPath $consumerRoot
    }
    $sourceSdk = Join-Path $consumerRoot "SlintForge-$script:ForgeVersion-source"
    $cppSdk = Join-Path $consumerRoot "SlintForge-$script:ForgeVersion-cpp"
    # 从已交付的源码 ZIP 复制独立消费者；构建不引用原仓库的 UI、头文件或实现。
    $examples = Join-Path $consumerRoot '业务 示例'
    [IO.Directory]::CreateDirectory($examples) | Out-Null
    foreach ($name in @('rust','cpp','plot-rust','plot-cpp','ui')) {
        Copy-Item -LiteralPath "$sourceSdk/examples/$name" -Destination $examples -Recurse
    }
    $sdkPath = ($sourceSdk + '/sdk/rust').Replace('\','/')
    $env:CARGO_TARGET_DIR = Join-Path $consumerRoot 'rust-target'
    foreach ($project in @(@('rust','slintforge-example-rust'),@('plot-rust','slintforge-plot-example'))) {
        $crate = Join-Path $examples $project[0]
        $manifest = @"
[package]
name = "$($project[1])"
version = "$script:ForgeVersion"
edition = "2021"
build = "build.rs"
[dependencies]
slint = "=1.18.0"
slintforge = { path = "$sdkPath" }
[build-dependencies]
slint-build = "=1.18.0"
slintforge = { path = "$sdkPath" }
[dev-dependencies]
i-slint-backend-testing = "=1.18.0"
[profile.dev.package."*"]
opt-level = 3
"@
        [IO.File]::WriteAllText("$crate/Cargo.toml", $manifest, [Text.UTF8Encoding]::new($false))
        Copy-Item -LiteralPath "$sourceSdk/Cargo.lock" -Destination "$crate/Cargo.lock"
        Copy-Item -LiteralPath "$sourceSdk/rust-toolchain.toml" -Destination "$crate/rust-toolchain.toml"
        # 首次解析移除锁文件里不再属于消费者的工作区项，其余依赖版本保持锁定。
        $flags = @('metadata','--format-version','1','--filter-platform','x86_64-pc-windows-msvc'); if (!$Online) { $flags += '--offline' }
        Invoke-ForgeCheck ('rust-lock-' + $project[0]) 'cargo' $flags $crate
        Invoke-ForgeCheck ('rust-lock-versions-' + $project[0]) $script:ForgePython @("$PSScriptRoot/verify_consumer_locks.py","$sourceSdk/Cargo.lock","$crate/Cargo.lock")
        $flags = @('build','--locked','--bin',$project[1]); if (!$Online) { $flags += '--offline' }
        Invoke-ForgeCheck ('rust-build-' + $project[0]) 'cargo' $flags $crate
    }
    foreach ($project in @('cpp','plot-cpp')) {
        $build = Join-Path $examples ('build-' + $project)
        Invoke-ForgeCheck ('cpp-configure-' + $project) $script:ForgeCMake @('-S',"$examples/$project",'-B',$build,'-G','Visual Studio 18 2026','-A','x64',"-DSlint_DIR=$script:ForgeSdk/lib/cmake/Slint","-DSlintForge_DIR=$cppSdk/lib/cmake/SlintForge",'-DSLINT_STYLE=fluent')
        foreach ($configuration in @('Debug','Release')) {
            Invoke-ForgeCheck ('cpp-build-' + $project + '-' + $configuration) $script:ForgeCMake @('--build',$build,'--config',$configuration,'--parallel',"$Jobs")
        }
    }
    $env:SLINTFORGE_INTERACTION_QA_DIR = "$script:ForgeEvidence/business/rust"
    $env:SLINTFORGE_PLOT_QA_DIR = "$script:ForgeEvidence/plot/rust"
    $rustExample = "$env:CARGO_TARGET_DIR/debug/slintforge-example-rust.exe"
    foreach ($scenario in @('capture','widgets','charts','grids','suggestion','dialog')) {
        Invoke-ForgeCheck "rust-$scenario" $rustExample @("--$scenario-smoke") $examples
    }
    Invoke-ForgeCheck 'rust-plot' "$env:CARGO_TARGET_DIR/debug/slintforge-plot-example.exe" @('--smoke') $examples
    foreach ($configuration in @('Debug','Release')) {
        $env:SLINTFORGE_INTERACTION_QA_DIR = "$script:ForgeEvidence/business/cpp-$configuration"
        $env:SLINTFORGE_PLOT_QA_DIR = "$script:ForgeEvidence/plot/cpp-$configuration"
        foreach ($scenario in @('capture','widgets','charts','grids','suggestion','dialog')) {
            Invoke-ForgeCheck "cpp-$configuration-$scenario" "$examples/build-cpp/$configuration/slintforge_example_cpp.exe" @("--$scenario-smoke") $examples
        }
        Invoke-ForgeCheck "cpp-$configuration-plot" "$examples/build-plot-cpp/$configuration/slintforge_plot_example.exe" @('--smoke') $examples
    }
    # 记录两个 SDK 的实际来源及 CMake 导入库，便于追溯迁移验收。
    Write-ForgeJson ([ordered]@{ passed=$true; consumer_directory=$consumerRoot; source_archive=$SourceArchive; cpp_archive=$CppArchive; source_sdk=$sourceSdk; cpp_sdk=$cppSdk; slint_sdk=$script:ForgeSdk; configurations=@('Debug','Release'); checks=$script:ForgeChecks.Count }) "$script:ForgeEvidence/consumer-result.json"
} catch {
    Write-ForgeJson ([ordered]@{ passed=$false; error=$_.Exception.Message; consumer_directory=$consumerRoot; checks=$script:ForgeChecks.Count }) "$script:ForgeEvidence/consumer-result.json"
    throw
}
Write-Host "CONSUMERS_OK directory=$consumerRoot"
