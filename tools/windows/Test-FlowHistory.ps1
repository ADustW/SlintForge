# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param([string]$EvidenceDirectory)
. (Join-Path $PSScriptRoot 'Common.ps1')
Initialize-ForgeEnvironment (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0') 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe' 'D:/.clion-env/qt-6.8.3-msvc.bat'
if(!$EvidenceDirectory){$EvidenceDirectory=Join-Path $script:ForgeRoot ('build/flow-history-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))}
$script:ForgeEvidence=[IO.Path]::GetFullPath($EvidenceDirectory)
if(Test-Path -LiteralPath $script:ForgeEvidence){throw '请使用新的证据目录'}
[IO.Directory]::CreateDirectory($script:ForgeEvidence)|Out-Null
$saved=@{};foreach($name in @('SLINT_BACKEND','WGPU_BACKEND','SLINT_SCALE_FACTOR','SLINTFORGE_FLOW_QA_SCENARIO','SLINTFORGE_FLOW_QA_DIR','SLINTFORGE_RUNTIME_LIBRARY')){$saved[$name]=[Environment]::GetEnvironmentVariable($name)}
try {
    $fingerprint=& $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
    $scenario=Join-Path $script:ForgeEvidence 'scenario.json'
    Invoke-ForgeCheck 'scenario' $script:ForgePython @('tools/windows/flow_history_scenario.py',$scenario)
    $env:WGPU_BACKEND='dx12';$env:SLINTFORGE_FLOW_QA_SCENARIO=$scenario
    $env:SLINTFORGE_RUNTIME_LIBRARY=Join-Path $script:ForgeRoot 'target/release/slintforge_runtime.dll'
    foreach($hostName in @('cpp','rust')) {
        $program=if($hostName -eq 'cpp'){"$script:ForgeRoot/build/cpp/examples/flow-cpp/Release/slintforge_flow_example.exe"}else{"$script:ForgeRoot/target/release/slintforge-flow-example.exe"}
        Invoke-ForgeCheck "$hostName-sdk-contract" $program @('--sdk-contract')
        foreach($combination in @(@('winit-skia','1'),@('winit-skia-software','2'))) {
            $env:SLINT_BACKEND=$combination[0];$env:SLINT_SCALE_FACTOR=$combination[1]
            $label="$hostName-$($combination[0])-$($combination[1])"
            $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence $label
            Invoke-ForgeCheck $label $program @('--smoke')
        }
    }
    Write-ForgeJson @{passed=$true;scope='双端窗口历史预算、撤销重做和超大编辑边界';source_fingerprint=$fingerprint;checks=$script:ForgeChecks;completed=(Get-Date).ToString('o')} (Join-Path $script:ForgeEvidence 'result.json')
} catch {Write-ForgeJson @{passed=$false;error=$_.Exception.Message;checks=$script:ForgeChecks} (Join-Path $script:ForgeEvidence 'result.json');throw}
finally {foreach($name in $saved.Keys){[Environment]::SetEnvironmentVariable($name,$saved[$name],'Process')}}
