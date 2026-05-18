#pragma once

#include "../_afx.h"

enum SedonaEditKind
{
	SEDONA_EDIT_ADD_ACTOR = 0,
	SEDONA_EDIT_UPDATE_ACTOR_TRANSFORM,
	SEDONA_EDIT_DELETE_ACTOR,
	SEDONA_EDIT_IMPORT_ASSET,
	SEDONA_EDIT_GEODATA_MARK
};

enum SedonaAssetKind
{
	SEDONA_ASSET_MAP = 0,
	SEDONA_ASSET_STATIC_MESH,
	SEDONA_ASSET_SYS_TEXTURE,
	SEDONA_ASSET_TEXTURE,
	SEDONA_ASSET_ANIMATION,
	SEDONA_ASSET_SOUND,
	SEDONA_ASSET_SYSTEM
};

struct SedonaActorTransform
{
	UVector Location;
	URotator Rotation;
	UVector DrawScale3D;
	float DrawScale;

	SedonaActorTransform();
};

struct SedonaLoadedTile
{
	int MapX;
	int MapY;
	char PackageName[64];
	char SourceClient[CM_SYSTEM_MAXNAME];
	bool HasOriginalGeodata;
	bool HasGeneratedGeodata;

	SedonaLoadedTile();
};

struct SedonaImportedAsset
{
	SedonaAssetKind Kind;
	char Token[128];
	char SourcePath[CM_SYSTEM_MAXNAME];
	char StagingPath[CM_SYSTEM_MAXNAME];
	char TargetPackage[128];
	bool CopiedToStaging;

	SedonaImportedAsset();
};

struct SedonaEditCommand
{
	SedonaEditKind Kind;
	int TileIndex;
	char ActorName[128];
	char ActorClass[128];
	char AssetToken[128];
	SedonaActorTransform Before;
	SedonaActorTransform After;

	SedonaEditCommand();
};

struct SedonaGeodataExportPlan
{
	char OutputDir[CM_SYSTEM_MAXNAME];
	int DirtyTileCount;
	int BlockingActorCount;
	int TerrainSampleCount;
	bool ReadyToExport;

	SedonaGeodataExportPlan();
};

class SedonaEditorModel
{
public:
	SedonaEditorModel();

	void Reset();
	int AddLoadedTile(int mapX, int mapY, const char* packageName, const char* sourceClient, bool hasOriginalGeodata);
	int AddImportedAsset(SedonaAssetKind kind, const char* token, const char* sourcePath, const char* stagingPath, const char* targetPackage, bool copiedToStaging);
	int AddEditCommand(const SedonaEditCommand& command);
	void MarkTileGeneratedGeodata(int mapX, int mapY);
	void SetGeodataExportDir(const char* outputDir);

	int GetLoadedTileCount() const;
	int GetImportedAssetCount() const;
	int GetEditCommandCount() const;
	const SedonaLoadedTile* GetLoadedTile(int index) const;
	const SedonaImportedAsset* GetImportedAsset(int index) const;
	const SedonaEditCommand* GetEditCommand(int index) const;
	const SedonaGeodataExportPlan& GetGeodataExportPlan() const;

private:
	static const int MAX_LOADED_TILES = 256;
	static const int MAX_IMPORTED_ASSETS = 2048;
	static const int MAX_EDIT_COMMANDS = 4096;

	SedonaLoadedTile m_loadedTiles[MAX_LOADED_TILES];
	SedonaImportedAsset m_importedAssets[MAX_IMPORTED_ASSETS];
	SedonaEditCommand m_editCommands[MAX_EDIT_COMMANDS];
	SedonaGeodataExportPlan m_geodataExportPlan;
	int m_loadedTileCount;
	int m_importedAssetCount;
	int m_editCommandCount;
};
