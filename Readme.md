# Sedona-L2MapViewer

Sedona-L2MapViewer is a standalone Lineage II world/map viewer for inspecting client map packages, geometry, static meshes, textures, and L2J geodata from one desktop tool.

It reads Lineage II client packages directly, loads map tiles on demand, renders the UE2/BSP world geometry and static meshes, and includes an upgraded package decoding path for protected client files.

## Current Build

The Release x64 output is:

```text
build/Sedona-L2MapViewer.exe
```

## Running

The executable expects a Lineage II client folder one level above the `build` folder.

Typical layout:

```text
SomeFolder/
  build/
    Sedona-L2MapViewer.exe
    data/
    resources.xml
    ...
  Maps/
  StaticMeshes/
  SysTextures/
  Textures/
  system/
```

Optional L2J geodata layout:

```text
SomeFolder/
  build/
  geodata/
    23_22.l2j
    ...
```

Controls:

- Right mouse button: rotate camera
- Mouse wheel: zoom
- W/A/S/D: move
- Space: move up
- Ctrl: move down
- Menu -> Map: open the tile selector

## Supported Client Data

The package loader scans:

```text
Maps/*.unr
SysTextures/*.utx
Textures/*.utx
StaticMeshes/*.usx
```

Package decoding supports XOR-protected Lineage package headers internally. For newer protected files, including RSA/zlib-protected client data, the viewer falls back to `l2encdec.exe` when available.

The fallback decoder is searched in this order:

1. `L2ENCDEC_EXE` environment variable
2. next to the executable
3. `build/data/l2encdec/l2encdec.exe`
4. known local `L2Modder_V2/L2FileEdit/data/l2encdec` paths

The repository build includes:

```text
build/data/l2encdec/l2encdec.exe
build/data/l2encdec/libgmp-10.dll
build/data/l2encdec/libz-1.dll
```

## Building

Prerequisites:

- Visual Studio 2022 Build Tools with C++ workload
- Windows 10 SDK
- The bundled `deps/` folder

This workspace has been updated to use:

- MSVC toolset: `v143`
- Windows SDK: `10.0`
- local dependency paths under `deps/`
- executable target name: `Sedona-L2MapViewer.exe`

Build command:

```powershell
& 'C:\BuildTools\MSBuild\Current\Bin\MSBuild.exe' Sedona-L2MapViewer.sln /m /p:Configuration=Release /p:Platform=x64 /v:minimal
```

Successful output:

```text
build/Sedona-L2MapViewer.exe
```

Known non-fatal linker warnings:

- `LNK4098`: runtime library mix from old third-party libs
- `LNK4099`: missing `vc120.pdb` for bundled `glew32s.lib`

## Notes

The renderer still uses legacy OpenGL in several places because Lineage II's map assets are UE2-era data, but the project build, package loading, executable naming, and decoder integration have been upgraded for the Sedona toolchain.

## Gallery

![](docs/tiles1.jpg)
![](docs/tiles2.jpg)
![](docs/map.jpg)
![](docs/geo.jpg)
