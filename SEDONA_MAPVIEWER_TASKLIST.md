# Sedona-L2MapViewer Tasklist

## Prototype Warning

This is not a finished release yet. The viewer starts and can load map tiles, but client source selection, automatic geodata generation, editing, cross-client asset transfer, and the full asset/UI workflow are still incomplete.

## Target Product

Sedona should become a Lineage II map builder, not only a map viewer.

- [ ] Viewer: inspect maps, BSP, terrain, static meshes, textures, actors, water, collision, and existing geodata.
- [ ] Editor: select, move, rotate, scale, duplicate, delete, and save/export placed map elements.
- [ ] Asset browser: list and preview static meshes, textures, packages, actors, and reusable scene elements.
- [ ] Cross-client workflow: choose a donor client and a target client, then move compatible graphical assets/elements between them.
- [ ] Dependency resolver: detect required textures, materials, meshes, animations, sounds, scripts, and package references before import.
- [ ] Compatibility gate: detect package protection, package version, missing dependencies, and unsupported object classes per client.
- [ ] Automatic geodata builder: generate geodata from edited terrain, BSP, static meshes, blocking volumes, and collision data.
- [ ] Geodata exporter: write generated geodata into a selected separate output folder.
- [ ] Batch mode: scan/export all supported client profiles through repeatable scripts.

## Feature Backlog

- [ ] Add real in-app client source selector with recent profiles.
- [ ] Add separate geodata output folder selector.
- [ ] Add source/donor client selector for importing assets from another client.
- [ ] Add package tree panel for `Maps`, `StaticMeshes`, `Textures`, `SysTextures`, `Animations`, `Sounds`, and `System`.
- [ ] Add map tile grid with present/missing/decodable/loaded/error states.
- [ ] Add object property inspector for selected actors, BSP surfaces, meshes, materials, and collision.
- [ ] Add editor transform gizmos or numeric transform fields.
- [ ] Add undo/redo command stack for editor operations.
- [ ] Add asset import plan preview before copying anything into a target workspace.
- [ ] Add dependency copy/export staging folder so target clients are not modified blindly.
- [ ] Add geodata preview overlay for generated output before saving.
- [ ] Add export report with edited tiles, imported assets, missing assets, generated geodata files, and warnings.

## Done

- [x] Restore matched runtime DLL set so `MyGUIEngine.dll` and `freetype.dll` load together.
- [x] Verify `freetype.dll` contains `FT_Get_WinFNT_Header`.
- [x] Keep local `l2encdec` beside the viewer runtime for protected client packages.
- [x] Add runtime/dependency bootstrap path for clean workspaces.
- [x] Add configurable client root through `--client=`, `--client-path=`, `/client=`, or `SEDONA_L2_CLIENT`.
- [x] Add configurable geodata root through `--geodata=`, `--geodata-path=`, `/geodata=`, or `SEDONA_L2_GEODATA`.
- [x] Guard missing map packages with a warning instead of crashing during tile load.
- [x] Add profile launcher for H5, Fafurion, and Homunculus client folders from `C:\GITHUB\L2Modder_V2`.
- [x] Add client scanner for package-count checks before loading a profile.
- [x] Treat Interlude as the known baseline and keep this phase focused on H5, Fafurion, and Homonkulus.
- [x] Add `--profile=H5|Fafurion|Homonkulus` and `SEDONA_L2_PROFILE` support.
- [x] Show the active profile and client path in the window title.
- [x] Add all-profile compatibility scan for the three sample clients.
- [x] Add tile package smoke test for H5, Fafurion, and Homonkulus sample clients.
- [x] Guard package open failures before reading package headers.
- [x] Smoke-test `23_22` and `22_22` on H5, Fafurion, and Homonkulus.
- [x] Confirm sample map tiles use `Lineage2Ver111` and the internal XOR loader path.
- [x] Map the `C:\GITHUB\L2Modder_V2` toolbox into a reusable toolchain manifest.
- [x] Add package-header smoke checks for map, staticmesh, systexture, and texture samples.
- [x] Confirm representative H5/Fafurion/Homonkulus maps, staticmeshes, systextures, and textures are internal-XOR ready.
- [x] Document the `L2Modder_V2` decoding, asset export, editor, and geodata references in `L2MODDER_INTEGRATION.md`.
- [x] Wrap the `L2AssetViewer` package index tool as a Sedona asset probe.
- [x] Smoke-test Java package indexing for H5, Fafurion, and Homonkulus map/staticmesh/systexture samples.

## Current Sprint

- [x] Add target/donor client model to config before implementing asset transfer.
- [x] Add geodata export output path to config and GUI.
- [x] Move client, donor, export, and profile controls into the top Client menu.
- [x] Keep the separate Editor Tools window focused on map/editor actions.
- [x] Add on-demand Client Status window so long paths do not occupy the workspace.
- [x] Add top-menu target and donor profile presets for H5, Fafurion, and Homonkulus.
- [x] Add top-menu asset staging folder selector for cross-client import/export preparation.
- [x] Add asset staging report script before real client file copying is implemented.
- [x] Connect an on-demand GUI staging report preview to the top Client menu.
- [x] Add package-level donor candidates and dependency warnings to the staging report preview.
- [x] Add exact dependency extraction report for selected packages.
- [x] Connect dependency extraction to a staged import preview script.
- [x] Add dry-run copy manifest generation for packages marked `NeedsStaging`.
- [x] Guard the Java asset probe wrapper against concurrent compile races.
- [x] Add opt-in staging copy command guarded by the dry-run manifest.
- [x] Add staged import copy report to the GUI preview flow.

## Next

- [ ] Add recent-client history behind the top Client menu.
- [ ] Design the editor data model: original map data, staged edits, imported assets, generated geodata.
- [ ] Implement a Sedona geodata writer from the Mobius `Region`/`IBlock` layout.
- [ ] Wrap `L2AssetViewer` texture export tools as Sedona texture preview probes.
- [ ] Convert package probe JSON into a persistent asset/dependency cache for the GUI.
- [ ] Harden texture/staticmesh object deserialize failures into warnings during real tile load.

## Usage

```powershell
.\build\Sedona-L2MapViewer.exe --client="C:\Path\To\LineageII" --geodata="C:\Path\To\geodata"
```

Profile launcher:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\Launch-ClientProfile.ps1 -Profile H5
powershell -ExecutionPolicy Bypass -File .\scripts\Launch-ClientProfile.ps1 -Profile Fafurion
powershell -ExecutionPolicy Bypass -File .\scripts\Launch-ClientProfile.ps1 -Profile Homonkulus
```

Client scan:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\Scan-Client.ps1 -Profile All
powershell -ExecutionPolicy Bypass -File .\scripts\Scan-Client.ps1 -Profile H5
powershell -ExecutionPolicy Bypass -File .\scripts\Test-ClientTiles.ps1 -Profile All
powershell -ExecutionPolicy Bypass -File .\scripts\Test-ClientAssets.ps1 -Profile All
powershell -ExecutionPolicy Bypass -File .\scripts\Get-L2ModderToolchain.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\Test-L2AssetProbe.ps1 -Profile All
powershell -ExecutionPolicy Bypass -File .\scripts\Invoke-L2AssetProbe.ps1 -Profile Fafurion -Kind Maps -Package 23_22.unr -Contains StaticMeshActor
powershell -ExecutionPolicy Bypass -File .\scripts\New-AssetStagingReport.ps1 -TargetProfile H5 -DonorProfile Fafurion
powershell -ExecutionPolicy Bypass -File .\scripts\New-AssetDependencyReport.ps1 -Profile Fafurion -Kind Maps -Package 23_22.unr
powershell -ExecutionPolicy Bypass -File .\scripts\New-StagedImportPreview.ps1 -TargetProfile H5 -DonorProfile Fafurion -Kind Maps -Package 23_22.unr
powershell -ExecutionPolicy Bypass -File .\scripts\Invoke-StagedImportCopy.ps1 -TargetProfile H5 -DonorProfile Fafurion -Kind Maps -Package 23_22.unr
```

Environment-variable mode:

```powershell
$env:SEDONA_L2_CLIENT="C:\Path\To\LineageII"
$env:SEDONA_L2_GEODATA="C:\Path\To\geodata"
.\build\Sedona-L2MapViewer.exe
```
