# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

$ErrorActionPreference = 'Stop'
$release = 'C:\SlintForgeRelease'
$evidence = 'C:\SlintForgeEvidence'
$working = 'C:\SlintForgeQA'
[IO.Directory]::CreateDirectory($working) | Out-Null
$result = [ordered]@{ passed=$false; environment='windows-sandbox'; os=[Environment]::OSVersion.Version.ToString(); toolchains=@(); completed=$null }
try {
    # 配置文件仅把发布目录映射进来；来宾中不得存在开发工具链。
    $result.toolchains = @(Get-Command cargo,rustc,cl,cmake -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source)
    if ($result.toolchains.Count -ne 0) { throw '来宾存在开发工具链，不能认定为干净环境验收' }
    $installer = @(Get-ChildItem -LiteralPath $release -Filter '*-Setup.exe')
    $portable = @(Get-ChildItem -LiteralPath $release -Directory | Where-Object { $_.Name -match '^SlintForge-\d+\.\d+\.\d+$' })
    $archives = @(Get-ChildItem -LiteralPath $release -File -Filter 'SlintForge-Gallery-*.zip')
    if ($installer.Count -ne 1 -or ($portable.Count + $archives.Count) -ne 1) { throw '发布目录必须包含唯一安装程序和一份便携载荷' }
    # 当前发布直接复制未压缩载荷；保留对历史 Gallery ZIP 的验收兼容。
    if ($portable.Count -eq 1) {
        Copy-Item -LiteralPath $portable[0].FullName -Destination $working -Recurse
        $payload = Join-Path $working $portable[0].Name
    } else {
        Add-Type -AssemblyName System.IO.Compression.FileSystem
        [IO.Compression.ZipFile]::ExtractToDirectory($archives[0].FullName, $working)
        $extracted = @(Get-ChildItem -LiteralPath $working -Directory -Filter 'SlintForge-Gallery-*')
        if ($extracted.Count -ne 1) { throw '历史归档中没有唯一 Gallery 目录' }
        $payload = $extracted[0].FullName
    }
    $result.installer_sha256 = (Get-FileHash -LiteralPath $installer[0].FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    $result.portable_directory = $payload
    # 先证明 Gallery 安装没有改变运行库，再从安装目录单独执行运行库安装验收。
    & "$PSScriptRoot/Test-Installer.ps1" -Installer $installer[0].FullName -ReferencePayload $payload -EvidenceDirectory "$evidence/installer" -SandboxGuest -InstallRuntimeForAcceptance
    & "$PSScriptRoot/Test-Runtime.ps1" -PackageDirectory $payload -EvidenceDirectory "$evidence/portable" -SandboxGuest
    $result.passed = $true
} catch {
    $result.error = $_.Exception.Message
} finally {
    $result.completed = (Get-Date).ToString('o')
    [IO.File]::WriteAllText("$evidence/sandbox-result.json", ($result | ConvertTo-Json -Depth 10), [Text.UTF8Encoding]::new($false))
}
if (!$result.passed) { throw $result.error }
Write-Host "SANDBOX_OK evidence=$evidence"
