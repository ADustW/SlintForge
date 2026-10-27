[CmdletBinding()]
param([string]$EvidenceDirectory)
. (Join-Path $PSScriptRoot 'Common.ps1')
Initialize-ForgeEnvironment (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0') 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe' 'D:/.clion-env/qt-6.8.3-msvc.bat'
if(!$EvidenceDirectory){$EvidenceDirectory=Join-Path $script:ForgeRoot ('build/flow-lifecycle-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))}
$script:ForgeEvidence=[IO.Path]::GetFullPath($EvidenceDirectory)
if(Test-Path -LiteralPath $script:ForgeEvidence){throw '请使用新的证据目录'}
[IO.Directory]::CreateDirectory($script:ForgeEvidence)|Out-Null
$flowSaved=@{};foreach($name in @('SLINT_BACKEND','WGPU_BACKEND','SLINT_SCALE_FACTOR','SLINTFORGE_FLOW_QA_SCENARIO','SLINTFORGE_FLOW_QA_DIR')){$flowSaved[$name]=[Environment]::GetEnvironmentVariable($name)}
$flowReports=[Collections.Generic.List[object]]::new()
try {
    $fingerprint=& $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
    $scenario=Join-Path $script:ForgeEvidence 'scenario.json'
    Invoke-ForgeCheck 'scenario' $script:ForgePython @('tools/windows/flow_lifecycle_scenario.py',$scenario)
    $env:SLINT_BACKEND='winit-skia';$env:WGPU_BACKEND='dx12';$env:SLINT_SCALE_FACTOR='1';$env:SLINTFORGE_FLOW_QA_SCENARIO=$scenario
    foreach($hostName in @('cpp','rust')) {
        $folder=Join-Path $script:ForgeEvidence $hostName;[IO.Directory]::CreateDirectory($folder)|Out-Null
        $env:SLINTFORGE_FLOW_QA_DIR=$folder
        $program=if($hostName -eq 'cpp'){"$script:ForgeRoot/build/cpp/examples/flow-cpp/Release/slintforge_flow_example.exe"}else{"$script:ForgeRoot/target/release/slintforge-flow-example.exe"}
        $stdout=Join-Path $folder 'stdout.log';$stderr=Join-Path $folder 'stderr.log'
        $watch=[Diagnostics.Stopwatch]::StartNew();$samples=[Collections.Generic.List[object]]::new()
        $process=Start-Process -FilePath $program -ArgumentList '--smoke' -WorkingDirectory $script:ForgeRoot -WindowStyle Hidden -RedirectStandardOutput $stdout -RedirectStandardError $stderr -PassThru
        $completed=$false
        try {
            while(!$process.HasExited) {
                $process.Refresh()
                $phase=Get-Content -LiteralPath $stdout -Tail 1 -ErrorAction SilentlyContinue
                $samples.Add(@{seconds=[Math]::Round($watch.Elapsed.TotalSeconds,2);private_bytes=$process.PrivateMemorySize64;working_set=$process.WorkingSet64;phase=$phase})
                if($watch.Elapsed.TotalSeconds -gt 420){throw '生命周期窗口场景超时'}
                Start-Sleep -Milliseconds 250
            }
            $process.WaitForExit();$completed=$true
            if($process.ExitCode -ne 0){throw "$hostName 生命周期窗口失败：$stderr"}
            if((Get-Content -Raw $stdout) -notmatch 'FLOW_WINDOW_OK'){throw '没有收到窗口场景完成标记'}
            $history=@($samples|Where-Object {$_.phase -match 'history-'});$recreate=@($samples|Where-Object {$_.phase -match 'recreating'})
            $report=@{host=$hostName;passed=$true;seconds=$watch.Elapsed.TotalSeconds;nodes=500;edges=1000;edits=900;history_limit=100;canvas_recreations=60;service_releases=64;peak_private_bytes=($samples.private_bytes|Measure-Object -Maximum).Maximum;history_start_private_bytes=$history[0].private_bytes;history_end_private_bytes=$history[-1].private_bytes;recreate_start_private_bytes=$recreate[0].private_bytes;recreate_end_private_bytes=$recreate[-1].private_bytes;samples=$samples.ToArray()}
            Write-ForgeJson $report (Join-Path $folder 'memory.json');$flowReports.Add($report)
            Write-Host "FLOW_LIFECYCLE_OK host=$hostName duration=$([Math]::Round($watch.Elapsed.TotalSeconds,1))s"
        } finally {if(!$completed -and !$process.HasExited){Stop-Process -Id $process.Id};$process.Dispose()}
    }
    Write-ForgeJson @{passed=$true;scope='Release sustained editing and repeated canvas/controller disposal';source_fingerprint=$fingerprint;reports=$flowReports.ToArray();memory_scope='记录本次持续编辑的进程内存趋势，不等于多小时运行或堆泄漏分析';completed=(Get-Date).ToString('o')} (Join-Path $script:ForgeEvidence 'result.json')
} catch {Write-ForgeJson @{passed=$false;error=$_.Exception.Message;reports=$flowReports.ToArray()} (Join-Path $script:ForgeEvidence 'result.json');throw}
finally {foreach($name in $flowSaved.Keys){[Environment]::SetEnvironmentVariable($name,$flowSaved[$name],'Process')}}
