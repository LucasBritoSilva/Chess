param([int]$Port = 8080)
$ErrorActionPreference = 'Stop'
$web = Join-Path (Split-Path $PSScriptRoot -Parent) 'web'
if (!(Test-Path -LiteralPath (Join-Path $web 'generated/chess.wasm'))) {
    throw 'Compile primeiro: powershell -ExecutionPolicy Bypass -File scripts/build.ps1'
}
Write-Host "ChessGame: http://localhost:$Port (Ctrl+C para encerrar)"
python -m http.server $Port --bind 127.0.0.1 --directory $web
