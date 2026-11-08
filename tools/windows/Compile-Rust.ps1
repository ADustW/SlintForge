# Copyright © 2026 陕西禾又科技有限责任公司
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$SourceFile,
    [Parameter(Mandatory=$true)][string]$UiFile,
    [Parameter(Mandatory=$true)][string]$OutputFile,
    [switch]$Offline,
    [switch]$Flow,
    [ValidateSet('Debug','Release')][string]$Configuration = 'Release',
    [string]$TargetDirectory,
    [int]$Jobs = 2
)
$ErrorActionPreference = 'Stop'
# 接口源码由客户工具链编译，计算核心始终使用包内 DLL。
$sdk = (Resolve-Path -LiteralPath $PSScriptRoot).Path
$metadata = Get-Content -Raw -LiteralPath (Join-Path $sdk 'sdk.json') | ConvertFrom-Json
$version = & rustc --version
if ($LASTEXITCODE -ne 0 -or $version -notmatch '^rustc (\d+)\.(\d+)\.') { throw '无法确定客户 Rust 编译器版本' }
if ([int]$Matches[1] -lt 1 -or ([int]$Matches[1] -eq 1 -and [int]$Matches[2] -lt 98)) { throw "Rust 接口最低要求 1.98.0，当前为 $version" }
if ($metadata.abi_version -ne 2) { throw 'Rust SDK 的 ABI 清单不兼容' }
$source = (Resolve-Path -LiteralPath $SourceFile).Path
$ui = (Resolve-Path -LiteralPath $UiFile).Path
$output = [IO.Path]::GetFullPath($OutputFile)
$destinationDirectory = Split-Path $output -Parent
[IO.Directory]::CreateDirectory($destinationDirectory) | Out-Null
$project = Join-Path $destinationDirectory ('.slintforge-build/' + [IO.Path]::GetFileNameWithoutExtension($output))
[IO.Directory]::CreateDirectory($project) | Out-Null
# 路径转义为 TOML 字符串，不拼接可执行 shell 文本。
function ConvertTo-ForgeTomlString([string]$Value) { return ConvertTo-Json -InputObject $Value -Compress }
function Write-ForgeChangedFile([string]$Path,[string]$Text) {
    if (!(Test-Path -LiteralPath $Path) -or [IO.File]::ReadAllText($Path) -ne $Text) { [IO.File]::WriteAllText($Path,$Text,[Text.UTF8Encoding]::new($false)) }
}
$sourceLiteral = ConvertTo-ForgeTomlString $source.Replace('\','/')
$sdkLiteral = ConvertTo-ForgeTomlString $sdk.Replace('\','/')
$helperLiteral = ConvertTo-ForgeTomlString ([IO.Path]::GetFullPath((Join-Path $sdk '../rust-build')).Replace('\','/'))
$uiLiteral = ConvertTo-ForgeTomlString $ui.Replace('\','/')
$flowSlintFeatures = if ($Flow) { '["raw-window-handle-06", "renderer-skia"]' } else { '["raw-window-handle-06"]' }
$flowSdkFeatures = if ($Flow) { ', features = ["flow"]' } else { '' }
$manifest = @"
[package]
name = "slintforge_consumer"
version = "0.0.0"
edition = "2021"
build = "build.rs"
[workspace]
[[bin]]
name = "slintforge_consumer"
path = $sourceLiteral
[dependencies]
slint = { version = "=1.18.0", features = $flowSlintFeatures }
slintforge = { path = $sdkLiteral$flowSdkFeatures }
[build-dependencies]
slintforge-build = { path = $helperLiteral }
[profile.release]
opt-level = 2
"@
Write-ForgeChangedFile (Join-Path $project 'Cargo.toml') $manifest
$buildScript = "fn main() { slintforge_build::compile_ui($uiLiteral).expect(`"Slint 界面编译失败`" ); }`n"
Write-ForgeChangedFile (Join-Path $project 'build.rs') $buildScript
if (!$TargetDirectory) { $TargetDirectory = Join-Path $env:LOCALAPPDATA ('SlintForge/CargoTargets/sdk-' + $metadata.version + '-rust-' + $Matches[1] + '.' + $Matches[2]) }
$target = [IO.Path]::GetFullPath($TargetDirectory)
$arguments = @('build','--manifest-path',(Join-Path $project 'Cargo.toml'),'--target-dir',$target,'--jobs',"$Jobs",'--message-format=json')
if ($Configuration -eq 'Release') { $arguments += '--release' }
if ($Offline) { $arguments += '--offline' }
$lock = Join-Path $project 'Cargo.lock'
if (Test-Path -LiteralPath $lock) { $arguments += '--locked' }
$log = Join-Path $project 'build.log'
$watch = [Diagnostics.Stopwatch]::StartNew()
& cargo @arguments *> $log
$code = $LASTEXITCODE
$watch.Stop()
if ($code -ne 0) { Get-Content -LiteralPath $log -Tail 45 | Write-Host; throw "应用 Cargo 编译失败，退出码 $code；日志：$log" }
$records = @(Get-Content -LiteralPath $log | Where-Object { $_.StartsWith('{') } | ForEach-Object { $_ | ConvertFrom-Json })
$artifact = $records | Where-Object { $_.reason -eq 'compiler-artifact' -and $_.target.name -eq 'slintforge_consumer' -and $_.executable } | Select-Object -Last 1
if (!$artifact) { throw 'Cargo 未报告应用可执行文件' }
Copy-Item -LiteralPath $artifact.executable -Destination $output -Force
Copy-Item -LiteralPath (Join-Path $sdk $metadata.runtime) -Destination (Join-Path $destinationDirectory 'slintforge_runtime.dll') -Force
$receipt = [ordered]@{ passed=$true; rust=$version; abi_version=2; configuration=$Configuration; seconds=$watch.Elapsed.TotalSeconds; output=$output; runtime=(Join-Path $destinationDirectory 'slintforge_runtime.dll'); target_directory=$target; offline=[bool]$Offline; log=$log }
[IO.File]::WriteAllText((Join-Path $project 'result.json'),($receipt | ConvertTo-Json),[Text.UTF8Encoding]::new($false))
Write-Host "RUST_CONSUMER_BUILD_OK output=$output seconds=$([math]::Round($watch.Elapsed.TotalSeconds,3)) rust=$version"
