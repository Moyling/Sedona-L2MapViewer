# Sedona-L2MapViewer Prototype Audit

## Status

This is a working prototype, not a finished Sedona release.

The executable can start, scan Lineage II package folders, decode protected packages through the integrated fallback, and load map tiles through the existing map window. The current UI is still thin: most engine capability is present in code but not exposed as a usable workflow.

## Current Capability Map

- Runtime: native Win32/OpenGL application with MyGUI.
- Package layer: scans `Maps`, `SysTextures`, `Textures`, and `StaticMeshes`.
- Decode layer: internal Lineage XOR path plus external `l2encdec` fallback.
- Rendering: skybox, BSP, terrain sectors, static mesh actors, textures/materials, water/semisolid handling.
- Geodata: can load existing `.l2j` geodata and render an overlay.
- Interaction: camera movement, map tile selector, tile load/hide, BSP toggle, object/BSP picking outline.
- Tooling: runtime bootstrap, profile launcher, client scanner.

## Defect List

- P0: The app is still a prototype and must be labeled as such.
- P0: Client source selection is not complete. Command-line/profile support exists, but the GUI needs a real selector.
- P0: Geodata is not generated automatically. The app only loads existing `.l2j` files.
- P1: The GUI exposes only a small part of the engine capabilities.
- P1: Map loading is still centered on hard-coded/default tiles like `23_22`.
- P1: Asset inspection exists internally through pick logic, but no useful property panel is active.
- P1: Errors from texture/staticmesh/package loading need to become visible non-fatal status entries.
- P1: Runtime dependencies are bundled but still legacy and should be replaced or documented per architecture.
- P2: The MyGUI layout is mostly programmatic and rough.
- P2: Several renderer paths are old immediate-mode OpenGL and need cleanup after feature parity is reached.

## Solution Plan

- Make the prototype state explicit in README and tasklist.
- Add an in-app client source selector that restarts the viewer with the selected client path/profile.
- Add a richer right-side tool panel: profile/client/geodata status, quick map loads, map visibility, BSP, and later inspection panels.
- Add a package/client scan panel so the user can see what the selected client actually contains.
- Add a geodata workflow in two stages: first detect missing geodata clearly, then add generation/export.
- Turn actor/BSP picking into a property panel with name, class, location, material, mesh, and package information.
- Keep every stage buildable and pushed as prototype milestones.

## Execution Plan

1. Prototype checkpoint commit with runtime bootstrap, profile support, and current GUI tool panel.
2. Implement GUI client selection and profile restart.
3. Implement scan/status panel for selected client package counts.
4. Implement asset/property inspector using existing pick code.
5. Implement geodata generation pipeline or integrate a proven generator from `L2Modder_V2` if available.
6. Replace hard-coded map defaults with user-selected tile/profile defaults.
7. Promote from prototype only when client selection, map loading, inspection, and geodata workflow are all usable.
