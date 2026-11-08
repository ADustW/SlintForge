# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param([string]$EvidenceDirectory, [string]$ConsumerDirectory)
. (Join-Path $PSScriptRoot 'Common.ps1')
Initialize-ForgeEnvironment (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0') 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe' 'D:/.clion-env/qt-6.8.3-msvc.bat'
if (!$EvidenceDirectory) { $EvidenceDirectory = Join-Path $script:ForgeRoot ('build/flow-consumers-' + (Get-Date -Format 'yyyyMMdd-HHmmss')) }
if (!$ConsumerDirectory) { $ConsumerDirectory = Join-Path $script:ForgeRoot ('build/flow-consumer-' + (Get-Date -Format 'yyyyMMdd-HHmmss')) }
$script:ForgeEvidence = [IO.Path]::GetFullPath($EvidenceDirectory)
$consumer = [IO.Path]::GetFullPath($ConsumerDirectory)
if (Test-Path -LiteralPath $consumer) { throw '使用新的独立验收目录，避免覆盖历史' }
[IO.Directory]::CreateDirectory($script:ForgeEvidence) | Out-Null
$env:SLINT_BACKEND = 'winit-skia'
$env:WGPU_BACKEND = 'dx12'
$env:SLINT_SCALE_FACTOR = '1'
Invoke-ForgeCheck 'prepare' $script:ForgePython @('tools/windows/flow_consumer_payload.py',$consumer)
Invoke-ForgeCheck 'install' $script:ForgeCMake @('--install','build/cpp','--config','Release','--prefix',"$consumer/sdk/cpp")
$env:SLINTFORGE_UI_LIBRARY=Join-Path $consumer 'sdk/cpp/share/slintforge/ui/slintforge.slint'
$env:SLINTFORGE_RUNTIME_LIBRARY=Join-Path $consumer 'sdk/rust/runtime/slintforge_runtime.dll'
Invoke-ForgeCheck 'cpp-configure' $script:ForgeCMake @('-S',"$consumer/examples/flow-cpp",'-B',"$consumer/cpp-build",'-G','Visual Studio 18 2026','-A','x64',"-DSlintForge_DIR=$consumer/sdk/cpp/lib/cmake/SlintForge","-DSlint_DIR=$script:ForgeSdk/lib/cmake/Slint") $consumer
Invoke-ForgeCheck 'cpp-build' $script:ForgeCMake @('--build',"$consumer/cpp-build",'--config','Release','--parallel','2') $consumer
Invoke-ForgeCheck 'cpp-sdk-contract' "$consumer/cpp-build/Release/slintforge_flow_example.exe" @('--sdk-contract') $consumer
$env:SLINTFORGE_FLOW_QA_DIR = Join-Path $script:ForgeEvidence 'cpp'
Invoke-ForgeCheck 'cpp-window' "$consumer/cpp-build/Release/slintforge_flow_example.exe" @('--smoke') $consumer
foreach ($scenario in @('edge','refinement','batch1','batch2','batch3')) {
    $env:SLINTFORGE_FLOW_QA_SCENARIO=Join-Path $consumer "examples/ui/flow-$scenario-qa.json"
    $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence "cpp-$scenario"
    Invoke-ForgeCheck "cpp-$scenario" "$consumer/cpp-build/Release/slintforge_flow_example.exe" @('--smoke') $consumer
}
$env:SLINTFORGE_FLOW_QA_SCENARIO=$null
# 依赖使用已缓存的固定版本；应用和 SDK 接口从独立目录重新编译。
$target = Join-Path $script:ForgeRoot 'target'
Invoke-ForgeCheck 'rust-build' 'cargo' @('build','--manifest-path',"$consumer/examples/flow-rust/Cargo.toml",'--target-dir',$target,'--release','--offline','--jobs','2','--bin','forgeflow-sdk-consumer') $consumer
$detached = Join-Path $consumer '独立运行'
[IO.Directory]::CreateDirectory($detached) | Out-Null
Copy-Item -LiteralPath "$target/release/forgeflow-sdk-consumer.exe" -Destination $detached
Copy-Item -LiteralPath "$consumer/sdk/rust/runtime/slintforge_runtime.dll" -Destination $detached
$env:SLINTFORGE_FLOW_QA_DIR = Join-Path $script:ForgeEvidence 'rust'
$savedPath = $env:PATH
try {
    $env:PATH = "$env:SystemRoot\System32;$env:SystemRoot"
    Invoke-ForgeCheck 'rust-sdk-contract' "$detached/forgeflow-sdk-consumer.exe" @('--sdk-contract') $detached
    Invoke-ForgeCheck 'rust-detached-window' "$detached/forgeflow-sdk-consumer.exe" @('--smoke') $detached
    foreach ($scenario in @('edge','refinement','batch1','batch2','batch3')) {
        $env:SLINTFORGE_FLOW_QA_SCENARIO=Join-Path $consumer "examples/ui/flow-$scenario-qa.json"
        $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence "rust-$scenario"
        Invoke-ForgeCheck "rust-detached-$scenario" "$detached/forgeflow-sdk-consumer.exe" @('--smoke') $detached
    }
} finally { $env:PATH = $savedPath; $env:SLINTFORGE_FLOW_QA_SCENARIO=$null }
$fingerprint = & $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
Write-ForgeJson ([ordered]@{ passed=$true; consumer_directory=$consumer; core_source_copied=$false; dependency_cache_reused=$true; runtime_sha256=(Get-FileHash -LiteralPath "$detached/slintforge_runtime.dll").Hash; source_fingerprint=$fingerprint; checks=$script:ForgeChecks; completed=(Get-Date).ToString('o') }) (Join-Path $script:ForgeEvidence 'result.json')
Write-Host "FLOW_EXTERNAL_CONSUMERS_OK $consumer"
