param(
    [ValidateSet("H5", "Fafurion", "Homonkulus", "Homunculus")]
    [string]$Profile = "H5",

    [string]$GeodataPath = "",

    [switch]$NoStart
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = (Resolve-Path (Join-Path $scriptDir "..")).Path
$exePath = Join-Path $repoRoot "build\Sedona-L2MapViewer.exe"

$profiles = @{
    H5 = "C:\GITHUB\L2Modder_V2\Kliensek\Lineage II H5 Custom"
    Fafurion = "C:\GITHUB\L2Modder_V2\Kliensek\FULL CLIENT LINEAGE2 FAFURION REV 166 EU"
    Homonkulus = "C:\GITHUB\L2Modder_V2\Kliensek\L2NAP286D20201216G269"
    Homunculus = "C:\GITHUB\L2Modder_V2\Kliensek\L2NAP286D20201216G269"
}

$clientPath = $profiles[$Profile]

if (!(Test-Path -LiteralPath $exePath)) {
    throw "Viewer executable was not found: $exePath. Run scripts\Bootstrap-Runtime.ps1 and build Release x64 first."
}

if (!(Test-Path -LiteralPath $clientPath)) {
    throw "Client profile path was not found: $clientPath"
}

$profileArg = if ($Profile -eq "Homunculus") { "Homonkulus" } else { $Profile }
$arguments = @("--profile=$profileArg", "--client=""$clientPath""")

if ($GeodataPath) {
    if (!(Test-Path -LiteralPath $GeodataPath)) {
        throw "Geodata path was not found: $GeodataPath"
    }
    $arguments += "--geodata=""$GeodataPath"""
}

Write-Host "Profile: $Profile"
Write-Host "Client:  $clientPath"
if ($GeodataPath) {
    Write-Host "Geodata: $GeodataPath"
}

if ($NoStart) {
    Write-Host "$exePath $($arguments -join ' ')"
    return
}

Start-Process -FilePath $exePath -WorkingDirectory (Join-Path $repoRoot "build") -ArgumentList $arguments
