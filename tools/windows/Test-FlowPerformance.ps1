[CmdletBinding()]
param([string]$EvidenceDirectory, [switch]$VerifyInteractions)
. (Join-Path $PSScriptRoot 'Common.ps1')
Initialize-ForgeEnvironment (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0') 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe' 'D:/.clion-env/qt-6.8.3-msvc.bat'
if (!$EvidenceDirectory) { $EvidenceDirectory=Join-Path $script:ForgeRoot ('build/flow-performance-'+(Get-Date -Format 'yyyyMMdd-HHmmss')) }
$script:ForgeEvidence=[IO.Path]::GetFullPath($EvidenceDirectory)
[IO.Directory]::CreateDirectory($script:ForgeEvidence) | Out-Null
$env:WGPU_BACKEND='dx12'; $env:SLINT_SCALE_FACTOR='1'
$executables=@{cpp="$script:ForgeRoot/build/cpp/examples/flow-cpp/Release/slintforge_flow_example.exe";rust="$script:ForgeRoot/target/release/slintforge-flow-example.exe"}
$reports=[Collections.Generic.List[object]]::new()
try {
    if ($VerifyInteractions) {
        $env:SLINT_BACKEND='winit-skia'
        foreach($hostName in @('cpp','rust')) { foreach($scenario in @('flow-qa.json','flow-batch1-qa.json')) {
            $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence "$hostName-$scenario"
            $env:SLINTFORGE_FLOW_QA_SCENARIO=Join-Path $script:ForgeRoot "examples/ui/$scenario"
            Invoke-ForgeCheck "$hostName-$scenario" $executables[$hostName] @('--smoke')
        } }
    }
    $env:SLINTFORGE_FLOW_QA_SCENARIO=$null
    # 串行测量，构建完成后调用，避免并行编译或窗口争用影响帧记录。
    foreach($hostName in @('rust','cpp')) {
        $env:SLINT_BACKEND=if($hostName -eq 'cpp'){'winit-skia-opengl'}else{'winit-skia'}
        $env:SLINTFORGE_FLOW_QA_DIR=Join-Path $script:ForgeEvidence "$hostName-performance"
        foreach($scenario in @('standard','mixed')) {
            $flags=@('--benchmark'); if($scenario -eq 'mixed'){$flags+='--complex'}
            Invoke-ForgeCheck "$hostName-$scenario" $executables[$hostName] $flags
            $report=Get-Content -Raw (Join-Path $env:SLINTFORGE_FLOW_QA_DIR "performance-$scenario.json") | ConvertFrom-Json
            $reports.Add($report); Write-Host ($report | ConvertTo-Json -Depth 6 -Compress)
            if(!$report.measurement_complete -or !$report.target_met){throw "$hostName/$scenario 性能未达标"}
        }
    }
    $fingerprint=& $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
    Write-ForgeJson ([ordered]@{passed=$true;source_fingerprint=$fingerprint;reports=$reports;checks=$script:ForgeChecks;completed=(Get-Date).ToString('o')}) (Join-Path $script:ForgeEvidence 'result.json')
    Write-Host "FLOW_PERFORMANCE_OK $script:ForgeEvidence"
} catch {
    Write-ForgeJson ([ordered]@{passed=$false;error=$_.Exception.Message;reports=$reports;checks=$script:ForgeChecks}) (Join-Path $script:ForgeEvidence 'result.json')
    throw
}
