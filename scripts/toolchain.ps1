$ErrorActionPreference = 'Stop'
function Find-Emscripten([string]$Sdk) {
    if ($Sdk) {
        foreach ($name in @('em++.exe', 'em++.bat', 'em++')) {
            $candidate = Join-Path $Sdk "upstream/emscripten/$name"
            if (Test-Path -LiteralPath $candidate) { return $candidate }
        }
        throw "Emscripten não encontrado em $Sdk."
    }
    $command = Get-Command 'em++' -ErrorAction SilentlyContinue
    if ($command) { return $command.Source }
    foreach ($root in @($env:EMSDK, "$env:USERPROFILE/.codex/tools/emsdk", "$env:USERPROFILE/emsdk", 'C:/emsdk')) {
        if ($root) {
            foreach ($name in @('em++.exe', 'em++.bat', 'em++')) {
                $candidate = Join-Path $root "upstream/emscripten/$name"
                if (Test-Path -LiteralPath $candidate) { return $candidate }
            }
        }
    }
    throw 'Instale/ative o Emscripten ou informe -EmsdkPath. Consulte README.md.'
}
function Find-Node([string]$Emxx) {
    $command = Get-Command node -ErrorAction SilentlyContinue
    if ($command) { return $command.Source }
    $sdk = Split-Path (Split-Path (Split-Path $Emxx -Parent) -Parent) -Parent
    $candidate = Get-ChildItem -Path (Join-Path $sdk 'node/*/bin/node.exe'),(Join-Path $sdk 'node/*/node.exe') -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($candidate) { return $candidate.FullName }
    throw 'Node.js não encontrado. Ative o ambiente do emsdk.'
}
