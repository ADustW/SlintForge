# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param([string]$EvidenceDirectory, [ValidateSet('cpp','rust')][string[]]$HostNames=@('cpp','rust'))
. (Join-Path $PSScriptRoot 'Common.ps1')
Initialize-ForgeEnvironment (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0') 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe' 'D:/.clion-env/qt-6.8.3-msvc.bat'
if(!$EvidenceDirectory){$EvidenceDirectory=Join-Path $script:ForgeRoot ('build/flow-routing-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))}
$script:ForgeEvidence=[IO.Path]::GetFullPath($EvidenceDirectory)
[IO.Directory]::CreateDirectory($script:ForgeEvidence)|Out-Null
$savedEnvironment=@{}
foreach($key in @('SLINT_BACKEND','WGPU_BACKEND','SLINT_SCALE_FACTOR','SLINTFORGE_FLOW_QA_DIR','SLINTFORGE_FLOW_QA_SCENARIO')){$savedEnvironment[$key]=[Environment]::GetEnvironmentVariable($key)}
try {
    $env:WGPU_BACKEND='dx12'
    # 窗口测试串行运行；应先在当前源码上构建 Release 的 C++ 和 Rust 示例。
    foreach($hostName in $HostNames){
        $executable=if($hostName -eq 'cpp'){Join-Path $script:ForgeRoot 'build/cpp/examples/flow-cpp/Release/slintforge_flow_example.exe'}else{Join-Path $script:ForgeRoot 'target/release/slintforge-flow-example.exe'}
        foreach($renderer in @('winit-skia','winit-skia-software')){foreach($scale in @('1','1.5','2')){
            $label="$hostName-$renderer-$scale"
            $env:SLINT_BACKEND=$renderer;$env:SLINT_SCALE_FACTOR=$scale
            $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence $label
            $env:SLINTFORGE_FLOW_QA_SCENARIO=Join-Path $script:ForgeRoot 'examples/ui/flow-routing-qa.json'
            Invoke-ForgeCheck $label $executable @('--smoke')
            Invoke-ForgeCheck "$label-pixels" $script:ForgePython @('tools/windows/flow_routing_evidence.py','--check',$env:SLINTFORGE_FLOW_QA_DIR,'--scale',$scale)
            $env:SLINTFORGE_FLOW_QA_SCENARIO=Join-Path $script:ForgeRoot 'examples/ui/flow-edge-qa.json'
            $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence "$label-edges"
            Invoke-ForgeCheck "$label-edges" $executable @('--smoke')
        }}
        $env:SLINT_BACKEND='winit-skia';$env:SLINT_SCALE_FACTOR='1'
        foreach($scenario in @('refinement','batch3')){
            $env:SLINTFORGE_FLOW_QA_SCENARIO=Join-Path $script:ForgeRoot "examples/ui/flow-$scenario-qa.json"
            $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence "$hostName-$scenario"
            Invoke-ForgeCheck "$hostName-$scenario" $executable @('--smoke')
        }
    }
    $fingerprint=& $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
    Write-ForgeJson ([ordered]@{passed=$true;hosts=$HostNames;source_fingerprint=$fingerprint;checks=$script:ForgeChecks;completed=(Get-Date).ToString('o')}) (Join-Path $script:ForgeEvidence 'result.json')
    Write-Host "FLOW_ROUTING_WINDOWS_OK $script:ForgeEvidence"
}catch{
    Write-ForgeJson ([ordered]@{passed=$false;error=$_.Exception.Message;checks=$script:ForgeChecks}) (Join-Path $script:ForgeEvidence 'result.json')
    throw
}finally{
    foreach($key in $savedEnvironment.Keys){[Environment]::SetEnvironmentVariable($key,$savedEnvironment[$key],'Process')}
}
