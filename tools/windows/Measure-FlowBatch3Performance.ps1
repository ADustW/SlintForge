# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param([string]$EvidenceDirectory, [switch]$ResumedRepair)
. (Join-Path $PSScriptRoot 'Common.ps1')
Initialize-ForgeEnvironment (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0') 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe' 'D:/.clion-env/qt-6.8.3-msvc.bat'
if(!$EvidenceDirectory){$EvidenceDirectory=Join-Path $script:ForgeRoot ('build/flow-batch3-performance-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))}
$script:ForgeEvidence=[IO.Path]::GetFullPath($EvidenceDirectory)
if(Test-Path -LiteralPath $script:ForgeEvidence){throw '请使用新目录保留历史性能记录'}
[IO.Directory]::CreateDirectory($script:ForgeEvidence)|Out-Null
$reports=[Collections.Generic.List[object]]::new()
$fingerprint=& $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
$machine=@{cpu=(Get-CimInstance Win32_Processor|Select-Object -First 1 -ExpandProperty Name);memory_gib=[Math]::Round((Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory/1GB,1);gpu=@(Get-CimInstance Win32_VideoController|Select-Object Name,DriverVersion);os=(Get-CimInstance Win32_OperatingSystem).Caption}
$env:WGPU_BACKEND='dx12';$env:SLINT_SCALE_FACTOR='1';$env:SLINTFORGE_FLOW_QA_SCENARIO=$null
# 正式测量必须保留全部绘制层；授权后的新一轮修复单独记录，不覆盖之前的失败记录。
$env:SLINTFORGE_FLOW_PROFILE_LAYER=$null;$env:SLINT_DEBUG_PERFORMANCE=$null
try {
    foreach($hostName in @('rust','cpp')) {
        $env:SLINT_BACKEND=if($hostName -eq 'cpp'){'winit-skia-opengl'}else{'winit-skia'}
        $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence $hostName
        $program=if($hostName -eq 'cpp'){"$script:ForgeRoot/build/cpp/examples/flow-cpp/Release/slintforge_flow_example.exe"}else{"$script:ForgeRoot/target/release/slintforge-flow-example.exe"}
        foreach($scenario in @('standard','mixed')) {
            $flags=@('--benchmark');if($scenario -eq 'mixed'){$flags+='--complex'}
            Invoke-ForgeCheck "$hostName-$scenario" $program $flags
            $report=Get-Content -Raw (Join-Path $env:SLINTFORGE_FLOW_QA_DIR "performance-$scenario.json")|ConvertFrom-Json
            if ($hostName -eq 'rust' -and $report.acceptance_eligible -ne $true) { throw 'Rust 性能报告关闭了绘制层，不能用于正式验收' }
            $reports.Add($report)
            Write-Host "FLOW_MEASURED host=$hostName scene=$scenario frame_p95=$($report.frame_p95_ms) feedback_p95=$($report.feedback_p95_ms) passed=$($report.target_met)"
        }
    }
    $complete=$reports.Count -eq 4 -and @($reports|Where-Object {!$_.measurement_complete}).Count -eq 0
    $passed=$complete -and @($reports|Where-Object {!$_.target_met}).Count -eq 0
    Write-ForgeJson @{passed=$passed;measurement_complete=$complete;source_fingerprint=$fingerprint;machine=$machine;reports=$reports.ToArray();checks=$script:ForgeChecks;repair_loop_reopened=[bool]$ResumedRepair;completed=(Get-Date).ToString('o')} (Join-Path $script:ForgeEvidence 'result.json')
} catch {Write-ForgeJson @{passed=$false;measurement_complete=$false;source_fingerprint=$fingerprint;machine=$machine;reports=$reports.ToArray();error=$_.Exception.Message} (Join-Path $script:ForgeEvidence 'result.json');throw}
