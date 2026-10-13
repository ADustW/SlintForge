[CmdletBinding()]
param(
    [ValidateSet('Quick','Full')][string]$Mode = 'Quick',
    [string]$SlintSdk = (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0'),
    [string]$Python = 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe',
    [string]$CMake = 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe',
    [string]$ToolchainEnvironment = 'D:/.clion-env/qt-6.8.3-msvc.bat',
    [string]$EvidenceDirectory,
    [int]$Jobs = 2,
    [switch]$Online
)
. (Join-Path $PSScriptRoot 'Common.ps1')
if (!$EvidenceDirectory) { $EvidenceDirectory = Join-Path $script:ForgeRoot ('build/verify-' + (Get-Date -Format 'yyyyMMdd-HHmmss')) }
$script:ForgeEvidence = [IO.Path]::GetFullPath($EvidenceDirectory)
[IO.Directory]::CreateDirectory($script:ForgeEvidence) | Out-Null
$cpp = Join-Path $script:ForgeRoot 'build/cpp'
$cargoFlags = @('--workspace','--locked'); if (!$Online) { $cargoFlags += '--offline' }
try {
    Initialize-ForgeEnvironment $SlintSdk $Python $CMake $ToolchainEnvironment
    $startedFingerprint = & $script:ForgePython "$PSScriptRoot/source_snapshot.py" --verification-fingerprint
    if ($LASTEXITCODE -ne 0) { throw '初始源码指纹生成失败' }
    Invoke-ForgeCheck 'rust-runtime-build' 'cargo' (@('build','-p','slintforge-runtime','--locked') + $(if (!$Online) { @('--offline') } else { @() }))
    Invoke-ForgeCheck 'runtime-ffi' $script:ForgePython @('tools/generate_runtime_ffi.py','--check')
    Invoke-ForgeCheck 'first-party-namespaces' $script:ForgePython @('tools/verify_first_party_namespaces.py')
    Invoke-ForgeCheck 'rust-tests' 'cargo' (@('test') + $cargoFlags)
    Invoke-ForgeCheck 'component-examples' $script:ForgePython @('tools/verify_components.py','--compiler',"$script:ForgeSdk/bin/slint-compiler.exe",'--output-dir',"$script:ForgeEvidence/snippets")
    Invoke-ForgeCheck 'flow-bindings' $script:ForgePython @('tools/generate_flow_bindings.py','--check')
    Invoke-ForgeCheck 'plot-bindings' $script:ForgePython @('tools/generate_plot_bindings.py','--check')
    Invoke-ForgeCheck 'plot-ffi' $script:ForgePython @('tools/generate_plot_ffi.py','--check')
    Invoke-ForgeCheck 'icons' $script:ForgePython @('ui/icons/sync_catalog.py','--check')
    Configure-ForgeCpp $cpp
    Invoke-ForgeCheck 'cpp-build-debug' $script:ForgeCMake @('--build',$cpp,'--config','Debug','--parallel',"$Jobs")
    if ($Mode -eq 'Quick') {
        Invoke-ForgeCheck 'cpp-native-tests' $script:ForgeCTest @('--test-dir',$cpp,'-C','Debug','--output-on-failure','-R','_tests$')
    } else {
        Invoke-ForgeCheck 'rust-build' 'cargo' (@('build') + $cargoFlags)
        $env:SLINT_BACKEND = 'winit-skia'
        $env:SLINTFORGE_COMPONENT_QA_FILTER = $null
        # 完整验收主动触发一次最小化，验证恢复后重新布局和整窗截图。
        $env:SLINTFORGE_COMPONENT_QA_MINIMIZE_ONCE = '1'
        $env:SLINTFORGE_COMPONENT_QA_DIR = "$script:ForgeEvidence/gallery/cpp"
        $env:SLINTFORGE_INTERACTION_QA_DIR = "$script:ForgeEvidence/business/cpp"
        $env:SLINTFORGE_PLOT_QA_DIR = "$script:ForgeEvidence/plot/cpp"
        Invoke-ForgeCheck 'cpp-all-tests' $script:ForgeCTest @('--test-dir',$cpp,'-C','Debug','--output-on-failure')
        $env:SLINTFORGE_COMPONENT_QA_DIR = "$script:ForgeEvidence/gallery/rust"
        Invoke-ForgeCheck 'rust-gallery-pages' "$script:ForgeRoot/target/debug/slintforge-gallery-rust.exe" @('--components-smoke')
        $env:SLINTFORGE_INTERACTION_QA_DIR = "$script:ForgeEvidence/business/rust"
        foreach ($case in @('suggestion','dialog','capture','widgets','charts','grids')) {
            Invoke-ForgeCheck "rust-consumer-$case" "$script:ForgeRoot/target/debug/slintforge-example-rust.exe" @("--$case-smoke")
        }
        $env:SLINTFORGE_PLOT_QA_DIR = "$script:ForgeEvidence/plot/rust"
        Invoke-ForgeCheck 'rust-plot-consumer' "$script:ForgeRoot/target/debug/slintforge-plot-example.exe" @('--smoke')
        Invoke-ForgeCheck 'gallery-coverage' $script:ForgePython @('tools/verify_gallery_presentation.py','--geometry','--qa-root',"$script:ForgeEvidence/gallery",'--host','rust','--host','cpp')
        Invoke-ForgeCheck 'chart-legends' $script:ForgePython @('tools/verify_chart_legends.py','--qa-root',"$script:ForgeEvidence/gallery",'--host','rust','--host','cpp')
        Invoke-ForgeCheck 'grid-contracts' $script:ForgePython @('tools/verify_grids.py','--cpp-build-dir','build/cpp','--host','rust','--host','cpp')
    }
    $verifiedFingerprint = & $script:ForgePython "$PSScriptRoot/source_snapshot.py" --verification-fingerprint
    if ($LASTEXITCODE -ne 0) { throw '源码指纹生成失败' }
    if ($verifiedFingerprint -ne $startedFingerprint) { throw '验收期间应用源码或检查入口发生变化，请重新执行检查' }
    $fingerprint = & $script:ForgePython "$PSScriptRoot/source_snapshot.py" --fingerprint
    if ($LASTEXITCODE -ne 0) { throw '交付源码指纹生成失败' }
    $rustTests = 0
    foreach ($match in [regex]::Matches((Get-Content -Raw "$script:ForgeEvidence/rust-tests.log"), 'test result: ok\. (\d+) passed;')) { $rustTests += [int]$match.Groups[1].Value }
    $cppLog = $(if ($Mode -eq 'Quick') { 'cpp-native-tests.log' } else { 'cpp-all-tests.log' })
    $cppTests = [int][regex]::Match((Get-Content -Raw "$script:ForgeEvidence/$cppLog"), '0 tests failed out of (\d+)').Groups[1].Value
    $galleryStates = $(if ($Mode -eq 'Full') { @(Get-ChildItem -LiteralPath "$script:ForgeEvidence/gallery/rust" -Filter '*.png').Count } else { 0 })
    Write-ForgeJson ([ordered]@{ version=$script:ForgeVersion; mode=$Mode; passed=$true; evidence=$script:ForgeEvidence; checks=$script:ForgeChecks.Count; rust_tests=$rustTests; cpp_tests=$cppTests; component_examples=@(Get-ChildItem -LiteralPath "$script:ForgeRoot/examples/component-usage" -Filter '*.slint').Count; gallery_routes_per_host=$(if ($Mode -eq 'Full') { @(Get-Content -LiteralPath "$script:ForgeRoot/Gallery/data/navigation.txt" | Where-Object { $_.StartsWith('P|') }).Count } else { 0 }); gallery_states_per_host=$galleryStates; source_fingerprint=$fingerprint; verification_fingerprint=$verifiedFingerprint; rust='1.98.0'; slint='1.18.0'; renderer=$env:SLINT_BACKEND; completed=(Get-Date).ToString('o') }) "$script:ForgeEvidence/result.json"
    Write-Host "VERIFY_OK mode=$Mode evidence=$script:ForgeEvidence"
} catch {
    Write-ForgeJson ([ordered]@{ mode=$Mode; passed=$false; error=$_.Exception.Message; evidence=$script:ForgeEvidence }) "$script:ForgeEvidence/result.json"
    throw
}
