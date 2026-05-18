# Sedona-L2MapViewer Tasklist

## Prototype Warning

This is not a finished release yet. The viewer starts and can load map tiles, but client source selection, automatic geodata generation, and the full asset/UI workflow are still incomplete.

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

## Next

- [ ] Add in-app profile selector after the command-line flow is stable.
- [ ] Harden texture/staticmesh failures into warnings during real tile load.
- [ ] Extend texture/staticmesh smoke checks beyond map packages.

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
```

Environment-variable mode:

```powershell
$env:SEDONA_L2_CLIENT="C:\Path\To\LineageII"
$env:SEDONA_L2_GEODATA="C:\Path\To\geodata"
.\build\Sedona-L2MapViewer.exe
```
