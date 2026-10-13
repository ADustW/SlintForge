[CmdletBinding()]
param([string]$EvidenceDirectory, [switch]$SkipBuild)
. (Join-Path $PSScriptRoot 'Common.ps1')
Initialize-ForgeEnvironment (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0') 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe' 'D:/.clion-env/qt-6.8.3-msvc.bat'
if (!$EvidenceDirectory) { $EvidenceDirectory = Join-Path $script:ForgeRoot ('build/flow-verify-' + (Get-Date -Format 'yyyyMMdd-HHmmss')) }
$script:ForgeEvidence = [IO.Path]::GetFullPath($EvidenceDirectory)
[IO.Directory]::CreateDirectory($script:ForgeEvidence) | Out-Null
$env:WGPU_BACKEND = 'dx12'
if (!$SkipBuild) {
    Invoke-ForgeCheck 'cpp-release' $script:ForgeCMake @('--build','build/cpp','--config','Release','--parallel','2','--target','slintforge_flow_tests','slintforge_flow_example')
    Invoke-ForgeCheck 'rust-release' 'cargo' @('build','-p','slintforge-runtime','-p','slintforge-flow-example','--release','--offline','--locked','-j','2')
}
Invoke-ForgeCheck 'flow-core' "$script:ForgeRoot/build/cpp/Release/slintforge_flow_tests.exe" @()
Invoke-ForgeCheck 'flow-bindings' $script:ForgePython @('tools/generate_flow_bindings.py','--check')
$executables = @{
    cpp = "$script:ForgeRoot/build/cpp/examples/flow-cpp/Release/slintforge_flow_example.exe"
    rust = "$script:ForgeRoot/target/release/slintforge-flow-example.exe"
}
foreach ($hostName in @('cpp','rust')) {
    foreach ($renderer in @('winit-skia','winit-skia-software')) {
        foreach ($scale in @('1','1.5','2')) {
            $env:SLINT_BACKEND = $renderer; $env:SLINT_SCALE_FACTOR = $scale
            $label = "$hostName-$renderer-dpi-$scale"
            $env:SLINTFORGE_FLOW_QA_DIR = Join-Path $script:ForgeEvidence $label
            Invoke-ForgeCheck $label $executables[$hostName] @('--smoke')
        }
    }
}
$env:SLINT_SCALE_FACTOR = '1'
foreach ($hostName in @('cpp','rust')) {
    # 官方 C++ DLL 的 DX12 路径不通知帧回调，使用同一 Skia 的 OpenGL 路径采样。
    $env:SLINT_BACKEND = if ($hostName -eq 'cpp') { 'winit-skia-opengl' } else { 'winit-skia' }
    $env:SLINTFORGE_FLOW_QA_DIR = Join-Path $script:ForgeEvidence "$hostName-performance"
    foreach ($scenario in @('standard','mixed')) {
        $flags = @('--benchmark'); if ($scenario -eq 'mixed') { $flags += '--complex' }
        Invoke-ForgeCheck "$hostName-performance-$scenario" $executables[$hostName] $flags
        $report = Get-Content -Raw (Join-Path $env:SLINTFORGE_FLOW_QA_DIR "performance-$scenario.json") | ConvertFrom-Json
        if (!$report.measurement_complete) { throw "$hostName/$scenario 帧样本不足" }
        if ($scenario -eq 'standard' -and !$report.target_met) { throw "$hostName 标准大图未达到性能目标" }
    }
}
$cpu = Get-CimInstance Win32_Processor | Select-Object Name,NumberOfCores,NumberOfLogicalProcessors
$gpu = Get-CimInstance Win32_VideoController | Select-Object Name,DriverVersion
$machine = Get-CimInstance Win32_ComputerSystem | Select-Object TotalPhysicalMemory
$fingerprint = & $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
Write-ForgeJson ([ordered]@{ passed=$true; cpu=$cpu; gpu=$gpu; memory=$machine; checks=$script:ForgeChecks; source_fingerprint=$fingerprint; completed=(Get-Date).ToString('o'); cpp_dx12_frame_timing='requires external ETW capture; OpenGL is measured separately' }) (Join-Path $script:ForgeEvidence 'result.json')
Write-Host "FLOW_VERIFICATION_OK $script:ForgeEvidence"
