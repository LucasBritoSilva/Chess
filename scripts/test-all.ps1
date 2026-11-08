param([string]$EmsdkPath = '', [string]$Compiler = 'g++', [switch]$WithOracle)
$ErrorActionPreference = 'Stop'
& "$PSScriptRoot/test-native.ps1" -Compiler $Compiler
& "$PSScriptRoot/test-wasm.ps1" -EmsdkPath $EmsdkPath
& "$PSScriptRoot/test-ui.ps1" -EmsdkPath $EmsdkPath
if ($WithOracle) { & "$PSScriptRoot/test-oracle.ps1" -Compiler $Compiler }
Write-Host 'Todas as verificacoes solicitadas passaram.'
