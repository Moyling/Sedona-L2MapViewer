param(
    [ValidateSet("H5", "Fafurion", "Homonkulus", "Homunculus", "All")]
    [string]$Profile = "All",

    [int]$Limit = 5
)

$ErrorActionPreference = "Stop"
$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$probe = Join-Path $scriptRoot "Invoke-L2AssetProbe.ps1"

$profiles = if ($Profile -eq "All") {
    @("H5", "Fafurion", "Homonkulus")
} else {
    @($Profile)
}

$samples = @(
    @{ Kind = "Maps"; Package = "23_22.unr"; Contains = "StaticMeshActor" },
    @{ Kind = "Maps"; Package = "22_22.unr"; Contains = "Terrain" },
    @{ Kind = "StaticMeshes"; Package = "Aden_blazingswamp_S.usx"; Contains = "StaticMesh" },
    @{ Kind = "SysTextures"; Package = "LineageEffectsTextures.utx"; Contains = "Texture" }
)

foreach ($profileName in $profiles) {
    foreach ($sample in $samples) {
        try {
            $result = & $probe `
                -Profile $profileName `
                -Kind $sample.Kind `
                -Package $sample.Package `
                -Contains $sample.Contains `
                -Limit $Limit

            [pscustomobject]@{
                Profile = $profileName
                Kind = $sample.Kind
                Package = $result.Package
                Indexed = $true
                Version = $result.Version
                Imports = $result.ImportCount
                Exports = $result.ExportCount
                ListedImports = $result.ListedImports
                ListedExports = $result.ListedExports
                Error = $null
            }
        }
        catch {
            [pscustomobject]@{
                Profile = $profileName
                Kind = $sample.Kind
                Package = $sample.Package
                Indexed = $false
                Version = $null
                Imports = $null
                Exports = $null
                ListedImports = 0
                ListedExports = 0
                Error = $_.Exception.Message
            }
        }
    }
}
