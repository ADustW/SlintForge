# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param([string]$EvidenceDirectory, [string]$WindowEvidenceDirectory, [string]$LifecycleEvidenceDirectory, [string]$PerformanceEvidenceDirectory)
. (Join-Path $PSScriptRoot 'Common.ps1')
Initialize-ForgeEnvironment (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0') 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe' 'D:/.clion-env/qt-6.8.3-msvc.bat'
if (!$EvidenceDirectory) { $EvidenceDirectory=Join-Path $script:ForgeRoot ('build/flow-batch3-acceptance-' + (Get-Date -Format 'yyyyMMdd-HHmmss')) }
$script:ForgeEvidence=[IO.Path]::GetFullPath($EvidenceDirectory)
if(Test-Path -LiteralPath $script:ForgeEvidence){throw '请使用新的验收目录，保留历史记录'}
[IO.Directory]::CreateDirectory($script:ForgeEvidence) | Out-Null
$flowShell=(Get-Process -Id $PID).Path
$flowSavedEnvironment=@{}
foreach($name in @('SLINT_BACKEND','WGPU_BACKEND','SLINT_SCALE_FACTOR','SLINTFORGE_COMPONENT_QA_FILTER','SLINTFORGE_COMPONENT_QA_DIR','SLINTFORGE_COMPONENT_QA_EXCLUDE','SLINTFORGE_UI_LIBRARY','SLINTFORGE_RUNTIME_LIBRARY')) { $flowSavedEnvironment[$name]=[Environment]::GetEnvironmentVariable($name) }
try {
    # 调用前必须完成两端 Release 构建；此入口不在窗口运行期间更新 DLL。
    $env:SLINTFORGE_UI_LIBRARY=Join-Path $script:ForgeRoot 'ui/slintforge.slint'
    $env:SLINTFORGE_RUNTIME_LIBRARY=Join-Path $script:ForgeRoot 'target/release/slintforge_runtime.dll'
    $fingerprint=& $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
    Invoke-ForgeCheck 'rust-flow-tests' 'cargo' @('test','-p','slintforge','--features','flow','--lib','flow::tests','--release','--offline','--locked','-j','2')
    Invoke-ForgeCheck 'runtime-compatibility' 'cargo' @('test','-p','slintforge','--test','dll_runtime','--release','--offline','--locked','-j','2')
    Invoke-ForgeCheck 'component-catalog' $script:ForgePython @('tools/component_catalog.py','--check')
    Invoke-ForgeCheck 'localization' $script:ForgePython @('Gallery/i18n/sync.py','--check')
    foreach($example in Get-ChildItem (Join-Path $script:ForgeRoot 'examples/component-usage/flow-*.slint')) {
        Invoke-ForgeCheck ('snippet-' + $example.BaseName) (Join-Path $script:ForgeSdk 'bin/slint-compiler.exe') @($example.FullName,'-f','cpp','-o',(Join-Path $script:ForgeEvidence ($example.BaseName + '.h')),'--style','fluent','-L',"slintforge=$env:SLINTFORGE_UI_LIBRARY")
    }
    if($WindowEvidenceDirectory) {
        $flowWindowReport=Get-Content -Raw -LiteralPath (Join-Path $WindowEvidenceDirectory 'result.json') | ConvertFrom-Json
        if(!$flowWindowReport.passed -or $flowWindowReport.quick -or !$flowWindowReport.regression_checked -or $flowWindowReport.hosts -notcontains 'cpp' -or $flowWindowReport.hosts -notcontains 'rust' -or $flowWindowReport.source_fingerprint -ne $fingerprint){throw '提供的窗口记录不是当前源码的完整双端通过记录'}
        $flowWindows=[IO.Path]::GetFullPath($WindowEvidenceDirectory)
    } else {
        $flowWindows=Join-Path $script:ForgeEvidence 'windows'
        Invoke-ForgeCheck 'window-matrix' $flowShell @('-NoProfile','-File',"$PSScriptRoot/Test-FlowBatch3.ps1",'-SkipBuild','-EvidenceDirectory',$flowWindows)
    }
    $env:SLINT_BACKEND='winit-skia';$env:WGPU_BACKEND='dx12';$env:SLINT_SCALE_FACTOR='1'
    $env:SLINTFORGE_COMPONENT_QA_FILTER='flow-';$env:SLINTFORGE_COMPONENT_QA_EXCLUDE=$null
    foreach($hostName in @('cpp','rust')) {
        $env:SLINTFORGE_COMPONENT_QA_DIR=Join-Path $script:ForgeEvidence "gallery/$hostName"
        $program=if($hostName -eq 'cpp'){"$script:ForgeRoot/build/cpp/Gallery/cpp/Release/slintforge_gallery_cpp.exe"}else{"$script:ForgeRoot/target/release/slintforge-gallery-rust.exe"}
        Invoke-ForgeCheck "$hostName-gallery" $program @('--components-smoke')
    }
    # Gallery 和 Flow 测试窗口均已退出后，才能重新编译外部 Rust 消费项目。
    Invoke-ForgeCheck 'external-consumers' $flowShell @('-NoProfile','-File',"$PSScriptRoot/Test-FlowConsumers.ps1",'-EvidenceDirectory',(Join-Path $script:ForgeEvidence 'consumers'),'-ConsumerDirectory',(Join-Path $script:ForgeEvidence 'consumer-project'))
    $finalFingerprint=& $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
    if($fingerprint -ne $finalFingerprint){throw '验收期间源码或脚本发生变化，请保留本记录并复核变化影响'}
    $lifecycle=Get-Content -Raw -LiteralPath (Join-Path $LifecycleEvidenceDirectory 'result.json') | ConvertFrom-Json
    $performance=Get-Content -Raw -LiteralPath (Join-Path $PerformanceEvidenceDirectory 'result.json') | ConvertFrom-Json
    if(!$lifecycle.passed -or $lifecycle.source_fingerprint -ne $fingerprint){throw '生命周期记录不是当前源码的通过记录'}
    if(!$performance.measurement_complete -or $performance.source_fingerprint -ne $fingerprint){throw '性能记录不完整或不是当前源码'}
    $artifacts=@{}
    foreach($file in @('build/cpp/examples/flow-cpp/Release/slintforge_flow_example.exe','build/cpp/Gallery/cpp/Release/slintforge_gallery_cpp.exe','target/release/slintforge-flow-example.exe','target/release/slintforge-gallery-rust.exe','target/release/slintforge_runtime.dll')) {
        $artifacts[$file]=(Get-FileHash -LiteralPath (Join-Path $script:ForgeRoot $file) -Algorithm SHA256).Hash
    }
    Write-ForgeJson @{passed=[bool]$performance.passed;functional_passed=$true;scope='ForgeFlow batch3 acceptance with explicit performance gate';source_fingerprint=$fingerprint;checks=$script:ForgeChecks;window_evidence=$flowWindows;artifacts_sha256=$artifacts;performance_checked=$true;performance_evidence=$PerformanceEvidenceDirectory;lifecycle_evidence=$LifecycleEvidenceDirectory;physical_touch_checked=$false;system_dpi_switch_checked=$false;completed=(Get-Date).ToString('o')} (Join-Path $script:ForgeEvidence 'result.json')
    Write-Host "FLOW_BATCH3_FUNCTIONAL_ACCEPTANCE_OK $script:ForgeEvidence"
} catch {
    Write-ForgeJson @{passed=$false;error=$_.Exception.Message;checks=$script:ForgeChecks;source_fingerprint=$fingerprint} (Join-Path $script:ForgeEvidence 'result.json')
    throw
} finally {
    foreach($name in $flowSavedEnvironment.Keys){[Environment]::SetEnvironmentVariable($name,$flowSavedEnvironment[$name],'Process')}
}
