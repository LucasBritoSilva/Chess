param([string]$EmsdkPath = '')
. "$PSScriptRoot/toolchain.ps1"
$emxx = Find-Emscripten $EmsdkPath
$node = Find-Node $emxx
& "$PSScriptRoot/build.ps1" -EmsdkPath $EmsdkPath
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    New-Item -ItemType Directory -Force build | Out-Null
    & $emxx -std=c++17 -O2 -DNDEBUG -fexceptions -Iengine/include engine/src/Board.cpp engine/src/Game.cpp tests/test_chess.cpp '-sENVIRONMENT=node' '-sEXIT_RUNTIME=1' '-sWASM_ASYNC_COMPILATION=0' -o build/test_chess.cjs
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao compilar testes WebAssembly.' }
    & $node build/test_chess.cjs
    if ($LASTEXITCODE -ne 0) { throw 'Testes C++ no WebAssembly falharam.' }
    & $node tests/test_wasm.mjs
    if ($LASTEXITCODE -ne 0) { throw 'Testes de integração falharam.' }
} finally { Pop-Location }
