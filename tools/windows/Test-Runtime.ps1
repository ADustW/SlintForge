# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$PackageDirectory, [Parameter(Mandatory=$true)][string]$EvidenceDirectory, [switch]$SandboxGuest)
$ErrorActionPreference = 'Stop'
$package = (Resolve-Path -LiteralPath $PackageDirectory).Path
$evidence = [IO.Path]::GetFullPath($EvidenceDirectory)
[IO.Directory]::CreateDirectory($evidence) | Out-Null
$originalPath = $env:PATH
$originalAppData = $env:APPDATA
$originalBackend = $env:SLINT_BACKEND
$results = @()
try {
    # 运行环境仅保留系统路径，Slint DLL 必须从便携包或安装目录加载。
    $env:PATH = "$env:SystemRoot\System32;$env:SystemRoot"
    $env:APPDATA = Join-Path $evidence 'appdata'
    $env:SLINT_BACKEND = 'winit-software'
    foreach ($hostName in @('rust','cpp')) {
        $executable = Join-Path $package ($hostName + '/' + $(if ($hostName -eq 'rust') { 'slintforge-gallery-rust.exe' } else { 'slintforge_gallery_cpp.exe' }))
        foreach ($scenario in @('smoke','run-smoke','capture-smoke','default-run')) {
            $env:SLINT_BACKEND = $(if ($scenario -eq 'default-run') { $null } else { 'winit-software' })
            $argument = $(if ($scenario -eq 'default-run') { '--run-smoke' } else { "--$scenario" })
            $log = Join-Path $evidence "$hostName-$scenario.log"
            $errorLog = Join-Path $evidence "$hostName-$scenario.stderr.log"
            # PowerShell 5.1 会把原生 stderr 转成错误记录；按进程退出码判定测试结果。
            $process = Start-Process -FilePath $executable -ArgumentList $argument -WorkingDirectory $evidence -WindowStyle Hidden -RedirectStandardOutput $log -RedirectStandardError $errorLog -Wait -PassThru
            $code = $process.ExitCode
            $resourceErrors = @([regex]::Matches([IO.File]::ReadAllText($errorLog), '(?im)^.*Error loading (?:image|font).*$') | ForEach-Object { $_.Value })
            $results += [ordered]@{ host=$hostName; scenario=$scenario; renderer=$(if ($scenario -eq 'default-run') { 'default' } else { 'winit-software' }); exit_code=$code; executable=$executable; log=$log; stderr_log=$errorLog; resource_errors=$resourceErrors }
            if ($code -ne 0) { throw "运行包验收失败：$hostName/$scenario，退出码 $code" }
            if ($resourceErrors.Count -ne 0) { throw "运行包资源加载失败：$hostName/$scenario；日志：$errorLog" }
        }
    }
    $result = [ordered]@{ passed=$true; environment=$(if ($SandboxGuest) { 'windows-sandbox' } else { 'system-path-only-on-development-host' }); clean_windows_machine=[bool]$SandboxGuest; package=$package; results=$results }
} catch {
    $result = [ordered]@{ passed=$false; error=$_.Exception.Message; results=$results }
    throw
} finally {
    $env:PATH = $originalPath; $env:APPDATA = $originalAppData; $env:SLINT_BACKEND = $originalBackend
    [IO.File]::WriteAllText((Join-Path $evidence 'runtime-result.json'), ($result | ConvertTo-Json -Depth 10), [Text.UTF8Encoding]::new($false))
}
Write-Host "RUNTIME_OK package=$package evidence=$evidence"
