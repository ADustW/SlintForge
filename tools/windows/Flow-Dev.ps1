[CmdletBinding()]
param([Parameter(Mandatory)][string]$Task, [string]$Renderer = 'winit-skia', [string]$Config = 'Debug')
. (Join-Path $PSScriptRoot 'Common.ps1')
Initialize-ForgeEnvironment (Join-Path $PSScriptRoot '../../../slint-sdk-1.18.0') 'C:/Users/wzp/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Program Files/JetBrains/CLion 2026.1.1/bin/cmake/win/x64/bin/cmake.exe' 'D:/.clion-env/qt-6.8.3-msvc.bat'
$env:SLINT_BACKEND = $Renderer
Set-Location -LiteralPath $script:ForgeRoot
switch ($Task) {
    'configure' { & $script:ForgeCMake -S . -B build/cpp -G 'Visual Studio 18 2026' -A x64 "-DSlint_DIR=$script:ForgeSdk/lib/cmake/Slint" }
    'cpp' { & $script:ForgeCMake --build build/cpp --config $Config --parallel 2 --target slintforge_flow_tests slintforge_flow_example }
    'rust' { $buildFlags = @(); if ($Config -eq 'Release') { $buildFlags += '--release' }; & cargo build -p slintforge-runtime -p slintforge-flow-example --offline @buildFlags }
    'gallery-cpp' { & $script:ForgeCMake --build build/cpp --config $Config --parallel 2 --target slintforge_gallery_cpp }
    'gallery-rust' { & cargo build -p slintforge-gallery-rust --offline }
    'ffi' { & $script:ForgePython tools/generate_runtime_ffi.py }
    default { throw "未知开发操作：$Task" }
}
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
