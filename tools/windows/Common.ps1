# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$script:ForgeRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$script:ForgeChecks = [Collections.Generic.List[object]]::new()

function Write-ForgeJson($Value, [string]$Path) {
    [IO.Directory]::CreateDirectory((Split-Path $Path -Parent)) | Out-Null
    [IO.File]::WriteAllText($Path, ($Value | ConvertTo-Json -Depth 15), [Text.UTF8Encoding]::new($false))
}

function Initialize-ForgeEnvironment([string]$SlintSdk, [string]$Python, [string]$CMake, [string]$ToolchainEnvironment) {
    foreach ($path in @($ToolchainEnvironment, $Python, $CMake, (Join-Path $SlintSdk 'bin/slint-compiler.exe'), (Join-Path $SlintSdk 'lib/slint_cpp.dll'))) {
        if (!(Test-Path -LiteralPath $path)) { throw "构建依赖不存在：$path" }
    }
    # 仅导入用户指定的 CLion 环境，不使用系统默认 Qt/CMake Kit。
    if ($ToolchainEnvironment -match '["\r\n&|<>]') { throw '工具链脚本路径含不支持的字符' }
    $environment = & $env:ComSpec /d /c "call `"$ToolchainEnvironment`" >nul && set"
    if ($LASTEXITCODE -ne 0) { throw 'CLion 工具链环境加载失败' }
    foreach ($line in $environment) {
        if ($line -match '^([^=]+)=(.*)$') { [Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], 'Process') }
    }
    $script:ForgeSdk = (Resolve-Path -LiteralPath $SlintSdk).Path
    $script:ForgePython = (Resolve-Path -LiteralPath $Python).Path
    $script:ForgeCMake = (Resolve-Path -LiteralPath $CMake).Path
    $script:ForgeCTest = Join-Path (Split-Path $script:ForgeCMake) 'ctest.exe'
    $env:PATH = (Join-Path $script:ForgeSdk 'lib') + ';' + $env:PATH
    $env:SLINT_BACKEND = 'winit-software'
    $env:SLINT_STYLE = 'fluent'
    $version = & rustc --version
    if ($LASTEXITCODE -ne 0 -or $version -notmatch '^rustc 1\.98\.0 ') { throw "需要 Rust 1.98.0：$version" }
    $script:ForgeVersion = [regex]::Match((Get-Content -Raw -LiteralPath (Join-Path $script:ForgeRoot 'Cargo.toml')), 'version = "([^"]+)"').Groups[1].Value
    if ((Get-Content -Raw (Join-Path $script:ForgeRoot 'CMakeLists.txt')) -notmatch ('VERSION ' + [regex]::Escape($script:ForgeVersion) + ' ')) { throw 'Rust/C++ 版本不一致' }
    Write-Host "SlintForge $script:ForgeVersion · $version · SDK=$script:ForgeSdk"
}

function Invoke-ForgeCheck([string]$Name, [string]$Executable, [string[]]$Arguments, [string]$WorkingDirectory = $script:ForgeRoot) {
    Write-Host "检查：$Name"
    $log = Join-Path $script:ForgeEvidence "$Name.log"
    $watch = [Diagnostics.Stopwatch]::StartNew()
    Push-Location -LiteralPath $WorkingDirectory
    $exitCode = -1
    try {
        # 保存实际参数数组，避免日志里的命令字符串被误当成可执行脚本。
        & $Executable @Arguments *> $log
        $exitCode = $LASTEXITCODE
        if ($exitCode -ne 0) {
            Get-Content -LiteralPath $log -Tail 35 | Write-Host
            throw "$Name 失败，退出码 $exitCode；日志：$log"
        }
    } finally {
        Pop-Location
        $watch.Stop()
        $script:ForgeChecks.Add([ordered]@{ name=$Name; executable=$Executable; arguments=$Arguments; cwd=$WorkingDirectory; exit_code=$exitCode; seconds=$watch.Elapsed.TotalSeconds; log=$log; log_file="$Name.log" })
        Write-ForgeJson @($script:ForgeChecks.ToArray()) (Join-Path $script:ForgeEvidence 'checks.json')
    }
}

function Configure-ForgeCpp([string]$Directory, [string]$Source = $script:ForgeRoot, [string[]]$Extra = @()) {
    Invoke-ForgeCheck 'cpp-configure' $script:ForgeCMake (@('-S',$Source,'-B',$Directory,'-G','Visual Studio 18 2026','-A','x64',"-DSlint_DIR=$script:ForgeSdk/lib/cmake/Slint",'-DSLINT_STYLE=fluent') + $Extra)
}
