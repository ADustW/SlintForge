# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param([string]$EvidenceDirectory)
$savedOptions = @{}
foreach ($name in @('SLINT_BACKEND','SLINT_SCALE_FACTOR','SLINTFORGE_COMPONENT_QA_FILTER','SLINTFORGE_COMPONENT_QA_EXCLUDE','SLINTFORGE_COMPONENT_QA_MINIMIZE_ONCE','SLINTFORGE_COMPONENT_QA_DIR')) {
    $savedOptions[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
}
. (Join-Path $PSScriptRoot 'Common.ps1')
Initialize-ForgeEnvironment (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0') 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe' 'D:/.clion-env/qt-6.8.3-msvc.bat'
if (!$EvidenceDirectory) { $EvidenceDirectory = Join-Path $script:ForgeRoot ('build/flow-legacy-' + (Get-Date -Format 'yyyyMMdd-HHmmss')) }
$script:ForgeEvidence = [IO.Path]::GetFullPath($EvidenceDirectory)
[IO.Directory]::CreateDirectory($script:ForgeEvidence) | Out-Null
$env:SLINT_BACKEND = 'winit-software'
$env:SLINT_SCALE_FACTOR = '1'
$env:SLINTFORGE_COMPONENT_QA_FILTER = $null
$env:SLINTFORGE_COMPONENT_QA_EXCLUDE = 'flow-'
$env:SLINTFORGE_COMPONENT_QA_MINIMIZE_ONCE = $null
$running = @()
$results = @()
try {
    foreach ($hostName in @('cpp','rust')) {
        $executable = if ($hostName -eq 'cpp') { "$script:ForgeRoot/build/cpp/Gallery/cpp/Debug/slintforge_gallery_cpp.exe" } else { "$script:ForgeRoot/target/debug/slintforge-gallery-rust.exe" }
        if (!(Test-Path -LiteralPath $executable)) { throw '请先执行 Verify.ps1 -Mode Full 构建两端 Gallery' }
        $env:SLINTFORGE_COMPONENT_QA_DIR = Join-Path $script:ForgeEvidence $hostName
        $process = Start-Process -FilePath $executable -ArgumentList '--components-smoke' -WorkingDirectory $script:ForgeRoot -WindowStyle Hidden -PassThru -RedirectStandardOutput "$script:ForgeEvidence/$hostName.log" -RedirectStandardError "$script:ForgeEvidence/$hostName-errors.log"
        $running += @{ name=$hostName; process=$process; started=[DateTime]::UtcNow }
    }
    foreach ($entry in $running) {
        $process = $entry.process
        while (!$process.WaitForExit(1000)) {
            if (([DateTime]::UtcNow - $entry.started).TotalSeconds -gt 1200) { throw "$($entry.name) 旧组件回归超时" }
        }
        $process.Refresh()
        if ($process.ExitCode -ne 0) { throw "$($entry.name) 旧组件回归失败：$($process.ExitCode)" }
        $log = Get-Content -Raw -LiteralPath "$script:ForgeEvidence/$($entry.name).log"
        $match = [regex]::Match($log, 'COMPONENTS_RENDER_OK deferred=0 frames=(\d+)')
        $screenshots = @(Get-ChildItem -LiteralPath "$script:ForgeEvidence/$($entry.name)" -Filter '*.png').Count
        if (!$match.Success -or $screenshots -ne [int]$match.Groups[1].Value) { throw '旧组件截图数量与完成记录不一致' }
        $results += @{ host=$entry.name; passed=$true; frames=$screenshots }
        Write-Host "FLOW_LEGACY_OK host=$($entry.name) frames=$screenshots"
    }
    $fingerprint = & $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
    Write-ForgeJson ([ordered]@{ passed=$true; renderer='winit-software'; excluded='flow-'; hosts=$results; source_fingerprint=$fingerprint; completed=(Get-Date).ToString('o') }) (Join-Path $script:ForgeEvidence 'result.json')
} finally {
    foreach ($entry in $running) { if (!$entry.process.HasExited) { Stop-Process -Id $entry.process.Id } }
    foreach ($name in $savedOptions.Keys) { [Environment]::SetEnvironmentVariable($name, $savedOptions[$name], 'Process') }
}
