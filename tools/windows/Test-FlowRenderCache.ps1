# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param([string]$EvidenceDirectory,[ValidateSet('cpp','rust')][string[]]$HostNames=@('cpp','rust'),[switch]$Quick)
. (Join-Path $PSScriptRoot 'Common.ps1')
Initialize-ForgeEnvironment (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0') 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe' 'D:/.clion-env/qt-6.8.3-msvc.bat'
if(!$EvidenceDirectory){$EvidenceDirectory=Join-Path $script:ForgeRoot ('build/flow-render-cache-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))}
$script:ForgeEvidence=[IO.Path]::GetFullPath($EvidenceDirectory)
if(Test-Path -LiteralPath $script:ForgeEvidence){throw '请使用新的证据目录'}
[IO.Directory]::CreateDirectory($script:ForgeEvidence)|Out-Null
$flowSaved=@{}
foreach($name in @('SLINT_BACKEND','WGPU_BACKEND','SLINT_SCALE_FACTOR','SLINTFORGE_FLOW_QA_DIR','SLINTFORGE_FLOW_QA_SCENARIO')){$flowSaved[$name]=[Environment]::GetEnvironmentVariable($name)}
$fingerprint=& $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
try {
    $env:WGPU_BACKEND='dx12'
    $combinations=if($Quick){@(@('winit-skia','1'),@('winit-skia-software','2'))}else{@(@('winit-skia','1'),@('winit-skia','1.5'),@('winit-skia','2'),@('winit-skia-software','1'),@('winit-skia-software','1.5'),@('winit-skia-software','2'))}
    foreach($hostName in $HostNames){
        $program=if($hostName -eq 'cpp'){"$script:ForgeRoot/build/cpp/examples/flow-cpp/Release/slintforge_flow_example.exe"}else{"$script:ForgeRoot/target/release/slintforge-flow-example.exe"}
        foreach($combination in $combinations){
            $env:SLINT_BACKEND=$combination[0];$env:SLINT_SCALE_FACTOR=$combination[1]
            foreach($scenario in @('background','render-cache')){
                $label="$hostName-$($combination[0])-$($combination[1])-$scenario"
                $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence $label
                $env:SLINTFORGE_FLOW_QA_SCENARIO=Join-Path $script:ForgeRoot "examples/ui/flow-$scenario-qa.json"
                Invoke-ForgeCheck $label $program @('--smoke')
                if($scenario -eq 'background'){Invoke-ForgeCheck ($label+'-images') $script:ForgePython @('tools/windows/flow_background_evidence.py',$env:SLINTFORGE_FLOW_QA_DIR)}
                if($scenario -eq 'render-cache'){Invoke-ForgeCheck ($label+'-images') $script:ForgePython @('tools/windows/flow_render_cache_evidence.py',$env:SLINTFORGE_FLOW_QA_DIR)}
            }
        }
    }
    Write-ForgeJson @{passed=$true;source_fingerprint=$fingerprint;hosts=$HostNames;quick=[bool]$Quick;checks=$script:ForgeChecks;completed=(Get-Date).ToString('o')} (Join-Path $script:ForgeEvidence 'result.json')
} catch {
    Write-ForgeJson @{passed=$false;source_fingerprint=$fingerprint;error=$_.Exception.Message;checks=$script:ForgeChecks} (Join-Path $script:ForgeEvidence 'result.json');throw
} finally {
    foreach($name in $flowSaved.Keys){[Environment]::SetEnvironmentVariable($name,$flowSaved[$name],'Process')}
}
