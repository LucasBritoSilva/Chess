param([string]$Compiler = 'g++')
$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    if (!(Test-Path -LiteralPath 'build/oracle-venv/Scripts/python.exe')) {
        python -m venv build/oracle-venv
        if ($LASTEXITCODE -ne 0) { throw 'Falha ao criar ambiente de teste Python.' }
    }
    & ./build/oracle-venv/Scripts/python.exe -m pip install -r tests/requirements.txt
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao instalar o oraculo de teste.' }
    & $Compiler -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic -Iengine/include engine/src/Board.cpp engine/src/Game.cpp tests/oracle_bridge.cpp -o build/oracle_bridge.exe
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao compilar o comparador.' }
    & ./build/oracle-venv/Scripts/python.exe tests/test_oracle.py
    if ($LASTEXITCODE -ne 0) { throw 'Comparacao independente falhou.' }
} finally { Pop-Location }
