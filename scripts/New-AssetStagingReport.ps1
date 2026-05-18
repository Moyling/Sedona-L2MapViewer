param(
    [ValidateSet("H5", "Fafurion", "Homonkulus", "Homunculus")]
    [string]$TargetProfile = "H5",

    [ValidateSet("H5", "Fafurion", "Homonkulus", "Homunculus")]
    [string]$DonorProfile = "Fafurion",

    [string]$StagingPath = ".\_asset-staging",
    [string]$L2ModderRoot = "C:\GITHUB\L2Modder_V2",
    [int]$LimitPerKind = 5,
    [switch]$Json
)

$ErrorActionPreference = "Stop"

$profiles = @{
    H5 = "Kliensek\Lineage II H5 Custom"
    Fafurion = "Kliensek\FULL CLIENT LINEAGE2 FAFURION REV 166 EU"
    Homonkulus = "Kliensek\L2NAP286D20201216G269"
    Homunculus = "Kliensek\L2NAP286D20201216G269"
}

$kinds = @(
    @{ Kind = "Maps"; Folder = "Maps"; Pattern = "*.unr" },
    @{ Kind = "StaticMeshes"; Folder = "StaticMeshes"; Pattern = "*.usx" },
    @{ Kind = "SysTextures"; Folder = "SysTextures"; Pattern = "*.utx" },
    @{ Kind = "Textures"; Folder = "Textures"; Pattern = "*.utx" },
    @{ Kind = "Animations"; Folder = "Animations"; Pattern = "*.ukx" },
    @{ Kind = "Sounds"; Folder = "Sounds"; Pattern = "*.uax" },
    @{ Kind = "System"; Folder = "system"; Pattern = "*.dat" }
)

function Resolve-ProfileRoot {
    param([string]$ProfileName)

    $relative = $profiles[$ProfileName]
    if (!$relative) {
        throw "Unknown profile: $ProfileName"
    }

    Join-Path $L2ModderRoot $relative
}

function Get-KindCount {
    param(
        [string]$Root,
        [hashtable]$Kind
    )

    $folder = Join-Path $Root $Kind.Folder
    if (!(Test-Path -LiteralPath $folder)) {
        return 0
    }

    @(Get-ChildItem -LiteralPath $folder -Filter $Kind.Pattern -File -ErrorAction SilentlyContinue).Count
}

function Get-DonorCandidates {
    param(
        [string]$Root,
        [hashtable]$Kind
    )

    $folder = Join-Path $Root $Kind.Folder
    if (!(Test-Path -LiteralPath $folder)) {
        return @()
    }

    Get-ChildItem -LiteralPath $folder -Filter $Kind.Pattern -File -ErrorAction SilentlyContinue |
        Where-Object { $_.BaseName -notmatch '(-fixed|-copy|backup|bak)$' } |
        Sort-Object Length -Descending |
        Select-Object -First $LimitPerKind |
        ForEach-Object {
            [pscustomobject]@{
                Kind = $Kind.Kind
                File = $_.Name
                SizeMB = [math]::Round($_.Length / 1MB, 2)
                Path = $_.FullName
            }
        }
}

$targetRoot = Resolve-ProfileRoot -ProfileName $TargetProfile
$donorRoot = Resolve-ProfileRoot -ProfileName $DonorProfile
$stagingFullPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($StagingPath)

$stagedFiles = if (Test-Path -LiteralPath $stagingFullPath) {
    @(Get-ChildItem -LiteralPath $stagingFullPath -File -Recurse -ErrorAction SilentlyContinue)
} else {
    @()
}

$kindRows = foreach ($kind in $kinds) {
    [pscustomobject]@{
        Kind = $kind.Kind
        TargetCount = Get-KindCount -Root $targetRoot -Kind $kind
        DonorCount = Get-KindCount -Root $donorRoot -Kind $kind
        StagedCount = @($stagedFiles | Where-Object { $_.Extension -like ([System.IO.Path]::GetExtension($kind.Pattern)) }).Count
    }
}

$report = [pscustomobject]@{
    TargetProfile = if ($TargetProfile -eq "Homunculus") { "Homonkulus" } else { $TargetProfile }
    TargetRoot = $targetRoot
    TargetExists = Test-Path -LiteralPath $targetRoot
    DonorProfile = if ($DonorProfile -eq "Homunculus") { "Homonkulus" } else { $DonorProfile }
    DonorRoot = $donorRoot
    DonorExists = Test-Path -LiteralPath $donorRoot
    StagingPath = $stagingFullPath
    StagingExists = Test-Path -LiteralPath $stagingFullPath
    StagedFileCount = $stagedFiles.Count
    StagedSizeMB = [math]::Round((($stagedFiles | Measure-Object Length -Sum).Sum) / 1MB, 2)
    KindSummary = @($kindRows)
    DonorCandidates = @($kinds | ForEach-Object { Get-DonorCandidates -Root $donorRoot -Kind $_ })
}

if ($Json) {
    $report | ConvertTo-Json -Depth 5
    return
}

Write-Host "Sedona asset staging report"
Write-Host "Target:  $($report.TargetProfile) - $($report.TargetRoot)"
Write-Host "Donor:   $($report.DonorProfile) - $($report.DonorRoot)"
Write-Host "Staging: $($report.StagingPath)"
Write-Host ""
Write-Host "Package summary"
$report.KindSummary | Format-Table -AutoSize
Write-Host ""
Write-Host "Top donor candidates by package size"
$report.DonorCandidates | Format-Table -AutoSize
