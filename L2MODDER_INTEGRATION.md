# L2Modder_V2 Integration Map

`C:\GITHUB\L2Modder_V2` is the reference toolbox for Sedona's decoding, asset export, editor, and geodata work.

## Source Roots

- `L2FileEdit`: external package/dat helper tools, including `l2encdec`.
- `L2ClientDat`: Java client `.dat` open/save/encrypt/decrypt workflow.
- `L2AssetViewer`: Unreal package index, texture extraction, mesh/effect probing, and GLB conversion pipeline.
- `l2w_tool`: Rust UE2 read/write helpers plus map/spawn/entity editor patterns.
- `source/L2J_Mobius_CT_2.6_HighFive`: server-side geodata reference implementation.
- `L2Editor`: original UE2 editor runtime and editor resources, useful as UX/reference material.
- `tools/umodel`: UE Viewer/UModel executables for PSK/PSA/static mesh export fallback.

## Decoding And Encoding References

- `L2FileEdit/data/l2encdec/l2encdec.exe`
  - Existing external fallback for protected Unreal package files.
  - Sedona already bundles this under `third_party/l2encdec`.
- `L2ClientDat/java/org/l2jmobius/clientcryptor/DatFile.java`
  - Writes `Lineage2Ver###` headers and calls the selected crypter for save/export.
- `L2ClientDat/java/org/l2jmobius/actions/OpenDat.java`
  - Detects `Lineage2Ver###` and chooses a matching decryptor.
- `L2ClientDat/java/org/l2jmobius/actions/SaveDat.java`
  - Packs text/structured data back into encrypted `.dat`.
- `L2ClientDat/java/org/l2jmobius/xml/CryptVersionParser.java`
  - Registry for available encrypt/decrypt keys.
- `L2ClientDat/java/org/l2jmobius/clientcryptor/crypt/XorDatCrypter.java`
  - XOR path used by older/internal-ready files.
- `L2ClientDat/java/org/l2jmobius/clientcryptor/crypt/RSADatCrypter.java`
  - RSA/zlib path for newer `.dat` files.
- `L2ClientDat/java/org/l2jmobius/util/ByteReader.java` and `ByteWriter.java`
  - Binary read/write primitives for client data export.

Sedona implementation rule: keep destructive writes out of source clients. Decode/import into a staging folder, validate, then export/copy into a target workspace only after dependency checks pass.

## Unreal Package And Asset Export References

- `L2AssetViewer/tools/UnrealPackageIndexJson.java`
  - Uses `acmi.l2.clientmod.io.UnrealPackage` to emit package metadata, imports, and exports.
- `L2AssetViewer/tools/UnrealPackageDump.java`
  - Human-readable package dump for quick inspection.
- `L2AssetViewer/tools/TextureProbe.java`
  - Resolves texture objects through a client-root environment.
- `L2AssetViewer/tools/TextureExtract.java`
  - Extracts texture exports to PNG.
- `L2AssetViewer/tools/UnrealEffectMeshRefs.java`
  - Finds mesh references inside effect packages.
- `L2AssetViewer/tools/UnrealEffectMeshProbe.java`
  - Probes effect mesh data against a client root.
- `L2AssetViewer/tools/convert_psk_to_glb.py`
  - Blender-based PSK/PSA to GLB conversion.
- `L2AssetViewer/tools/convert_unreal_3d_to_glb.py`
  - Unreal `.3d` model to GLB conversion.
- `tools/umodel/umodel_64.exe`
  - Fallback exporter for PSK/PSA/textures when Java indexing is not enough.

Sedona implementation rule: asset migration must copy the selected export plus all referenced dependencies: textures, systextures, materials, mesh packages, animations, sounds, and system/script references where applicable.

## UE2 Read/Write And Editor References

- `l2w_tool/l2_rw/src/ue2_rw.rs`
  - Rust UE2 compact int, string, matrix, vector, read/write traits.
- `l2w_tool/l2_rw/src/lib.rs`
  - DAT deserialize/save helpers and XOR handling.
- `l2w_tool/spawn_editor/src/backend.rs`
  - World coordinate to map-square conversion and spawn indexing.
- `l2w_tool/spawn_editor/src/frontend.rs`
  - Existing map editor UI pattern.
- `l2w_tool/tool/src/backend/dat_loader/*`
  - Client DAT entity readers for NPCs, skills, items, regions, maps, and names.

Sedona implementation rule: the first editor save format should be a Sedona staging manifest, not direct `.unr` rewriting. Direct package mutation comes after round-trip tests are reliable.

## Geodata References

- `source/L2J_Mobius_CT_2.6_HighFive/java/org/l2jmobius/gameserver/geoengine/GeoEngine.java`
  - Region naming, loading, coordinate scaling, movement and LOS checks.
- `source/L2J_Mobius_CT_2.6_HighFive/java/org/l2jmobius/gameserver/geoengine/geodata/Cell.java`
  - NSWE constants.
- `source/L2J_Mobius_CT_2.6_HighFive/java/org/l2jmobius/gameserver/geoengine/geodata/IBlock.java`
  - Block constants and block geometry contract.
- `source/L2J_Mobius_CT_2.6_HighFive/java/org/l2jmobius/gameserver/geoengine/geodata/IRegion.java`
  - Region/block dimensions.
- `source/L2J_Mobius_CT_2.6_HighFive/java/org/l2jmobius/gameserver/geoengine/geodata/regions/Region.java`
  - `.l2j` read/write layout and `saveToFile`.
- `source/L2J_Mobius_CT_2.6_HighFive/java/org/l2jmobius/gameserver/geoengine/geodata/blocks/FlatBlock.java`
  - Flat block read/write model.
- `source/L2J_Mobius_CT_2.6_HighFive/java/org/l2jmobius/gameserver/geoengine/geodata/blocks/ComplexBlock.java`
  - 64-cell complex block model.
- `source/L2J_Mobius_CT_2.6_HighFive/java/org/l2jmobius/gameserver/geoengine/geodata/blocks/MultilayerBlock.java`
  - Multi-layer cell model.
- `src/L2Geodata.cpp`
  - Current Sedona `.l2j` loader/preview path.

Known `.l2j` layout:

- Region file name: `%d_%d.l2j`.
- Region contains `256 * 256` blocks.
- Block type `0`: flat block, followed by one little-endian `short` height.
- Block type `1`: complex block, followed by `64` little-endian `short` cell entries.
- Block type `2`: multilayer block, followed by 64 variable cells. Each cell starts with a layer count byte, then that many little-endian `short` entries.
- Complex/multilayer cell entry packs height and NSWE. NSWE is low 4 bits; height is the remaining bits shifted right by 1.
- NSWE flags: east `1`, west `2`, south `4`, north `8`, all `15`.

Sedona implementation rule: geodata generation should create an in-memory `Region`-like model first, preview it, validate movement/LOS samples, and only then export `.l2j` files to a selected output directory.

## Builder Pipeline

1. Select target client.
2. Select optional donor client.
3. Scan target and donor package indexes.
4. Decode protected packages into staging as needed.
5. Build an asset dependency graph.
6. Preview import plan and conflicts.
7. Copy/export compatible dependencies into a staging output tree.
8. Load/edit map tile in Sedona.
9. Generate geodata from terrain, BSP, static meshes, collision, and blocking volumes.
10. Preview generated geodata overlay.
11. Export `.l2j` files to a separate selected geodata output folder.
12. Write an export report with warnings, skipped assets, imported assets, edited tiles, and generated geodata files.

