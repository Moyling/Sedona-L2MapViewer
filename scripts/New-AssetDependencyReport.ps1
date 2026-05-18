param(
    [ValidateSet("H5", "Fafurion", "Homonkulus", "Homunculus")]
    [string]$Profile = "H5",

    [ValidateSet("Maps", "StaticMeshes", "SysTextures", "Textures", "Animations", "Sounds", "System")]
    [string]$Kind = "Maps",

    [string]$Package = "23_22.unr",
    [int]$Limit = 2000,
    [string]$L2ModderRoot = "C:\GITHUB\L2Modder_V2",
    [switch]$Json
)

$ErrorActionPreference = "Stop"
$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$probe = Join-Path $scriptRoot "Invoke-L2AssetProbe.ps1"

function Get-PackageToken {
    param([object]$Item)

    $candidates = @(
        $Item.packageName,
        $Item.PackageName,
        $Item.outerName,
        $Item.OuterName,
        $Item.objectName,
        $Item.ObjectName,
        $Item.className,
        $Item.ClassName
    )

    foreach ($candidate in $candidates) {
        if (!$candidate) {
            continue
        }

        $text = [string]$candidate
        if ($text -match "^([A-Za-z0-9_\-]+)\.") {
            return $matches[1]
        }
    }

    return $null
}

function Get-ObjectText {
    param([object]$Item)

    $parts = @(
        $Item.objectName,
        $Item.ObjectName,
        $Item.className,
        $Item.ClassName,
        $Item.packageName,
        $Item.PackageName
    ) | Where-Object { $_ }

    ($parts | ForEach-Object { [string]$_ }) -join " | "
}

function Group-ByClass {
    param([object[]]$Items)

    $Items |
        ForEach-Object {
            $className = if ($_.className) { $_.className } elseif ($_.ClassName) { $_.ClassName } else { "Unknown" }
            [pscustomobject]@{ ClassName = [string]$className }
        } |
        Group-Object ClassName |
        Sort-Object -Property @{ Expression = "Count"; Descending = $true }, Name |
        ForEach-Object {
            [pscustomobject]@{
                ClassName = $_.Name
                Count = $_.Count
            }
        }
}

$rawJson = & $probe -Profile $Profile -Kind $Kind -Package $Package -Limit $Limit -L2ModderRoot $L2ModderRoot -Raw
if ($LASTEXITCODE -ne 0) {
    throw "Asset probe failed with exit code $LASTEXITCODE"
}

$packageInfo = $rawJson | ConvertFrom-Json
$imports = @($packageInfo.imports)
$exports = @($packageInfo.exports)

$dependencyPackages = @(
    $imports |
        ForEach-Object { Get-PackageToken -Item $_ } |
        Where-Object { $_ } |
        Sort-Object -Unique
)

$warnings = New-Object System.Collections.Generic.List[string]
if ($imports.Count -eq 0) {
    $warnings.Add("No imports were listed. The probe limit may be too low or the package has no external imports.")
}
if ($packageInfo.importCount -gt $imports.Count) {
    $warnings.Add("Only $($imports.Count) of $($packageInfo.importCount) imports are listed. Increase -Limit for a fuller dependency report.")
}
if ($packageInfo.exportCount -gt $exports.Count) {
    $warnings.Add("Only $($exports.Count) of $($packageInfo.exportCount) exports are listed. Increase -Limit for a fuller export report.")
}
if ($dependencyPackages.Count -eq 0 -and $imports.Count -gt 0) {
    $warnings.Add("Imports exist but package names could not be extracted from the current probe JSON shape.")
}

$report = [pscustomobject]@{
    Profile = if ($Profile -eq "Homunculus") { "Homonkulus" } else { $Profile }
    Kind = $Kind
    Package = $Package
    PackageName = $packageInfo.packageName
    Version = $packageInfo.version
    License = $packageInfo.license
    ImportCount = $packageInfo.importCount
    ExportCount = $packageInfo.exportCount
    ListedImports = $imports.Count
    ListedExports = $exports.Count
    DependencyPackages = $dependencyPackages
    ImportClassSummary = @(Group-ByClass -Items $imports)
    ExportClassSummary = @(Group-ByClass -Items $exports)
    SampleImports = @($imports | Select-Object -First 20 | ForEach-Object { Get-ObjectText -Item $_ })
    SampleExports = @($exports | Select-Object -First 20 | ForEach-Object { Get-ObjectText -Item $_ })
    Warnings = @($warnings)
}

if ($Json) {
    $report | ConvertTo-Json -Depth 6
    return
}

Write-Host "Sedona asset dependency report"
Write-Host "Profile: $($report.Profile)"
Write-Host "Package: $($report.PackageName) [$Kind/$Package]"
Write-Host "Version: $($report.Version)  License: $($report.License)"
Write-Host "Imports: $($report.ListedImports)/$($report.ImportCount)  Exports: $($report.ListedExports)/$($report.ExportCount)"
Write-Host ""
Write-Host "Dependency package tokens"
if ($report.DependencyPackages.Count -gt 0) {
    $report.DependencyPackages | ForEach-Object { Write-Host " - $_" }
} else {
    Write-Host " - none detected"
}
Write-Host ""
Write-Host "Import class summary"
$report.ImportClassSummary | Format-Table -AutoSize
Write-Host ""
Write-Host "Export class summary"
$report.ExportClassSummary | Format-Table -AutoSize
Write-Host ""
Write-Host "Warnings"
if ($report.Warnings.Count -gt 0) {
    $report.Warnings | ForEach-Object { Write-Host " - $_" }
} else {
    Write-Host " - none"
}
