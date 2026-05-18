#include "SedonaEditorModel.h"

static void CopyModelString(char* dest, size_t destSize, const char* value)
{
	if(!dest || destSize == 0)
		return;

	if(value)
		strcpy_s(dest, destSize, value);
	else
		dest[0] = 0;
}

SedonaActorTransform::SedonaActorTransform()
{
	Location = UVector(0, 0, 0);
	Rotation = URotator(0, 0, 0);
	DrawScale3D = UVector(1, 1, 1);
	DrawScale = 1.0f;
}

SedonaLoadedTile::SedonaLoadedTile()
{
	MapX = 0;
	MapY = 0;
	PackageName[0] = 0;
	SourceClient[0] = 0;
	HasOriginalGeodata = false;
	HasGeneratedGeodata = false;
}

SedonaImportedAsset::SedonaImportedAsset()
{
	Kind = SEDONA_ASSET_MAP;
	Token[0] = 0;
	SourcePath[0] = 0;
	StagingPath[0] = 0;
	TargetPackage[0] = 0;
	CopiedToStaging = false;
}

SedonaEditCommand::SedonaEditCommand()
{
	Kind = SEDONA_EDIT_UPDATE_ACTOR_TRANSFORM;
	TileIndex = -1;
	ActorName[0] = 0;
	ActorClass[0] = 0;
	AssetToken[0] = 0;
}

SedonaGeodataExportPlan::SedonaGeodataExportPlan()
{
	OutputDir[0] = 0;
	DirtyTileCount = 0;
	BlockingActorCount = 0;
	TerrainSampleCount = 0;
	ReadyToExport = false;
}

SedonaEditorModel::SedonaEditorModel()
{
	Reset();
}

void SedonaEditorModel::Reset()
{
	m_loadedTileCount = 0;
	m_importedAssetCount = 0;
	m_editCommandCount = 0;
	m_geodataExportPlan = SedonaGeodataExportPlan();
}

int SedonaEditorModel::AddLoadedTile(int mapX, int mapY, const char* packageName, const char* sourceClient, bool hasOriginalGeodata)
{
	if(m_loadedTileCount >= MAX_LOADED_TILES)
		return -1;

	SedonaLoadedTile& tile = m_loadedTiles[m_loadedTileCount];
	tile = SedonaLoadedTile();
	tile.MapX = mapX;
	tile.MapY = mapY;
	CopyModelString(tile.PackageName, sizeof(tile.PackageName), packageName);
	CopyModelString(tile.SourceClient, sizeof(tile.SourceClient), sourceClient);
	tile.HasOriginalGeodata = hasOriginalGeodata;
	return m_loadedTileCount++;
}

int SedonaEditorModel::AddImportedAsset(SedonaAssetKind kind, const char* token, const char* sourcePath, const char* stagingPath, const char* targetPackage, bool copiedToStaging)
{
	if(m_importedAssetCount >= MAX_IMPORTED_ASSETS)
		return -1;

	SedonaImportedAsset& asset = m_importedAssets[m_importedAssetCount];
	asset = SedonaImportedAsset();
	asset.Kind = kind;
	CopyModelString(asset.Token, sizeof(asset.Token), token);
	CopyModelString(asset.SourcePath, sizeof(asset.SourcePath), sourcePath);
	CopyModelString(asset.StagingPath, sizeof(asset.StagingPath), stagingPath);
	CopyModelString(asset.TargetPackage, sizeof(asset.TargetPackage), targetPackage);
	asset.CopiedToStaging = copiedToStaging;
	return m_importedAssetCount++;
}

int SedonaEditorModel::AddEditCommand(const SedonaEditCommand& command)
{
	if(m_editCommandCount >= MAX_EDIT_COMMANDS)
		return -1;

	m_editCommands[m_editCommandCount] = command;
	if(command.TileIndex >= 0 && command.TileIndex < m_loadedTileCount)
	{
		if(!m_loadedTiles[command.TileIndex].HasGeneratedGeodata)
			m_geodataExportPlan.DirtyTileCount++;
		m_loadedTiles[command.TileIndex].HasGeneratedGeodata = true;
	}
	return m_editCommandCount++;
}

void SedonaEditorModel::MarkTileGeneratedGeodata(int mapX, int mapY)
{
	for(int i = 0; i < m_loadedTileCount; i++)
	{
		if(m_loadedTiles[i].MapX == mapX && m_loadedTiles[i].MapY == mapY)
		{
			if(!m_loadedTiles[i].HasGeneratedGeodata)
				m_geodataExportPlan.DirtyTileCount++;
			m_loadedTiles[i].HasGeneratedGeodata = true;
			m_geodataExportPlan.ReadyToExport = m_geodataExportPlan.OutputDir[0] != 0;
			return;
		}
	}
}

void SedonaEditorModel::SetGeodataExportDir(const char* outputDir)
{
	CopyModelString(m_geodataExportPlan.OutputDir, sizeof(m_geodataExportPlan.OutputDir), outputDir);
	m_geodataExportPlan.ReadyToExport = m_geodataExportPlan.OutputDir[0] != 0 && m_geodataExportPlan.DirtyTileCount > 0;
}

int SedonaEditorModel::GetLoadedTileCount() const
{
	return m_loadedTileCount;
}

int SedonaEditorModel::GetImportedAssetCount() const
{
	return m_importedAssetCount;
}

int SedonaEditorModel::GetEditCommandCount() const
{
	return m_editCommandCount;
}

const SedonaLoadedTile* SedonaEditorModel::GetLoadedTile(int index) const
{
	if(index < 0 || index >= m_loadedTileCount)
		return 0;
	return &m_loadedTiles[index];
}

const SedonaImportedAsset* SedonaEditorModel::GetImportedAsset(int index) const
{
	if(index < 0 || index >= m_importedAssetCount)
		return 0;
	return &m_importedAssets[index];
}

const SedonaEditCommand* SedonaEditorModel::GetEditCommand(int index) const
{
	if(index < 0 || index >= m_editCommandCount)
		return 0;
	return &m_editCommands[index];
}

const SedonaGeodataExportPlan& SedonaEditorModel::GetGeodataExportPlan() const
{
	return m_geodataExportPlan;
}
