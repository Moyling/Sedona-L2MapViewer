param(
    [ValidateSet("H5", "Fafurion", "Homonkulus", "Homunculus", "All")]
    [string]$Profile = "All",

    [string[]]$Tiles = @("23_22", "22_22"),

    [int]$DecodeTimeoutSeconds = 10
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = (Resolve-Path (Join-Path $scriptDir "..")).Path

$profiles = @{
    H5 = "C:\GITHUB\L2Modder_V2\Kliensek\Lineage II H5 Custom"
    Fafurion = "C:\GITHUB\L2Modder_V2\Kliensek\FULL CLIENT LINEAGE2 FAFURION REV 166 EU"
    Homonkulus = "C:\GITHUB\L2Modder_V2\Kliensek\L2NAP286D20201216G269"
    Homunculus = "C:\GITHUB\L2Modder_V2\Kliensek\L2NAP286D20201216G269"
}

function Get-L2EncDecPath {
    $candidates = @(
        (Join-Path $repoRoot "build\data\l2encdec\l2encdec.exe"),
        (Join-Path $repoRoot "third_party\l2encdec\l2encdec.exe"),
        "C:\GITHUB\L2Modder_V2\L2FileEdit\data\l2encdec\l2encdec.exe"
    )

    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate) {
            return $candidate
        }
    }

    return ""
}

function Get-PackageHeader {
    param([string]$Path)

    if (!(Test-Path -LiteralPath $Path)) {
        return [pscustomobject]@{
            Exists = $false
            Kind = "missing"
            Signature = ""
            Version = $null
        }
    }

    $stream = [System.IO.File]::OpenRead($Path)
    try {
        $bytes = New-Object byte[] 32
        $bytesRead = $stream.Read($bytes, 0, $bytes.Length)
    } finally {
        $stream.Dispose()
    }

    if ($bytesRead -lt 4) {
        return [pscustomobject]@{
            Exists = $true
            Kind = "too-small"
            Signature = ""
            Version = $null
        }
    }

    $signature = [BitConverter]::ToInt32($bytes, 0)
    $version = if ($bytesRead -ge 8) { [BitConverter]::ToInt32($bytes, 4) -band 0xffff } else { $null }
    $wideLineage = $bytesRead -ge 22 -and
        $bytes[0] -eq 0x4c -and $bytes[1] -eq 0x00 -and
        $bytes[2] -eq 0x69 -and $bytes[3] -eq 0x00

    $kind = if ($signature -eq -1641389119) {
        "unreal-package"
    } elseif ($wideLineage) {
        "lineage-protected"
    } else {
        "unknown"
    }

    [pscustomobject]@{
        Exists = $true
        Kind = $kind
        Signature = ("0x{0:X8}" -f ($signature -band 0xffffffff))
        Version = $version
    }
}

function Test-Decode {
    param(
        [string]$SourcePath,
        [string]$DecoderPath
    )

    if (!$DecoderPath -or !(Test-Path -LiteralPath $DecoderPath)) {
        return "no-decoder"
    }

    $tmp = Join-Path ([System.IO.Path]::GetTempPath()) ("sedona-mapviewer-" + [guid]::NewGuid().ToString("N") + ".unr")
    try {
        $process = Start-Process -FilePath $DecoderPath -ArgumentList @("-l", $SourcePath, $tmp) -WorkingDirectory (Split-Path -Parent $DecoderPath) -PassThru -WindowStyle Hidden
        if (!$process.WaitForExit($DecodeTimeoutSeconds * 1000)) {
            Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
            return "timeout"
        }

        if ($process.ExitCode -ne 0 -or !(Test-Path -LiteralPath $tmp)) {
            return "failed"
        }

        $decodedHeader = Get-PackageHeader -Path $tmp
        if ($decodedHeader.Kind -eq "unreal-package") {
            return "decoded"
        }

        return "decoded-invalid"
    } finally {
        Remove-Item -LiteralPath $tmp -Force -ErrorAction SilentlyContinue
    }
}

function Test-ProfileTiles {
    param(
        [string]$Name,
        [string]$ClientPath
    )

    $decoder = Get-L2EncDecPath

    foreach ($tile in $Tiles) {
        $mapPath = Join-Path $ClientPath ("Maps\{0}.unr" -f $tile)
        $header = Get-PackageHeader -Path $mapPath
        $decode = "not-needed"

        if ($header.Kind -eq "lineage-protected") {
            $decode = Test-Decode -SourcePath $mapPath -DecoderPath $decoder
        } elseif (!$header.Exists) {
            $decode = "missing"
        } elseif ($header.Kind -ne "unreal-package") {
            $decode = "unknown"
        }

        [pscustomobject]@{
            Profile = $Name
            Tile = $tile
            MapPath = $mapPath
            Header = $header.Kind
            Signature = $header.Signature
            Version = $header.Version
            Decode = $decode
            Ready = ($header.Kind -eq "unreal-package" -or $decode -eq "decoded")
        }
    }
}

if ($Profile -eq "All") {
    @(
        Test-ProfileTiles -Name "H5" -ClientPath $profiles.H5
        Test-ProfileTiles -Name "Fafurion" -ClientPath $profiles.Fafurion
        Test-ProfileTiles -Name "Homonkulus" -ClientPath $profiles.Homonkulus
    ) | Select-Object Profile, Tile, Header, Signature, Version, Decode, Ready | Format-Table -AutoSize
    return
}

$profileName = if ($Profile -eq "Homunculus") { "Homonkulus" } else { $Profile }
Test-ProfileTiles -Name $profileName -ClientPath $profiles[$Profile] | Select-Object Profile, Tile, Header, Signature, Version, Decode, Ready | Format-Table -AutoSize
