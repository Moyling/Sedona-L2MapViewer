#pragma once

#include "../_afx.h"
#include <vector>

enum SedonaGeoBlockType
{
	SEDONA_GEO_BLOCK_FLAT = 0,
	SEDONA_GEO_BLOCK_COMPLEX = 1,
	SEDONA_GEO_BLOCK_MULTILAYER = 2
};

struct SedonaGeoCell
{
	int16 Height;
	uint8 Nswe;

	SedonaGeoCell();
};

struct SedonaGeoBlock
{
	SedonaGeoBlockType Type;
	int16 FlatHeight;
	SedonaGeoCell Cells[64];

	SedonaGeoBlock();
};

class SedonaGeodataWriter
{
public:
	static const int REGION_BLOCKS_X = 256;
	static const int REGION_BLOCKS_Y = 256;
	static const int REGION_BLOCKS = REGION_BLOCKS_X * REGION_BLOCKS_Y;
	static const int BLOCK_CELLS_X = 8;
	static const int BLOCK_CELLS_Y = 8;
	static const int BLOCK_CELLS = BLOCK_CELLS_X * BLOCK_CELLS_Y;
	static const uint8 NSWE_ALL = 0x0f;

	SedonaGeodataWriter();

	void ResetFlat(int16 height, uint8 nswe = NSWE_ALL);
	bool SetFlatBlock(int blockX, int blockY, int16 height);
	bool SetComplexCell(int geoX, int geoY, int16 height, uint8 nswe);
	bool SaveToFile(const char* path) const;
	bool SaveRegionToFolder(const char* outputDir, int mapX, int mapY) const;

	const char* GetLastError() const;

private:
	static uint16 EncodeComplexCell(int16 height, uint8 nswe);
	static bool IsValidBlock(int blockX, int blockY);
	static bool IsValidCell(int geoX, int geoY);
	static bool EnsureParentDirectory(const char* path);
	static void WriteUInt16LE(FILE* file, uint16 value);
	int BlockIndex(int blockX, int blockY) const;
	void ConvertFlatBlockToComplex(SedonaGeoBlock& block);
	void SetLastError(const char* message) const;

	std::vector<SedonaGeoBlock> m_blocks;
	mutable char m_lastError[256];
};
