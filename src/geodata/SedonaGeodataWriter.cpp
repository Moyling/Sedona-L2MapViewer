#include "SedonaGeodataWriter.h"

SedonaGeoCell::SedonaGeoCell()
{
	Height = 0;
	Nswe = SedonaGeodataWriter::NSWE_ALL;
}

SedonaGeoBlock::SedonaGeoBlock()
{
	Type = SEDONA_GEO_BLOCK_FLAT;
	FlatHeight = 0;
	for(int i = 0; i < SedonaGeodataWriter::BLOCK_CELLS; i++)
		Cells[i] = SedonaGeoCell();
}

SedonaGeodataWriter::SedonaGeodataWriter()
{
	m_lastError[0] = 0;
	m_blocks.resize(REGION_BLOCKS);
	ResetFlat(0);
}

void SedonaGeodataWriter::ResetFlat(int16 height, uint8 nswe)
{
	m_lastError[0] = 0;
	for(int i = 0; i < REGION_BLOCKS; i++)
	{
		m_blocks[i] = SedonaGeoBlock();
		m_blocks[i].FlatHeight = height;
		for(int cell = 0; cell < BLOCK_CELLS; cell++)
		{
			m_blocks[i].Cells[cell].Height = height;
			m_blocks[i].Cells[cell].Nswe = nswe & NSWE_ALL;
		}
	}
}

bool SedonaGeodataWriter::SetFlatBlock(int blockX, int blockY, int16 height)
{
	if(!IsValidBlock(blockX, blockY))
	{
		SetLastError("Flat block coordinate is outside the 256x256 region.");
		return false;
	}

	SedonaGeoBlock& block = m_blocks[BlockIndex(blockX, blockY)];
	block.Type = SEDONA_GEO_BLOCK_FLAT;
	block.FlatHeight = height;
	for(int i = 0; i < BLOCK_CELLS; i++)
	{
		block.Cells[i].Height = height;
		block.Cells[i].Nswe = NSWE_ALL;
	}
	return true;
}

bool SedonaGeodataWriter::SetComplexCell(int geoX, int geoY, int16 height, uint8 nswe)
{
	if(!IsValidCell(geoX, geoY))
	{
		SetLastError("Geodata cell coordinate is outside the 2048x2048 region.");
		return false;
	}

	int blockX = geoX / BLOCK_CELLS_X;
	int blockY = geoY / BLOCK_CELLS_Y;
	int localX = geoX % BLOCK_CELLS_X;
	int localY = geoY % BLOCK_CELLS_Y;
	int cellIndex = localX * BLOCK_CELLS_Y + localY;

	SedonaGeoBlock& block = m_blocks[BlockIndex(blockX, blockY)];
	if(block.Type == SEDONA_GEO_BLOCK_FLAT)
		ConvertFlatBlockToComplex(block);

	if(block.Type != SEDONA_GEO_BLOCK_COMPLEX)
	{
		SetLastError("Only flat and complex geodata blocks are writable in this phase.");
		return false;
	}

	block.Cells[cellIndex].Height = height;
	block.Cells[cellIndex].Nswe = nswe & NSWE_ALL;
	return true;
}

bool SedonaGeodataWriter::SaveToFile(const char* path) const
{
	if(!path || !path[0])
	{
		SetLastError("Missing geodata output path.");
		return false;
	}

	if(!EnsureParentDirectory(path))
	{
		SetLastError("Could not create geodata output folder.");
		return false;
	}

	FILE* file = fopen(path, "wb");
	if(!file)
	{
		SetLastError("Could not open geodata output file.");
		return false;
	}

	for(int blockIndex = 0; blockIndex < REGION_BLOCKS; blockIndex++)
	{
		const SedonaGeoBlock& block = m_blocks[blockIndex];
		uint8 type = (uint8)block.Type;
		fwrite(&type, 1, 1, file);

		if(block.Type == SEDONA_GEO_BLOCK_FLAT)
		{
			WriteUInt16LE(file, (uint16)block.FlatHeight);
		}
		else if(block.Type == SEDONA_GEO_BLOCK_COMPLEX)
		{
			for(int cell = 0; cell < BLOCK_CELLS; cell++)
				WriteUInt16LE(file, EncodeComplexCell(block.Cells[cell].Height, block.Cells[cell].Nswe));
		}
		else
		{
			fclose(file);
			SetLastError("Multilayer geodata writing is not enabled yet.");
			return false;
		}
	}

	fclose(file);
	return true;
}

bool SedonaGeodataWriter::SaveRegionToFolder(const char* outputDir, int mapX, int mapY) const
{
	if(!outputDir || !outputDir[0])
	{
		SetLastError("Missing geodata output folder.");
		return false;
	}

	char path[CM_SYSTEM_MAXNAME];
	size_t len = strlen(outputDir);
	if(len > 0 && (outputDir[len - 1] == '\\' || outputDir[len - 1] == '/'))
		sprintf_s(path, sizeof(path), "%s%d_%d.l2j", outputDir, mapX, mapY);
	else
		sprintf_s(path, sizeof(path), "%s\\%d_%d.l2j", outputDir, mapX, mapY);
	return SaveToFile(path);
}

const char* SedonaGeodataWriter::GetLastError() const
{
	return m_lastError;
}

uint16 SedonaGeodataWriter::EncodeComplexCell(int16 height, uint8 nswe)
{
	uint16 encodedHeight = (uint16)(height * 2);
	return (uint16)((encodedHeight & 0xfff0) | (nswe & NSWE_ALL));
}

bool SedonaGeodataWriter::IsValidBlock(int blockX, int blockY)
{
	return blockX >= 0 && blockX < REGION_BLOCKS_X && blockY >= 0 && blockY < REGION_BLOCKS_Y;
}

bool SedonaGeodataWriter::IsValidCell(int geoX, int geoY)
{
	return geoX >= 0 && geoX < REGION_BLOCKS_X * BLOCK_CELLS_X && geoY >= 0 && geoY < REGION_BLOCKS_Y * BLOCK_CELLS_Y;
}

bool SedonaGeodataWriter::EnsureParentDirectory(const char* path)
{
	char folder[CM_SYSTEM_MAXNAME];
	strcpy_s(folder, sizeof(folder), path);

	char* slash = strrchr(folder, '\\');
	char* forwardSlash = strrchr(folder, '/');
	if(forwardSlash && (!slash || forwardSlash > slash))
		slash = forwardSlash;
	if(!slash)
		return true;

	*slash = 0;
	if(folder[0] == 0)
		return true;

	for(char* cursor = folder; *cursor; cursor++)
	{
		if((*cursor == '\\' || *cursor == '/') && cursor > folder && cursor[-1] != ':')
		{
			char saved = *cursor;
			*cursor = 0;
			CreateDirectoryA(folder, 0);
			*cursor = saved;
		}
	}

	return CreateDirectoryA(folder, 0) || ::GetLastError() == ERROR_ALREADY_EXISTS;
}

void SedonaGeodataWriter::WriteUInt16LE(FILE* file, uint16 value)
{
	uint8 bytes[2];
	bytes[0] = (uint8)(value & 0xff);
	bytes[1] = (uint8)((value >> 8) & 0xff);
	fwrite(bytes, 1, 2, file);
}

int SedonaGeodataWriter::BlockIndex(int blockX, int blockY) const
{
	return blockX * REGION_BLOCKS_Y + blockY;
}

void SedonaGeodataWriter::ConvertFlatBlockToComplex(SedonaGeoBlock& block)
{
	block.Type = SEDONA_GEO_BLOCK_COMPLEX;
	for(int i = 0; i < BLOCK_CELLS; i++)
	{
		block.Cells[i].Height = block.FlatHeight;
		block.Cells[i].Nswe = NSWE_ALL;
	}
}

void SedonaGeodataWriter::SetLastError(const char* message) const
{
	if(message)
		strcpy_s(m_lastError, sizeof(m_lastError), message);
	else
		m_lastError[0] = 0;
}
