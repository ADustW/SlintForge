[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$VerificationReport,
    [string]$SlintSdk = (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0'),
    [string]$Python = 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe',
    [string]$CMake = 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe',
    [string]$ToolchainEnvironment = 'D:/.clion-env/qt-6.8.3-msvc.bat',
    [string]$InnoCompiler = "$env:LOCALAPPDATA/Programs/Inno Setup 6/ISCC.exe",
    [string]$VcRedist = 'C:/Program Files/Microsoft Visual Studio/18/Community/VC/Redist/MSVC/v145/vc_redist.x64.exe',
    [int]$Jobs = 2,
    [switch]$Online
)
. (Join-Path $PSScriptRoot 'Common.ps1')
$stamp = [TimeZoneInfo]::ConvertTimeBySystemTimeZoneId([DateTime]::UtcNow, 'China Standard Time').ToString('yyyyMMdd_HHmmss')
$distRoot = [IO.Path]::GetFullPath((Join-Path $script:ForgeRoot 'dist'))
$buildRoot = [IO.Path]::GetFullPath((Join-Path $script:ForgeRoot 'build'))
$stage = Join-Path $buildRoot ('release-staging-' + $stamp)
$destination = Join-Path $distRoot ('dist_SlintForge_' + $stamp)
foreach ($candidate in @($stage,$destination)) {
    $expected = $(if ($candidate -eq $stage) { $buildRoot } else { $distRoot })
    if (![IO.Path]::GetFullPath($candidate).StartsWith($expected + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw '发布目标越界' }
    if (Test-Path -LiteralPath $candidate) { throw "发布目录已存在：$candidate" }
}
$script:ForgeEvidence = Join-Path $buildRoot ('release-evidence-' + $stamp)
[IO.Directory]::CreateDirectory($script:ForgeEvidence) | Out-Null
Initialize-ForgeEnvironment $SlintSdk $Python $CMake $ToolchainEnvironment
$report = Get-Content -Raw -LiteralPath $VerificationReport | ConvertFrom-Json
if (!$report.passed -or $report.mode -ne 'Full' -or $report.version -ne $script:ForgeVersion) { throw '发布需要当前版本的完整验收报告' }
$fingerprint = & $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
if ($LASTEXITCODE -ne 0) { throw '发布源码指纹生成失败' }
$verifiedFingerprint = & $script:ForgePython "$PSScriptRoot/source_snapshot.py" --verification-fingerprint
if ($LASTEXITCODE -ne 0 -or $verifiedFingerprint -ne $report.verification_fingerprint) { throw '完整验收后应用或检查入口发生变化，需要重新验收' }
foreach ($path in @($InnoCompiler,$VcRedist)) { if (!(Test-Path -LiteralPath $path)) { throw "打包依赖不存在：$path" } }
$cargoFlags = @('--workspace','--locked','--release'); if (!$Online) { $cargoFlags += '--offline' }
Invoke-ForgeCheck 'rust-build-release' 'cargo' (@('build') + $cargoFlags)
$cpp = Join-Path $script:ForgeRoot 'build/cpp'
Invoke-ForgeCheck 'cpp-build-release' $script:ForgeCMake @('--build',$cpp,'--config','Release','--parallel',"$Jobs")
$payload = Join-Path $stage "SlintForge-$script:ForgeVersion"
foreach ($folder in @('rust','cpp','licenses','prerequisites','sdk/cpp')) { [IO.Directory]::CreateDirectory((Join-Path $payload $folder)) | Out-Null }
foreach ($configuration in @('Debug','Release')) { Invoke-ForgeCheck "cpp-install-$configuration" $script:ForgeCMake @('--install',$cpp,'--config',$configuration,'--prefix',"$payload/sdk/cpp") }
Copy-Item -LiteralPath "$script:ForgeRoot/target/release/slintforge-gallery-rust.exe" -Destination "$payload/rust/"
Copy-Item -LiteralPath "$cpp/Gallery/cpp/Release/slintforge_gallery_cpp.exe" -Destination "$payload/cpp/"
Copy-Item -LiteralPath "$script:ForgeSdk/lib/slint_cpp.dll" -Destination "$payload/cpp/"
Copy-Item -LiteralPath $VcRedist -Destination "$payload/prerequisites/vc_redist.x64.exe"
Copy-Item -LiteralPath "$PSScriptRoot/RUNTIME-README.txt" -Destination "$payload/prerequisites/"
# 交付计算 DLL 和公共接口源码，不复制 Cargo 的中间库与元数据。
$flags = @('build','-p','slintforge-runtime','--locked','--release'); if (!$Online) { $flags += '--offline' }
Invoke-ForgeCheck 'rust-runtime-release' 'cargo' $flags
$compilerBuild = Join-Path $script:ForgeRoot 'build/ui-compiler-release'
$env:CARGO_TARGET_DIR = $compilerBuild
try {
    $flags = @('build','--manifest-path',"$PSScriptRoot/ui-compiler/Cargo.toml",'--locked','--release'); if (!$Online) { $flags += '--offline' }
    Invoke-ForgeCheck 'rust-ui-compiler' 'cargo' $flags
} finally { $env:CARGO_TARGET_DIR = $null }
Invoke-ForgeCheck 'binary-sdk-payload' $script:ForgePython @("$PSScriptRoot/release_payload.py",'--payload',$payload,'--runtime',"$script:ForgeRoot/target/release/slintforge_runtime.dll",'--compiler',"$compilerBuild/release/slintforge-ui-compiler.exe",'--slint-sdk',$script:ForgeSdk,'--receipt',"$script:ForgeEvidence/content-receipt.json")
$launcherTools = Join-Path $script:ForgeEvidence 'launcher'
[IO.Directory]::CreateDirectory($launcherTools) | Out-Null
$launcherRc = Join-Path $launcherTools 'launcher.rc'
$launcherRes = Join-Path $launcherTools 'launcher.res'
[IO.File]::WriteAllText($launcherRc, ('1 ICON "' + $script:ForgeRoot.Replace('\','/') + '/Gallery/ui/assets/brand/slintforge-gallery.ico"'), [Text.UTF8Encoding]::new($false))
Invoke-ForgeCheck 'launcher-icon' 'rc.exe' @('/nologo',('/fo'+$launcherRes),$launcherRc)
Invoke-ForgeCheck 'launcher-build' 'rustc' @('--edition','2021','--target','x86_64-pc-windows-msvc','-C','opt-level=2','-C','target-feature=+crt-static','-C','strip=symbols','-C',('link-arg='+$launcherRes),"$PSScriptRoot/Gallery-Launcher.rs",'-o',"$launcherTools/Gallery-Launcher.exe")
Invoke-ForgeCheck 'launcher-dependencies' 'dumpbin.exe' @('/dependents',"$launcherTools/Gallery-Launcher.exe")
if ([IO.File]::ReadAllText("$script:ForgeEvidence/launcher-dependencies.log") -match '(?i)\b(?:msvcp\d\S*|vcruntime\d\S*|api-ms-win-crt\S*)\.dll\b') { throw '启动入口仍依赖动态 Visual C++ 运行库' }
foreach ($hostName in @('Rust','Cpp')) { Copy-Item -LiteralPath "$launcherTools/Gallery-Launcher.exe" -Destination "$payload/Gallery-$hostName.exe" }
Copy-Item -LiteralPath "$script:ForgeRoot/ui/assets/fonts/LICENSE-FluentSystemIcons.txt" -Destination "$payload/licenses/"
Copy-Item -LiteralPath "$script:ForgeRoot/third_party/qrcodegen/LICENSE.txt" -Destination "$payload/licenses/LICENSE-qrcodegen.txt"
Invoke-ForgeCheck 'content-final' $script:ForgePython @("$PSScriptRoot/release_payload.py",'--payload',$payload,'--receipt',"$script:ForgeEvidence/content-receipt.json")
$consumerArguments = @('-NoProfile','-File',"$PSScriptRoot/Test-BinaryConsumers.ps1",'-PackageDirectory',$payload,'-EvidenceDirectory',"$script:ForgeEvidence/consumers",'-CMake',$script:ForgeCMake)
if ($Online) { $consumerArguments += '-Online' }
Invoke-ForgeCheck 'external-binary-consumers' 'pwsh' $consumerArguments
Invoke-ForgeCheck 'portable-runtime' 'powershell.exe' @('-NoProfile','-ExecutionPolicy','Bypass','-File',"$PSScriptRoot/Test-Runtime.ps1",'-PackageDirectory',$payload,'-EvidenceDirectory',"$script:ForgeEvidence/portable-runtime")
Invoke-ForgeCheck 'installer-build' $InnoCompiler @("/DForgeVersion=$script:ForgeVersion","/DForgePayload=$payload","/DForgeOutput=$stage","/DForgeIcon=$script:ForgeRoot/Gallery/ui/assets/brand/slintforge-gallery.ico","$PSScriptRoot/SlintForge.iss")
$installer = Join-Path $stage "SlintForge-Gallery-$script:ForgeVersion-Setup.exe"
Invoke-ForgeCheck 'installer-runtime' 'powershell.exe' @('-NoProfile','-ExecutionPolicy','Bypass','-File',"$PSScriptRoot/Test-Installer.ps1",'-Installer',$installer,'-ReferencePayload',$payload,'-EvidenceDirectory',"$script:ForgeEvidence/installer")
$entries = @(Get-ChildItem -LiteralPath $stage)
if ($entries.Count -ne 2 -or @($entries | Where-Object PSIsContainer).Count -ne 1 -or @($entries | Where-Object Extension -eq '.exe').Count -ne 1) { throw '发布必须仅包含一个未压缩目录和一个安装包' }
$finishedFingerprint = & $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
if ($LASTEXITCODE -ne 0 -or $finishedFingerprint -ne $fingerprint) { throw '验收期间源码发生变化，不发布本轮产物' }
$sha = (Get-FileHash -LiteralPath $installer -Algorithm SHA256).Hash.ToLowerInvariant()
$content = Get-Content -Raw -LiteralPath "$script:ForgeEvidence/content-receipt.json" | ConvertFrom-Json
Write-ForgeJson ([ordered]@{passed=$true; version=$script:ForgeVersion; source_fingerprint=$fingerprint; verification_fingerprint=$verifiedFingerprint; application_verification=(Resolve-Path -LiteralPath $VerificationReport).Path; directory=$destination; payload="SlintForge-$script:ForgeVersion"; installer=(Split-Path $installer -Leaf); installer_sha256=$sha; core_source_files=$content.core_source_files; interface_source_files=$content.interface_source_files; rust_sdk_bytes=$content.rust_sdk_bytes; rust_sdk_mib=$content.rust_sdk_mib; precompiled_rust_dependency_files=$content.precompiled_rust_dependency_files; archives=0; clean_windows_acceptance='not-run'; evidence=$script:ForgeEvidence}) "$script:ForgeEvidence/manifest.json"
# 两条绝对路径均已确认位于工作区的 build/dist 子目录，原子移动保留历史发布。
Move-Item -LiteralPath $stage -Destination $destination
Write-Host "PACKAGE_OK directory=$destination evidence=$script:ForgeEvidence sha256=$sha"
