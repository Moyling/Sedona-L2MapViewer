#pragma once

#include "../_afx.h"
#include "../editor/SedonaEditorModel.h"
#include "SedonaGeodataWriter.h"
#include <vector>

struct SedonaGeodataExportEntry
{
	int MapX;
	int MapY;
	char OutputPath[CM_SYSTEM_MAXNAME];
	char Message[256];
	bool Written;

	SedonaGeodataExportEntry();
};

struct SedonaGeodataExportReport
{
	int DirtyTiles;
	int WrittenFiles;
	int SkippedTiles;
	char OutputDir[CM_SYSTEM_MAXNAME];
	char Message[256];
	std::vector<SedonaGeodataExportEntry> Entries;

	SedonaGeodataExportReport();
};

class SedonaGeodataExport
{
public:
	static bool ExportDirtyFlatRegions(const SedonaEditorModel& model, int16 defaultHeight, SedonaGeodataExportReport& report);

private:
	static void BuildOutputPath(char* dest, size_t destSize, const char* outputDir, int mapX, int mapY);
};
