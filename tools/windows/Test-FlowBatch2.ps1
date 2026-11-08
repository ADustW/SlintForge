# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param([string]$EvidenceDirectory, [switch]$SkipBuild, [switch]$Quick, [switch]$SkipRegression,
    [ValidateSet('cpp','rust')][string[]]$HostNames=@('cpp','rust'))
. (Join-Path $PSScriptRoot 'Common.ps1')
Initialize-ForgeEnvironment (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0') 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe' 'D:/.clion-env/qt-6.8.3-msvc.bat'
if (!$EvidenceDirectory) { $EvidenceDirectory=Join-Path $script:ForgeRoot ('build/flow-batch2-' + (Get-Date -Format 'yyyyMMdd-HHmmss')) }
$script:ForgeEvidence=[IO.Path]::GetFullPath($EvidenceDirectory)
[IO.Directory]::CreateDirectory($script:ForgeEvidence) | Out-Null
$savedFlowEnvironment=@{}
foreach($name in @('SLINT_BACKEND','WGPU_BACKEND','SLINT_SCALE_FACTOR','SLINTFORGE_FLOW_QA_DIR','SLINTFORGE_FLOW_QA_SCENARIO','SLINTFORGE_UI_LIBRARY','SLINTFORGE_RUNTIME_LIBRARY')) { $savedFlowEnvironment[$name]=[Environment]::GetEnvironmentVariable($name) }
try {
    $env:SLINTFORGE_UI_LIBRARY=Join-Path $script:ForgeRoot 'ui/slintforge.slint'
    $env:SLINTFORGE_RUNTIME_LIBRARY=Join-Path $script:ForgeRoot 'target/release/slintforge_runtime.dll'
    if (!$SkipBuild) {
        Invoke-ForgeCheck 'cpp-build' $script:ForgeCMake @('--build','build/cpp','--config','Release','--parallel','2','--target','slintforge_flow_tests','slintforge_flow_example')
        Invoke-ForgeCheck 'rust-build' 'cargo' @('build','-p','slintforge-runtime','-p','slintforge-flow-example','--release','--offline','--locked','-j','2')
    }
    $fingerprint=& $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
    Invoke-ForgeCheck 'flow-core' "$script:ForgeRoot/build/cpp/Release/slintforge_flow_tests.exe" @()
    Invoke-ForgeCheck 'bindings' $script:ForgePython @('tools/generate_flow_bindings.py','--check')
    Invoke-ForgeCheck 'runtime-ffi' $script:ForgePython @('tools/generate_runtime_ffi.py','--check')
    Invoke-ForgeCheck 'namespaces' $script:ForgePython @('tools/verify_first_party_namespaces.py')
    $env:WGPU_BACKEND='dx12'
    $combinations=if($Quick){@(@('winit-skia','1'),@('winit-skia-software','2'))}else{@(@('winit-skia','1'),@('winit-skia','1.5'),@('winit-skia','2'),@('winit-skia-software','1'),@('winit-skia-software','1.5'),@('winit-skia-software','2'))}
    # 原生窗口串行运行，避免焦点相互影响；性能与正式外部消费验收单独执行。
    foreach($hostName in $HostNames) {
        $program=if($hostName -eq 'cpp'){"$script:ForgeRoot/build/cpp/examples/flow-cpp/Release/slintforge_flow_example.exe"}else{"$script:ForgeRoot/target/release/slintforge-flow-example.exe"}
        Invoke-ForgeCheck "$hostName-sdk-contract" $program @('--sdk-contract')
        foreach($combination in $combinations) {
            $env:SLINT_BACKEND=$combination[0];$env:SLINT_SCALE_FACTOR=$combination[1]
            $label="$hostName-$($combination[0])-$($combination[1])"
            $env:SLINTFORGE_FLOW_QA_SCENARIO=Join-Path $script:ForgeRoot 'examples/ui/flow-batch2-qa.json'
            $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence $label
            Invoke-ForgeCheck $label $program @('--smoke')
        }
        if(!$SkipRegression) {
            $env:SLINT_BACKEND='winit-skia';$env:SLINT_SCALE_FACTOR='1'
            foreach($scenario in @('qa','edge-qa','refinement-qa','batch1-qa')) {
                $env:SLINTFORGE_FLOW_QA_SCENARIO=Join-Path $script:ForgeRoot "examples/ui/flow-$scenario.json"
                $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence "$hostName-$scenario"
                Invoke-ForgeCheck "$hostName-$scenario" $program @('--smoke')
            }
        }
    }
    Write-ForgeJson @{passed=$true; scope='ForgeFlow batch2 current scenarios'; hosts=$HostNames; quick=[bool]$Quick; regression_checked=(!$SkipRegression); external_consumers_checked=$false; performance_checked=$false; source_fingerprint=$fingerprint; checks=$script:ForgeChecks; completed=(Get-Date).ToString('o')} (Join-Path $script:ForgeEvidence 'result.json')
    Write-Host "FLOW_BATCH2_OK $script:ForgeEvidence"
} catch {
    Write-ForgeJson @{passed=$false; error=$_.Exception.Message; checks=$script:ForgeChecks} (Join-Path $script:ForgeEvidence 'result.json')
    throw
} finally {
    foreach($name in $savedFlowEnvironment.Keys){[Environment]::SetEnvironmentVariable($name,$savedFlowEnvironment[$name],'Process')}
}
