param([string]$EmsdkPath = '')
. "$PSScriptRoot/toolchain.ps1"
$node = Find-Node (Find-Emscripten $EmsdkPath)
Push-Location (Split-Path $PSScriptRoot -Parent)
$previousPath = $env:PATH
try {
    if (!(Test-Path -LiteralPath 'web/generated/chess.wasm')) { throw 'Compile primeiro com scripts/build.ps1.' }
    $env:PATH = (Split-Path $node -Parent) + ';' + $env:PATH
    & (Join-Path (Split-Path $node -Parent) 'npm.cmd') ci --ignore-scripts
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao instalar dependencias de teste.' }
    & $node tests/test_ui.mjs
    if ($LASTEXITCODE -ne 0) { throw 'Os testes de interface falharam.' }
} finally {
    $env:PATH = $previousPath
    Pop-Location
}
