$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = (Resolve-Path (Join-Path $scriptDir "..")).Path
$runtimeSource = Join-Path $repoRoot "runtime"
$depsPath = Join-Path $repoRoot "deps"
$buildPath = Join-Path $repoRoot "build"
$decoderSource = Join-Path $repoRoot "third_party\l2encdec"
$decoderTarget = Join-Path $buildPath "data\l2encdec"

if (!(Test-Path -LiteralPath $runtimeSource)) {
    throw "Missing runtime folder: $runtimeSource"
}

if (!(Test-Path -LiteralPath $depsPath)) {
    throw "Missing deps folder: $depsPath"
}

Write-Host "Installing Sedona runtime files..."
New-Item -ItemType Directory -Force -Path $buildPath | Out-Null
Copy-Item -Path (Join-Path $runtimeSource "*") -Destination $buildPath -Recurse -Force

if (Test-Path -LiteralPath (Join-Path $decoderSource "l2encdec.exe")) {
    Write-Host "Installing l2encdec decoder..."
    New-Item -ItemType Directory -Force -Path $decoderTarget | Out-Null
    Copy-Item -Path (Join-Path $decoderSource "*") -Destination $decoderTarget -Recurse -Force
}

Write-Host "Bootstrap complete."
Write-Host "Build with:"
Write-Host "  msbuild Sedona-L2MapViewer.sln /m /p:Configuration=Release /p:Platform=x64"
