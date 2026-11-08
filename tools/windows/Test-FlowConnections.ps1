# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param([string]$EvidenceDirectory, [switch]$SkipBuild,
    [ValidateSet('cpp','rust')][string[]]$HostNames=@('cpp','rust'),
    [switch]$SkipCore, [switch]$SkipGallery, [switch]$SkipPerformance)
. (Join-Path $PSScriptRoot 'Common.ps1')
Initialize-ForgeEnvironment (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0') 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe' 'D:/.clion-env/qt-6.8.3-msvc.bat'
if (!$EvidenceDirectory) { $EvidenceDirectory = Join-Path $script:ForgeRoot ('build/flow-connections-' + (Get-Date -Format 'yyyyMMdd-HHmmss')) }
$script:ForgeEvidence = [IO.Path]::GetFullPath($EvidenceDirectory)
[IO.Directory]::CreateDirectory($script:ForgeEvidence) | Out-Null
$savedEnvironment = @{}
foreach ($key in @('SLINT_BACKEND','WGPU_BACKEND','SLINT_SCALE_FACTOR','SLINTFORGE_FLOW_QA_DIR','SLINTFORGE_FLOW_QA_SCENARIO','SLINTFORGE_COMPONENT_QA_FILTER','SLINTFORGE_COMPONENT_QA_EXCLUDE','SLINTFORGE_COMPONENT_QA_DIR','SLINTFORGE_UI_LIBRARY','SLINTFORGE_RUNTIME_LIBRARY')) { $savedEnvironment[$key] = [Environment]::GetEnvironmentVariable($key) }
try {
    $env:SLINTFORGE_UI_LIBRARY=Join-Path $script:ForgeRoot 'ui/slintforge.slint'
    $env:SLINTFORGE_RUNTIME_LIBRARY=Join-Path $script:ForgeRoot 'target/release/slintforge_runtime.dll'
    if (!$SkipBuild) {
        Invoke-ForgeCheck 'cpp-build' $script:ForgeCMake @('--build','build/cpp','--config','Release','--parallel','2','--target','slintforge_flow_tests','slintforge_flow_example','slintforge_gallery_cpp')
        Invoke-ForgeCheck 'rust-build' 'cargo' @('build','-p','slintforge-runtime','-p','slintforge-flow-example','-p','slintforge-gallery-rust','--release','--offline','--locked','-j','2')
    }
    if (!$SkipCore) {
    Invoke-ForgeCheck 'flow-core' "$script:ForgeRoot/build/cpp/Release/slintforge_flow_tests.exe" @()
    Invoke-ForgeCheck 'flow-bindings' $script:ForgePython @('tools/generate_flow_bindings.py','--check')
    Invoke-ForgeCheck 'runtime-ffi' $script:ForgePython @('tools/generate_runtime_ffi.py','--check')
    Invoke-ForgeCheck 'namespaces' $script:ForgePython @('tools/verify_first_party_namespaces.py')
    Invoke-ForgeCheck 'rust-flow-tests' 'cargo' @('test','-p','slintforge','--features','flow','--lib','flow::tests','--release','--offline','--locked','-j','2')
    Invoke-ForgeCheck 'runtime-compatibility' 'cargo' @('test','-p','slintforge','--test','dll_runtime','--release','--offline','--locked','-j','2')
    }
    $executables = @{cpp="$script:ForgeRoot/build/cpp/examples/flow-cpp/Release/slintforge_flow_example.exe"; rust="$script:ForgeRoot/target/release/slintforge-flow-example.exe"}
    $env:WGPU_BACKEND='dx12'
    # 输入验收串行执行，避免窗口焦点互相影响；只检查 Flow 及其 Gallery 接入。
    foreach ($hostName in $HostNames) {
        $env:SLINT_BACKEND='winit-skia'; $env:SLINT_SCALE_FACTOR='1'; $env:SLINTFORGE_FLOW_QA_SCENARIO=$null
        $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence "$hostName-editor"
        Invoke-ForgeCheck "$hostName-editor" $executables[$hostName] @('--smoke')
        $env:SLINTFORGE_FLOW_QA_SCENARIO=Join-Path $script:ForgeRoot 'examples/ui/flow-edge-qa.json'
        foreach ($renderer in @('winit-skia','winit-skia-software')) {
            foreach ($scale in @('1','1.5','2')) {
                $env:SLINT_BACKEND=$renderer; $env:SLINT_SCALE_FACTOR=$scale
                $label="$hostName-$renderer-$scale"; $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence $label
                Invoke-ForgeCheck $label $executables[$hostName] @('--smoke')
                $env:SLINTFORGE_FLOW_QA_SCENARIO=Join-Path $script:ForgeRoot 'examples/ui/flow-refinement-qa.json'
                $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence "$label-refinement"
                Invoke-ForgeCheck "$label-refinement" $executables[$hostName] @('--smoke')
                $env:SLINTFORGE_FLOW_QA_SCENARIO=Join-Path $script:ForgeRoot 'examples/ui/flow-batch1-qa.json'
                $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence "$label-batch1"
                Invoke-ForgeCheck "$label-batch1" $executables[$hostName] @('--smoke')
                $env:SLINTFORGE_FLOW_QA_SCENARIO=Join-Path $script:ForgeRoot 'examples/ui/flow-batch2-qa.json'
                $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence "$label-batch2"
                Invoke-ForgeCheck "$label-batch2" $executables[$hostName] @('--smoke')
                $env:SLINTFORGE_FLOW_QA_SCENARIO=Join-Path $script:ForgeRoot 'examples/ui/flow-edge-qa.json'
            }
        }
    }
    $env:SLINTFORGE_FLOW_QA_SCENARIO=$null; $env:SLINT_SCALE_FACTOR='1'; $env:SLINT_BACKEND='winit-skia'
    $env:SLINTFORGE_COMPONENT_QA_FILTER='flow-'; $env:SLINTFORGE_COMPONENT_QA_EXCLUDE=$null
    if (!$SkipGallery) { foreach ($hostName in $HostNames) {
        $env:SLINTFORGE_COMPONENT_QA_DIR=Join-Path $script:ForgeEvidence "gallery/$hostName"
        $executable=if($hostName -eq 'cpp'){"$script:ForgeRoot/build/cpp/Gallery/cpp/Release/slintforge_gallery_cpp.exe"}else{"$script:ForgeRoot/target/release/slintforge-gallery-rust.exe"}
        Invoke-ForgeCheck "$hostName-gallery-flow" $executable @('--components-smoke')
    } }
    if (!$SkipPerformance) { foreach ($hostName in $HostNames) {
        $env:SLINT_BACKEND=if($hostName -eq 'cpp'){'winit-skia-opengl'}else{'winit-skia'}
        $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence "$hostName-performance"
        foreach ($scenario in @('standard','mixed')) {
            $flags=@('--benchmark'); if($scenario -eq 'mixed'){$flags+='--complex'}
            Invoke-ForgeCheck "$hostName-performance-$scenario" $executables[$hostName] $flags
            $report=Get-Content -Raw (Join-Path $env:SLINTFORGE_FLOW_QA_DIR "performance-$scenario.json") | ConvertFrom-Json
            if(!$report.measurement_complete -or !$report.target_met){throw "$hostName/$scenario 性能未达标"}
        }
    } }
    $fingerprint=& $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
    Write-ForgeJson ([ordered]@{passed=$true; scope='ForgeFlow'; hosts=$HostNames; core_checked=(!$SkipCore); gallery_checked=(!$SkipGallery); performance_checked=(!$SkipPerformance); source_fingerprint=$fingerprint; checks=$script:ForgeChecks; completed=(Get-Date).ToString('o')}) (Join-Path $script:ForgeEvidence 'result.json')
    Write-Host "FLOW_CONNECTIONS_OK $script:ForgeEvidence"
} catch {
    Write-ForgeJson ([ordered]@{passed=$false; error=$_.Exception.Message; checks=$script:ForgeChecks}) (Join-Path $script:ForgeEvidence 'result.json')
    throw
} finally {
    foreach($key in $savedEnvironment.Keys){[Environment]::SetEnvironmentVariable($key,$savedEnvironment[$key],'Process')}
}
