param([string]$Compiler = 'g++')
$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    New-Item -ItemType Directory -Force build | Out-Null
    & $Compiler -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic -Iengine/include engine/src/Board.cpp engine/src/Game.cpp tests/test_chess.cpp -o build/test_chess.exe
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao compilar os testes.' }
    & ./build/test_chess.exe
    if ($LASTEXITCODE -ne 0) { throw 'Os testes falharam.' }
} finally { Pop-Location }
