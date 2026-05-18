# Sedona Geodata Writer

Sedona now has a C++ writer anchor for Lineage II region geodata: `src/geodata/SedonaGeodataWriter.*`.

## Format

The writer follows the Mobius `Region` / `IBlock` layout used by the High Five source under `C:\GITHUB\L2Modder_V2`:

- region: `256 x 256` blocks
- block: `8 x 8` cells
- flat block type: `0`, followed by one little-endian height
- complex block type: `1`, followed by 64 little-endian encoded cells
- encoded complex cell: `(height << 1) | nswe`
- NSWE flags: low 4 bits, `0x0f` means all directions open

Multilayer writing is deliberately blocked until the editor has a real collision/layer generator.

## Current Scope

- Generate a complete flat region.
- Convert individual cells to complex blocks when edited.
- Save to a selected export folder as `<mapX>_<mapY>.l2j`.
- Keep generated files separate from the selected client source.

## Next Connections

- Feed dirty tiles from `SedonaEditorModel` into this writer.
- Sample terrain/BSP/static mesh collision into complex cells.
- Add a GUI export report listing written files and blocked multilayer cases.
