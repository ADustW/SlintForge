[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$ReleaseDirectory, [string]$OutputDirectory, [string]$MappedReleaseDirectory, [string]$EvidenceDirectory)
$ErrorActionPreference = 'Stop'
$release = (Resolve-Path -LiteralPath $ReleaseDirectory).Path
if (!$OutputDirectory) {
    $stamp = [TimeZoneInfo]::ConvertTimeBySystemTimeZoneId([DateTime]::UtcNow, 'China Standard Time').ToString('yyyyMMdd_HHmmss')
    $OutputDirectory = Join-Path $PSScriptRoot "../../build/sandbox-$stamp"
}
if (!$MappedReleaseDirectory) { $MappedReleaseDirectory = $release }
if (!$EvidenceDirectory) { $EvidenceDirectory = Join-Path $OutputDirectory 'results' }
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
$EvidenceDirectory = [IO.Path]::GetFullPath($EvidenceDirectory)
[IO.Directory]::CreateDirectory($OutputDirectory) | Out-Null
[IO.Directory]::CreateDirectory($EvidenceDirectory) | Out-Null
foreach ($scriptName in @('Sandbox-Guest.ps1','Test-Installer.ps1','Test-Runtime.ps1')) {
    $text = [IO.File]::ReadAllText((Join-Path $PSScriptRoot $scriptName))
    # Windows Sandbox 自带 Windows PowerShell 5.1，BOM 保证中文注释和字符串正确解码。
    [IO.File]::WriteAllText((Join-Path $OutputDirectory $scriptName), $text, [Text.UTF8Encoding]::new($true))
}
$releaseXml = [Security.SecurityElement]::Escape([IO.Path]::GetFullPath($MappedReleaseDirectory))
$scriptsXml = [Security.SecurityElement]::Escape($OutputDirectory)
$evidenceXml = [Security.SecurityElement]::Escape($EvidenceDirectory)
$xml = @"
<Configuration>
  <VGpu>Disable</VGpu>
  <Networking>Disable</Networking>
  <ClipboardRedirection>Disable</ClipboardRedirection>
  <MemoryInMB>8192</MemoryInMB>
  <MappedFolders>
    <MappedFolder><HostFolder>$releaseXml</HostFolder><SandboxFolder>C:\SlintForgeRelease</SandboxFolder><ReadOnly>true</ReadOnly></MappedFolder>
    <MappedFolder><HostFolder>$scriptsXml</HostFolder><SandboxFolder>C:\SlintForgeScripts</SandboxFolder><ReadOnly>true</ReadOnly></MappedFolder>
    <MappedFolder><HostFolder>$evidenceXml</HostFolder><SandboxFolder>C:\SlintForgeEvidence</SandboxFolder><ReadOnly>false</ReadOnly></MappedFolder>
  </MappedFolders>
  <LogonCommand><Command>powershell.exe -NoProfile -ExecutionPolicy Bypass -File C:\SlintForgeScripts\Sandbox-Guest.ps1</Command></LogonCommand>
</Configuration>
"@
[IO.File]::WriteAllText((Join-Path $OutputDirectory 'SlintForge.wsb'), $xml, [Text.UTF8Encoding]::new($false))
Write-Host "SANDBOX_CONFIG_OK path=$OutputDirectory/SlintForge.wsb"
