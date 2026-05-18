param(
    [ValidateSet("H5", "Fafurion", "Homonkulus", "Homunculus")]
    [string]$TargetProfile = "H5",

    [ValidateSet("H5", "Fafurion", "Homonkulus", "Homunculus")]
    [string]$DonorProfile = "Fafurion",

    [ValidateSet("Maps", "StaticMeshes", "SysTextures", "Textures", "Animations", "Sounds", "System")]
    [string]$Kind = "Maps",

    [string]$Package = "23_22.unr",
    [string]$StagingPath = ".\_asset-staging",
    [string]$L2ModderRoot = "C:\GITHUB\L2Modder_V2",
    [int]$Limit = 2000,
    [switch]$Execute,
    [switch]$Json
)

$ErrorActionPreference = "Stop"
$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$previewScript = Join-Path $scriptRoot "New-StagedImportPreview.ps1"

function Assert-IsUnderRoot {
    param(
        [string]$Root,
        [string]$Path
    )

    $rootFull = [System.IO.Path]::GetFullPath($Root).TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar
    $pathFull = [System.IO.Path]::GetFullPath($Path)

    if (!$pathFull.StartsWith($rootFull, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to write outside staging root. Root='$rootFull' Path='$pathFull'"
    }
}

$previewJson = & $previewScript `
    -TargetProfile $TargetProfile `
    -DonorProfile $DonorProfile `
    -Kind $Kind `
    -Package $Package `
    -StagingPath $StagingPath `
    -L2ModderRoot $L2ModderRoot `
    -Limit $Limit `
    -Json
$preview = $previewJson | ConvertFrom-Json
$stagingRoot = [System.IO.Path]::GetFullPath($preview.StagingPath)
$manifest = @($preview.CopyManifest)

$copyResults = foreach ($item in $manifest) {
    Assert-IsUnderRoot -Root $stagingRoot -Path $item.StagingTarget

    $result = [pscustomobject]@{
        Token = $item.Token
        Kind = $item.Kind
        Source = $item.Source
        StagingTarget = $item.StagingTarget
        Action = if ($Execute) { "Copy" } else { "DryRun" }
        Copied = $false
        Skipped = $false
        Message = ""
    }

    if (!(Test-Path -LiteralPath $item.Source)) {
        $result.Skipped = $true
        $result.Message = "Source file is missing."
        $result
        continue
    }

    if ((Test-Path -LiteralPath $item.StagingTarget) -and !$Execute) {
        $result.Skipped = $true
        $result.Message = "Already exists in staging."
        $result
        continue
    }

    if ($Execute) {
        $targetDir = Split-Path -Parent $item.StagingTarget
        if (!(Test-Path -LiteralPath $targetDir)) {
            New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
        }

        Copy-Item -LiteralPath $item.Source -Destination $item.StagingTarget -Force
        $result.Copied = $true
        $result.Message = "Copied to staging."
    } else {
        $result.Message = "Would copy to staging. Re-run with -Execute to write files."
    }

    $result
}

$report = [pscustomobject]@{
    Execute = [bool]$Execute
    TargetProfile = $preview.TargetProfile
    DonorProfile = $preview.DonorProfile
    Kind = $Kind
    Package = $Package
    StagingPath = $stagingRoot
    PlannedCopies = $manifest.Count
    Copied = @($copyResults | Where-Object { $_.Copied }).Count
    Skipped = @($copyResults | Where-Object { $_.Skipped }).Count
    Results = @($copyResults)
    PreviewWarnings = @($preview.Warnings)
}

if ($Json) {
    $report | ConvertTo-Json -Depth 8
    return
}

Write-Host "Sedona staged import copy"
Write-Host "Mode:    $(if ($Execute) { 'EXECUTE' } else { 'DRY-RUN' })"
Write-Host "Target:  $($report.TargetProfile)"
Write-Host "Donor:   $($report.DonorProfile)"
Write-Host "Package: $Kind/$Package"
Write-Host "Staging: $($report.StagingPath)"
Write-Host ""
Write-Host "Copy results"
if ($report.Results.Count -gt 0) {
    $report.Results | Select-Object Token, Kind, Action, Copied, Skipped, Message | Format-Table -AutoSize
} else {
    Write-Host " - no files need staging"
}
Write-Host ""
Write-Host "Preview warnings"
if ($report.PreviewWarnings.Count -gt 0) {
    $report.PreviewWarnings | ForEach-Object { Write-Host " - $_" }
} else {
    Write-Host " - none"
}
