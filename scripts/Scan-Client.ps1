param(
    [string]$ClientPath = "",

    [ValidateSet("H5", "Fafurion", "Homonkulus", "Homunculus", "All")]
    [string]$Profile = ""
)

$ErrorActionPreference = "Stop"

$profiles = @{
    H5 = "C:\GITHUB\L2Modder_V2\Kliensek\Lineage II H5 Custom"
    Fafurion = "C:\GITHUB\L2Modder_V2\Kliensek\FULL CLIENT LINEAGE2 FAFURION REV 166 EU"
    Homonkulus = "C:\GITHUB\L2Modder_V2\Kliensek\L2NAP286D20201216G269"
    Homunculus = "C:\GITHUB\L2Modder_V2\Kliensek\L2NAP286D20201216G269"
}

function Get-ClientScan {
    param(
        [string]$Name,
        [string]$Path
    )

    if (!(Test-Path -LiteralPath $Path)) {
        return [pscustomobject]@{
            Profile = $Name
            Path = $Path
            Exists = $false
            Maps = 0
            StaticMeshes = 0
            SysTextures = 0
            Textures = 0
            SystemDat = 0
            DecodeMode = "missing"
            Ready = $false
        }
    }

    $maps = @(Get-ChildItem -LiteralPath (Join-Path $Path 'Maps') -Filter *.unr -ErrorAction SilentlyContinue).Count
    $staticMeshes = @(Get-ChildItem -LiteralPath (Join-Path $Path 'StaticMeshes') -Filter *.usx -ErrorAction SilentlyContinue).Count
    $sysTextures = @(Get-ChildItem -LiteralPath (Join-Path $Path 'SysTextures') -Filter *.utx -ErrorAction SilentlyContinue).Count
    $textures = @(Get-ChildItem -LiteralPath (Join-Path $Path 'Textures') -Filter *.utx -ErrorAction SilentlyContinue).Count
    $systemDat = @(Get-ChildItem -LiteralPath (Join-Path $Path 'system') -Filter *.dat -ErrorAction SilentlyContinue).Count
    $ready = $maps -gt 0 -and $staticMeshes -gt 0 -and ($sysTextures + $textures) -gt 0
    $decodeMode = if ($Name -eq "H5") { "XOR + l2encdec fallback" } else { "l2encdec fallback expected" }

    [pscustomobject]@{
        Profile = $Name
        Path = $Path
        Exists = $true
        Maps = $maps
        StaticMeshes = $staticMeshes
        SysTextures = $sysTextures
        Textures = $textures
        SystemDat = $systemDat
        DecodeMode = $decodeMode
        Ready = $ready
    }
}

if ($Profile -eq "All") {
    @(
        Get-ClientScan -Name "H5" -Path $profiles.H5
        Get-ClientScan -Name "Fafurion" -Path $profiles.Fafurion
        Get-ClientScan -Name "Homonkulus" -Path $profiles.Homonkulus
    ) | Format-Table -AutoSize
    return
}

if ($Profile) {
    $profileName = if ($Profile -eq "Homunculus") { "Homonkulus" } else { $Profile }
    Get-ClientScan -Name $profileName -Path $profiles[$Profile] | Format-List
    return
}

if (!$ClientPath) {
    throw "Use -Profile H5/Fafurion/Homonkulus/All or pass -ClientPath."
}

if (!(Test-Path -LiteralPath $ClientPath)) {
    throw "Client path was not found: $ClientPath"
}

Get-ClientScan -Name "Custom" -Path $ClientPath | Format-List
