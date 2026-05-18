param(
    [ValidateSet("H5", "Fafurion", "Homonkulus", "Homunculus")]
    [string]$Profile = "H5",

    [ValidateSet("Maps", "StaticMeshes", "SysTextures", "Textures", "Animations", "Sounds", "System")]
    [string]$Kind = "Maps",

    [string]$Package = "23_22.unr",
    [string]$Contains = "",
    [int]$Limit = 20,
    [string]$L2ModderRoot = "C:\GITHUB\L2Modder_V2",
    [switch]$Raw
)

$ErrorActionPreference = "Stop"

function Resolve-ExistingPath {
    param([string[]]$Candidates)

    foreach ($candidate in $Candidates) {
        if ($candidate -and (Test-Path -LiteralPath $candidate)) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    return $null
}

function Resolve-ClientRoot {
    param([string]$ProfileName, [string]$Root)

    switch ($ProfileName) {
        "H5" { return Join-Path $Root "Kliensek\Lineage II H5 Custom" }
        "Fafurion" { return Join-Path $Root "Kliensek\FULL CLIENT LINEAGE2 FAFURION REV 166 EU" }
        "Homonkulus" { return Join-Path $Root "Kliensek\L2NAP286D20201216G269" }
        "Homunculus" { return Join-Path $Root "Kliensek\L2NAP286D20201216G269" }
    }
}

function Resolve-PackagePath {
    param(
        [string]$ClientRoot,
        [string]$PackageKind,
        [string]$PackageName
    )

    if ([System.IO.Path]::IsPathRooted($PackageName)) {
        if (!(Test-Path -LiteralPath $PackageName)) {
            throw "Package path not found: $PackageName"
        }

        return (Resolve-Path -LiteralPath $PackageName).Path
    }

    $folder = Join-Path $ClientRoot $PackageKind
    if (!(Test-Path -LiteralPath $folder)) {
        throw "Package folder not found: $folder"
    }

    $direct = Join-Path $folder $PackageName
    if (Test-Path -LiteralPath $direct) {
        return (Resolve-Path -LiteralPath $direct).Path
    }

    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($PackageName)
    $matches = Get-ChildItem -LiteralPath $folder -File |
        Where-Object {
            $_.BaseName -ieq $baseName -or $_.Name -ieq $PackageName
        } |
        Sort-Object Name

    if ($matches.Count -gt 0) {
        return $matches[0].FullName
    }

    throw "Package '$PackageName' was not found under $folder"
}

function Find-L2XdatEditorHome {
    param([string]$Root)

    $candidates = @(
        (Join-Path $Root "Kliensek\L2NAP286D20201216G269\L2Xdat_editor"),
        (Join-Path $Root "L2Xdat_editor")
    )

    $direct = Resolve-ExistingPath $candidates
    if ($direct) {
        return $direct
    }

    $found = Get-ChildItem -LiteralPath $Root -Directory -Recurse -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -ieq "L2Xdat_editor" -and (Test-Path -LiteralPath (Join-Path $_.FullName "lib\l2unreal-1.3.3.jar")) } |
        Select-Object -First 1

    if ($found) {
        return $found.FullName
    }

    return $null
}

$clientRoot = Resolve-ClientRoot -ProfileName $Profile -Root $L2ModderRoot
if (!(Test-Path -LiteralPath $clientRoot)) {
    throw "Client root not found for profile ${Profile}: $clientRoot"
}

$toolRoot = Join-Path $L2ModderRoot "L2AssetViewer\tools"
$cmd = Join-Path $toolRoot "dump-package-json.cmd"
if (!(Test-Path -LiteralPath $cmd)) {
    throw "L2AssetViewer package index command not found: $cmd"
}

$l2XdatEditorHome = Find-L2XdatEditorHome -Root $L2ModderRoot
if (!$l2XdatEditorHome) {
    throw "L2Xdat_editor with l2unreal libraries was not found under $L2ModderRoot"
}

$packagePath = Resolve-PackagePath -ClientRoot $clientRoot -PackageKind $Kind -PackageName $Package
$previousHome = $env:L2XDAT_EDITOR_HOME
$env:L2XDAT_EDITOR_HOME = $l2XdatEditorHome
$mutex = New-Object System.Threading.Mutex($false, "Global\SedonaL2AssetProbeCompile")
$hasMutex = $false

try {
    $hasMutex = $mutex.WaitOne([TimeSpan]::FromMinutes(2))
    if (!$hasMutex) {
        throw "Timed out waiting for the L2AssetViewer package index compile lock."
    }

    $containsArg = if ($Contains) { $Contains } else { " " }
    $jsonText = & $cmd $packagePath $containsArg $Limit
    if ($LASTEXITCODE -ne 0) {
        throw "L2AssetViewer probe failed with exit code $LASTEXITCODE"
    }

    if ($Raw) {
        $jsonText
        return
    }

    $json = $jsonText | ConvertFrom-Json
    [pscustomobject]@{
        Profile = $Profile
        Kind = $Kind
        Package = Split-Path -Leaf $packagePath
        PackagePath = $packagePath
        PackageName = $json.packageName
        Version = $json.version
        License = $json.license
        NameCount = $json.nameCount
        ImportCount = $json.importCount
        ExportCount = $json.exportCount
        ListedImports = @($json.imports).Count
        ListedExports = @($json.exports).Count
        FirstImports = (@($json.imports) | Select-Object -First 5 | ForEach-Object { $_.objectName }) -join "; "
        FirstExports = (@($json.exports) | Select-Object -First 5 | ForEach-Object { $_.objectName }) -join "; "
    }
}
finally {
    if ($hasMutex) {
        $mutex.ReleaseMutex()
    }
    $mutex.Dispose()
    $env:L2XDAT_EDITOR_HOME = $previousHome
}
