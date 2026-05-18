# Sedona Editor Data Model

This model separates loaded client data from staged editor work. The viewer can keep reading original Lineage II packages through `ULevel`, `AActor`, `UStaticMeshActor`, terrain, BSP, and geodata classes, while the editor records changes in a Sedona-owned layer.

## Layers

- Original map data: immutable package state loaded from the selected target client.
- Staged edits: transform, add, delete, and import commands that can later become undo/redo entries.
- Imported assets: donor-client packages copied or planned for the staging folder.
- Generated geodata: dirty tile state and export readiness for separate geodata output.

## Current C++ Anchor

The first concrete anchor is `src/editor/SedonaEditorModel.*`.

- `SedonaLoadedTile` tracks map tile identity, source client, and geodata state.
- `SedonaImportedAsset` tracks donor/staging/target package movement without modifying the client directly.
- `SedonaEditCommand` tracks staged actor and asset operations.
- `SedonaGeodataExportPlan` tracks generated tile count and output folder readiness.

## Next Connections

- Register loaded tiles from `L2UIMap::loadTileArea` or `L2LevelManager::loadTiles`.
- Push selection and transform edits into `SedonaEditCommand`.
- Feed imported asset copy results from `Invoke-StagedImportCopy.ps1` into `SedonaImportedAsset`.
- Let the geodata writer consume dirty loaded tiles instead of scanning the whole client blindly.
