param(
    [ValidateSet("H5", "Fafurion", "Homonkulus", "Homunculus", "All")]
    [string]$Profile = "All",

    [int]$LimitPerKind = 3
)

$ErrorActionPreference = "Stop"

$profiles = @{
    H5 = "C:\GITHUB\L2Modder_V2\Kliensek\Lineage II H5 Custom"
    Fafurion = "C:\GITHUB\L2Modder_V2\Kliensek\FULL CLIENT LINEAGE2 FAFURION REV 166 EU"
    Homonkulus = "C:\GITHUB\L2Modder_V2\Kliensek\L2NAP286D20201216G269"
    Homunculus = "C:\GITHUB\L2Modder_V2\Kliensek\L2NAP286D20201216G269"
}

$kinds = @(
    @{ Kind = "Map"; Folder = "Maps"; Pattern = "*.unr"; Preferred = @("23_22.unr", "22_22.unr") },
    @{ Kind = "StaticMesh"; Folder = "StaticMeshes"; Pattern = "*.usx"; Preferred = @() },
    @{ Kind = "SysTexture"; Folder = "SysTextures"; Pattern = "*.utx"; Preferred = @() },
    @{ Kind = "Texture"; Folder = "Textures"; Pattern = "*.utx"; Preferred = @() }
)

function Get-PackageHeader {
    param([string]$Path)

    if (!(Test-Path -LiteralPath $Path)) {
        return [pscustomobject]@{ Header = "missing"; Protection = ""; Ready = $false }
    }

    $stream = [System.IO.File]::OpenRead($Path)
    try {
        $bytes = New-Object byte[] 32
        $bytesRead = $stream.Read($bytes, 0, $bytes.Length)
    } finally {
        $stream.Dispose()
    }

    if ($bytesRead -lt 4) {
        return [pscustomobject]@{ Header = "too-small"; Protection = ""; Ready = $false }
    }

    $signature = [BitConverter]::ToInt32($bytes, 0)
    $wideLineage = $bytesRead -ge 22 -and $bytes[0] -eq 0x4c -and $bytes[1] -eq 0x00 -and $bytes[2] -eq 0x69 -and $bytes[3] -eq 0x00
    $protection = ""

    if ($wideLineage) {
        $match = [regex]::Match([Text.Encoding]::Unicode.GetString($bytes), "Lineage2Ver\d+")
        if ($match.Success) {
            $protection = $match.Value
        }
    }

    $header = if ($signature -eq -1641389119) { "unreal-package" } elseif ($wideLineage) { "lineage-protected" } else { "unknown" }
    $ready = $header -eq "unreal-package" -or $protection -in @("Lineage2Ver111", "Lineage2Ver120", "Lineage2Ver121", "Lineage2Ver211", "Lineage2Ver212")

    [pscustomobject]@{
        Header = $header
        Protection = $protection
        Ready = $ready
    }
}

function Get-SampleFiles {
    param(
        [string]$ClientPath,
        [hashtable]$Kind
    )

    $folder = Join-Path $ClientPath $Kind.Folder
    if (!(Test-Path -LiteralPath $folder)) {
        return @()
    }

    $preferred = foreach ($name in $Kind.Preferred) {
        $path = Join-Path $folder $name
        if (Test-Path -LiteralPath $path) {
            Get-Item -LiteralPath $path
        }
    }

    $remaining = Get-ChildItem -LiteralPath $folder -Filter $Kind.Pattern -File -ErrorAction SilentlyContinue |
        Where-Object { @($preferred).FullName -notcontains $_.FullName } |
        Where-Object { $_.BaseName -notmatch '(-fixed|-copy|backup|bak)$' } |
        Sort-Object Length -Descending |
        Select-Object -First ([Math]::Max(0, $LimitPerKind - @($preferred).Count))

    @($preferred) + @($remaining)
}

function Test-ProfileAssets {
    param(
        [string]$Name,
        [string]$ClientPath
    )

    foreach ($kind in $kinds) {
        foreach ($file in Get-SampleFiles -ClientPath $ClientPath -Kind $kind) {
            $header = Get-PackageHeader -Path $file.FullName
            [pscustomobject]@{
                Profile = $Name
                Kind = $kind.Kind
                File = $file.Name
                Header = $header.Header
                Protection = $header.Protection
                Ready = $header.Ready
            }
        }
    }
}

if ($Profile -eq "All") {
    @(
        Test-ProfileAssets -Name "H5" -ClientPath $profiles.H5
        Test-ProfileAssets -Name "Fafurion" -ClientPath $profiles.Fafurion
        Test-ProfileAssets -Name "Homonkulus" -ClientPath $profiles.Homonkulus
    ) | Format-Table -AutoSize
    return
}

$profileName = if ($Profile -eq "Homunculus") { "Homonkulus" } else { $Profile }
Test-ProfileAssets -Name $profileName -ClientPath $profiles[$Profile] | Format-Table -AutoSize
