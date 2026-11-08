# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$Installer,
    [Parameter(Mandatory=$true)][string]$EvidenceDirectory,
    [string]$ReferencePayload,
    [switch]$SandboxGuest,
    [switch]$Interactive,
    [switch]$InstallRuntimeForAcceptance
)
$ErrorActionPreference = 'Stop'
$evidence = [IO.Path]::GetFullPath($EvidenceDirectory)
[IO.Directory]::CreateDirectory($evidence) | Out-Null
$installRoot = Join-Path $evidence '安装 测试'
$appId = '{64227F76-6CEB-4F1E-971F-67939FD826C8}_is1'
$registry = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\' + $appId
if (Test-Path -LiteralPath $registry) { throw '检测到已有 SlintForge 安装，验收不会覆盖现有安装' }
if (Test-Path -LiteralPath $installRoot) { throw '安装验收目录已存在，请使用新的证据目录' }
function Get-ForgeRuntimeSnapshot {
    $snapshot = [ordered]@{ compatible=$true; dlls=@() }
    foreach ($name in @('MSVCP140.dll','MSVCP140_ATOMIC_WAIT.dll','VCRUNTIME140.dll','VCRUNTIME140_1.dll')) {
        $path = Join-Path "$env:WINDIR/System32" $name
        $version = $null
        if (Test-Path -LiteralPath $path) { $version = (Get-Item -LiteralPath $path).VersionInfo.FileVersion }
        $compatible = $version -and ([version]([regex]::Match($version,'\d+\.\d+\.\d+\.\d+').Value)) -ge [version]'14.51.0.0'
        if (!$compatible) { $snapshot.compatible = $false }
        $snapshot.dlls += [ordered]@{ name=$name; version=$version }
    }
    return $snapshot
}
$results = [ordered]@{ passed=$false; environment=$(if ($SandboxGuest) { 'windows-sandbox' } else { 'development-host' }); clean_windows_machine=[bool]$SandboxGuest; interactive=[bool]$Interactive; install_directory=$installRoot; registered=$false; runtime_passed=$false; uninstalled=$false; install_exit_code=$null; uninstall_exit_code=$null; gallery_install_seconds=$null; runtime_before=(Get-ForgeRuntimeSnapshot); runtime_after_gallery_install=$null; runtime_unchanged=$false; launcher_checks=@(); manual_runtime_execution=$null; prerequisite_logs=@(); error=$null }
try {
    $arguments = @('/NORESTART',('/DIR="' + $installRoot + '"'),('/LOG="' + (Join-Path $evidence 'install.log') + '"'))
    $windowStyle = 'Hidden'
    if ($Interactive) { $windowStyle = 'Normal' } else { $arguments += @('/VERYSILENT','/SUPPRESSMSGBOXES') }
    $watch = [Diagnostics.Stopwatch]::StartNew()
    $process = Start-Process -FilePath (Resolve-Path -LiteralPath $Installer).Path -ArgumentList $arguments -WindowStyle $windowStyle -Wait -PassThru
    $watch.Stop(); $results.gallery_install_seconds = $watch.Elapsed.TotalSeconds
    $results.install_exit_code = $process.ExitCode
    # Gallery 安装器只能复制运行库安装文件，不允许执行或更新系统运行库。
    $setupLog = [IO.File]::ReadAllText((Join-Path $evidence 'install.log'))
    if ($setupLog -match 'SLINTFORGE_RUNTIME_INSTALL_BEGIN|Installing Visual C\+\+ Runtime') { throw 'Gallery 安装器仍在执行运行库安装' }
    if ($process.ExitCode -ne 0) { throw "安装失败：$($process.ExitCode)" }
    if (!(Test-Path -LiteralPath $registry)) { throw '缺少卸载注册项' }
    $results.registered = $true
    if ($ReferencePayload) {
        $reference = (Resolve-Path -LiteralPath $ReferencePayload).Path
        foreach ($file in Get-ChildItem -LiteralPath $reference -Recurse -File) {
            $relative = $file.FullName.Substring($reference.Length + 1)
            $installed = Join-Path $installRoot $relative
            if (!(Test-Path -LiteralPath $installed) -or (Get-FileHash -LiteralPath $installed).Hash -ne (Get-FileHash -LiteralPath $file.FullName).Hash) { throw "安装内容不一致：$relative" }
        }
        $results['payload_matches_reference'] = $true
    }
    $results.runtime_after_gallery_install = Get-ForgeRuntimeSnapshot
    $results.runtime_unchanged = ($results.runtime_before | ConvertTo-Json -Depth 10 -Compress) -eq ($results.runtime_after_gallery_install | ConvertTo-Json -Depth 10 -Compress)
    if (!$results.runtime_unchanged) { throw 'Gallery 安装改变了系统运行库' }
    foreach ($name in @('prerequisites/vc_redist.x64.exe','prerequisites/RUNTIME-README.txt','Gallery-Rust.exe','Gallery-Cpp.exe','SlintForge-Docs.exe','docs/site/index.html','docs/site/en/index.html')) {
        if (!(Test-Path -LiteralPath (Join-Path $installRoot $name))) { throw "安装目录缺少手动运行库文件或启动入口：$name" }
    }
    foreach ($hostName in @('Rust','Cpp')) {
        $launcher = Join-Path $installRoot "Gallery-$hostName.exe"
        $check = Start-Process -FilePath $launcher -ArgumentList '/check' -WindowStyle Hidden -Wait -PassThru
        $expectedCode = $(if ($results.runtime_before.compatible) { 0 } else { 2 })
        $results.launcher_checks += [ordered]@{ host=$hostName; exit_code=$check.ExitCode; expected=$expectedCode }
        if ($check.ExitCode -ne $expectedCode) { throw "运行库启动检查不符合预期：$hostName / $($check.ExitCode)" }
    }
    $docsLauncher = Join-Path $installRoot 'SlintForge-Docs.exe'
    $docsCheck = Start-Process -FilePath $docsLauncher -ArgumentList '--check' -WindowStyle Hidden -Wait -PassThru
    $results.launcher_checks += [ordered]@{ host='Docs'; exit_code=$docsCheck.ExitCode; expected=0 }
    if ($docsCheck.ExitCode -ne 0) { throw "文档启动器自检失败：$($docsCheck.ExitCode)" }
    if (!$results.runtime_before.compatible -and $InstallRuntimeForAcceptance) {
        # 显式验收阶段模拟用户从软件目录单独安装；此操作不属于 Gallery 安装器。
        $logDirectory = Join-Path $evidence 'manual-runtime'
        [IO.Directory]::CreateDirectory($logDirectory) | Out-Null
        $runtimeLog = Join-Path $logDirectory 'vc-redist.log'
        $watch = [Diagnostics.Stopwatch]::StartNew()
        $manual = Start-Process -FilePath (Join-Path $installRoot 'prerequisites/vc_redist.x64.exe') -ArgumentList @('/install','/quiet','/norestart','/log',('"'+$runtimeLog+'"')) -WindowStyle Hidden -Wait -PassThru
        $watch.Stop()
        $results.manual_runtime_execution = [ordered]@{ phase='explicit-acceptance-step'; source='installed-application-prerequisites'; exit_code=$manual.ExitCode; seconds=$watch.Elapsed.TotalSeconds }
        $results.prerequisite_logs = @(Get-ChildItem -LiteralPath $logDirectory -File | ForEach-Object { 'manual-runtime/' + $_.Name })
        if ($manual.ExitCode -notin @(0,3010,1638) -or !(Get-ForgeRuntimeSnapshot).compatible) { throw "单独运行库安装失败：$($manual.ExitCode)" }
    }
    if (!(Get-ForgeRuntimeSnapshot).compatible) { throw '运行库缺失：请先从安装目录手动安装，或为干净来宾验收显式启用 InstallRuntimeForAcceptance' }
    if ($results.manual_runtime_execution) {
        foreach ($hostName in @('Rust','Cpp')) {
            $launcher = Join-Path $installRoot "Gallery-$hostName.exe"
            $check = Start-Process -FilePath $launcher -ArgumentList '/check' -WindowStyle Hidden -Wait -PassThru
            $results.launcher_checks += [ordered]@{ host=$hostName; phase='after-manual-runtime'; exit_code=$check.ExitCode; expected=0 }
            if ($check.ExitCode -ne 0) { throw "手动安装后启动检查仍失败：$hostName" }
        }
    }
    & "$PSScriptRoot/Test-Runtime.ps1" -PackageDirectory $installRoot -EvidenceDirectory "$evidence/installed-runtime" -SandboxGuest:$SandboxGuest
    $results.runtime_passed = [bool](Get-Content -Raw -LiteralPath "$evidence/installed-runtime/runtime-result.json" | ConvertFrom-Json).passed
} catch {
    $results.error = $_.Exception.Message
    throw
} finally {
    # 测试只卸载本轮唯一目标，绝不卸载或移动原有用户安装。
    $uninstaller = Join-Path $installRoot 'unins000.exe'
    if (Test-Path -LiteralPath $uninstaller) {
        $uninstallArgs = @('/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART',('/LOG="' + (Join-Path $evidence 'uninstall.log') + '"'))
        $process = Start-Process -FilePath $uninstaller -ArgumentList $uninstallArgs -WindowStyle Hidden -Wait -PassThru
        $results.uninstall_exit_code = $process.ExitCode
        $results.uninstalled = !(Test-Path -LiteralPath $uninstaller) -and !(Test-Path -LiteralPath $registry)
    }
    $results.passed = $results.registered -and $results.runtime_unchanged -and $results.runtime_passed -and $results.uninstalled -and $results.uninstall_exit_code -eq 0
    [IO.File]::WriteAllText("$evidence/installer-result.json", ($results | ConvertTo-Json -Depth 10), [Text.UTF8Encoding]::new($false))
}
if (!$results.passed) { throw '安装、运行或卸载验收未通过，请查看 installer-result.json' }
Write-Host "INSTALLER_OK evidence=$evidence"
