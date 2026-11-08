param([string]$EmsdkPath = '')
. "$PSScriptRoot/toolchain.ps1"
$emxx = Find-Emscripten $EmsdkPath
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    New-Item -ItemType Directory -Force web/generated | Out-Null
    & $emxx -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic -Iengine/include engine/src/Board.cpp engine/src/Game.cpp engine/src/wasm_api.cpp --no-entry '-sMODULARIZE=1' '-sEXPORT_ES6=1' '-sEXPORT_NAME=createChess' '-sENVIRONMENT=web,node' '-sALLOW_MEMORY_GROWTH=1' '-sFILESYSTEM=0' '-sEXPORTED_FUNCTIONS=_chess_state,_chess_reset,_chess_move' '-sEXPORTED_RUNTIME_METHODS=cwrap' -o web/generated/chess.mjs
    if ($LASTEXITCODE -ne 0) { throw 'Falha na compilacao WebAssembly.' }
    Write-Host 'WebAssembly gerado em web/generated. Execute scripts/serve.ps1.'
} finally { Pop-Location }
