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
    [switch]$Json
)

$ErrorActionPreference = "Stop"
$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$dependencyScript = Join-Path $scriptRoot "New-AssetDependencyReport.ps1"

$profiles = @{
    H5 = "Kliensek\Lineage II H5 Custom"
    Fafurion = "Kliensek\FULL CLIENT LINEAGE2 FAFURION REV 166 EU"
    Homonkulus = "Kliensek\L2NAP286D20201216G269"
    Homunculus = "Kliensek\L2NAP286D20201216G269"
}

$kindFolders = @(
    @{ Kind = "Maps"; Folder = "Maps"; Extension = ".unr" },
    @{ Kind = "StaticMeshes"; Folder = "StaticMeshes"; Extension = ".usx" },
    @{ Kind = "SysTextures"; Folder = "SysTextures"; Extension = ".utx" },
    @{ Kind = "Textures"; Folder = "Textures"; Extension = ".utx" },
    @{ Kind = "Animations"; Folder = "Animations"; Extension = ".ukx" },
    @{ Kind = "Sounds"; Folder = "Sounds"; Extension = ".uax" },
    @{ Kind = "System"; Folder = "system"; Extension = ".dat" }
)

$builtInTokens = @("Core", "Engine", "Editor", "Fire", "Gameplay", "IpDrv", "UWindow")

function Resolve-ProfileRoot {
    param([string]$ProfileName)

    $relative = $profiles[$ProfileName]
    if (!$relative) {
        throw "Unknown profile: $ProfileName"
    }

    Join-Path $L2ModderRoot $relative
}

function Test-PackageTokenExists {
    param(
        [string]$Root,
        [string]$Token
    )

    foreach ($kind in $kindFolders) {
        $candidate = Join-Path (Join-Path $Root $kind.Folder) ($Token + $kind.Extension)
        if (Test-Path -LiteralPath $candidate) {
            return [pscustomobject]@{
                Exists = $true
                Kind = $kind.Kind
                Path = (Resolve-Path -LiteralPath $candidate).Path
            }
        }
    }

    return [pscustomobject]@{
        Exists = $false
        Kind = $null
        Path = $null
    }
}

function Test-StagedTokenExists {
    param(
        [string]$Root,
        [string]$Token
    )

    if (!(Test-Path -LiteralPath $Root)) {
        return [pscustomobject]@{ Exists = $false; Path = $null }
    }

    foreach ($kind in $kindFolders) {
        $direct = Join-Path $Root (Join-Path $kind.Folder ($Token + $kind.Extension))
        if (Test-Path -LiteralPath $direct) {
            return [pscustomobject]@{
                Exists = $true
                Path = (Resolve-Path -LiteralPath $direct).Path
            }
        }
    }

    $matches = Get-ChildItem -LiteralPath $Root -File -Recurse -ErrorAction SilentlyContinue |
        Where-Object { $_.BaseName -ieq $Token } |
        Select-Object -First 1

    if ($matches) {
        return [pscustomobject]@{
            Exists = $true
            Path = $matches.FullName
        }
    }

    return [pscustomobject]@{ Exists = $false; Path = $null }
}

$targetRoot = Resolve-ProfileRoot -ProfileName $TargetProfile
$donorRoot = Resolve-ProfileRoot -ProfileName $DonorProfile
$stagingFullPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($StagingPath)

$dependencyJson = & $dependencyScript `
    -Profile $DonorProfile `
    -Kind $Kind `
    -Package $Package `
    -Limit $Limit `
    -L2ModderRoot $L2ModderRoot `
    -Json
$dependencyReport = $dependencyJson | ConvertFrom-Json

$dependencyRows = foreach ($token in @($dependencyReport.DependencyPackages)) {
    if ($builtInTokens -icontains $token) {
        [pscustomobject]@{
            Token = $token
            State = "BuiltIn"
            TargetKind = "Engine"
            DonorKind = "Engine"
            DonorPath = $null
            StagingPath = $null
        }
        continue
    }

    $targetCheck = Test-PackageTokenExists -Root $targetRoot -Token $token
    $donorCheck = Test-PackageTokenExists -Root $donorRoot -Token $token
    $stagingCheck = Test-StagedTokenExists -Root $stagingFullPath -Token $token

    $state = if ($targetCheck.Exists) {
        "AlreadyInTarget"
    } elseif ($stagingCheck.Exists) {
        "AlreadyStaged"
    } elseif ($donorCheck.Exists) {
        "NeedsStaging"
    } else {
        "MissingFromKnownFolders"
    }

    [pscustomobject]@{
        Token = $token
        State = $state
        TargetKind = $targetCheck.Kind
        DonorKind = $donorCheck.Kind
        DonorPath = $donorCheck.Path
        StagingPath = $stagingCheck.Path
    }
}

$warnings = New-Object System.Collections.Generic.List[string]
if (!(Test-Path -LiteralPath $targetRoot)) {
    $warnings.Add("Target client path is missing: $targetRoot")
}
if (!(Test-Path -LiteralPath $donorRoot)) {
    $warnings.Add("Donor client path is missing: $donorRoot")
}
if (!(Test-Path -LiteralPath $stagingFullPath)) {
    $warnings.Add("Staging folder is missing: $stagingFullPath")
}
$missing = @($dependencyRows | Where-Object { $_.State -eq "MissingFromKnownFolders" })
if ($missing.Count -gt 0) {
    $warnings.Add("$($missing.Count) dependency tokens were not found in target, donor, or staging folders.")
}
$needsStaging = @($dependencyRows | Where-Object { $_.State -eq "NeedsStaging" })
if ($needsStaging.Count -gt 0) {
    $warnings.Add("$($needsStaging.Count) dependency package(s) need staging before import.")
}

$copyManifest = @(
    $needsStaging |
        Where-Object { $_.DonorPath } |
        ForEach-Object {
            $relativeFolder = if ($_.DonorKind -eq "System") { "system" } else { $_.DonorKind }
            $targetPath = Join-Path $stagingFullPath (Join-Path $relativeFolder (Split-Path -Leaf $_.DonorPath))

            [pscustomobject]@{
                Token = $_.Token
                Kind = $_.DonorKind
                Source = $_.DonorPath
                StagingTarget = $targetPath
                ExistsInStaging = Test-Path -LiteralPath $targetPath
                Action = "Copy"
            }
        }
)

$report = [pscustomobject]@{
    TargetProfile = if ($TargetProfile -eq "Homunculus") { "Homonkulus" } else { $TargetProfile }
    TargetRoot = $targetRoot
    DonorProfile = if ($DonorProfile -eq "Homunculus") { "Homonkulus" } else { $DonorProfile }
    DonorRoot = $donorRoot
    Kind = $Kind
    Package = $Package
    StagingPath = $stagingFullPath
    DependencyReport = $dependencyReport
    DependencyPreview = @($dependencyRows)
    CopyManifest = @($copyManifest)
    Summary = [pscustomobject]@{
        AlreadyInTarget = @($dependencyRows | Where-Object { $_.State -eq "AlreadyInTarget" }).Count
        AlreadyStaged = @($dependencyRows | Where-Object { $_.State -eq "AlreadyStaged" }).Count
        NeedsStaging = @($dependencyRows | Where-Object { $_.State -eq "NeedsStaging" }).Count
        MissingFromKnownFolders = @($dependencyRows | Where-Object { $_.State -eq "MissingFromKnownFolders" }).Count
        BuiltIn = @($dependencyRows | Where-Object { $_.State -eq "BuiltIn" }).Count
        ManifestCopies = $copyManifest.Count
    }
    Warnings = @($warnings)
}

if ($Json) {
    $report | ConvertTo-Json -Depth 8
    return
}

Write-Host "Sedona staged import preview"
Write-Host "Target:  $($report.TargetProfile) - $($report.TargetRoot)"
Write-Host "Donor:   $($report.DonorProfile) - $($report.DonorRoot)"
Write-Host "Package: $Kind/$Package"
Write-Host "Staging: $($report.StagingPath)"
Write-Host ""
Write-Host "Dependency summary"
$report.Summary | Format-List
Write-Host ""
Write-Host "Dependencies needing staging"
$report.DependencyPreview |
    Where-Object { $_.State -in @("NeedsStaging", "MissingFromKnownFolders") } |
    Select-Object Token, State, DonorKind, DonorPath |
    Format-Table -AutoSize
Write-Host ""
Write-Host "Dry-run copy manifest"
if ($report.CopyManifest.Count -gt 0) {
    $report.CopyManifest |
        Select-Object Token, Kind, Source, StagingTarget |
        Format-Table -AutoSize
} else {
    Write-Host " - no copy actions required"
}
Write-Host ""
Write-Host "Warnings"
if ($report.Warnings.Count -gt 0) {
    $report.Warnings | ForEach-Object { Write-Host " - $_" }
} else {
    Write-Host " - none"
}
