#include "SedonaGeodataExport.h"

static void CopyExportString(char* dest, size_t destSize, const char* value)
{
	if(!dest || destSize == 0)
		return;

	if(value)
		strcpy_s(dest, destSize, value);
	else
		dest[0] = 0;
}

SedonaGeodataExportEntry::SedonaGeodataExportEntry()
{
	MapX = 0;
	MapY = 0;
	OutputPath[0] = 0;
	Message[0] = 0;
	Written = false;
}

SedonaGeodataExportReport::SedonaGeodataExportReport()
{
	DirtyTiles = 0;
	WrittenFiles = 0;
	SkippedTiles = 0;
	OutputDir[0] = 0;
	Message[0] = 0;
}

bool SedonaGeodataExport::ExportDirtyFlatRegions(const SedonaEditorModel& model, int16 defaultHeight, SedonaGeodataExportReport& report)
{
	report = SedonaGeodataExportReport();

	const SedonaGeodataExportPlan& plan = model.GetGeodataExportPlan();
	CopyExportString(report.OutputDir, sizeof(report.OutputDir), plan.OutputDir);

	if(!plan.OutputDir[0])
	{
		CopyExportString(report.Message, sizeof(report.Message), "No geodata export folder is selected.");
		return false;
	}

	bool allWritten = true;
	for(int i = 0; i < model.GetLoadedTileCount(); i++)
	{
		const SedonaLoadedTile* tile = model.GetLoadedTile(i);
		if(!tile || !tile->HasGeneratedGeodata)
			continue;

		report.DirtyTiles++;
		SedonaGeodataExportEntry entry;
		entry.MapX = tile->MapX;
		entry.MapY = tile->MapY;
		BuildOutputPath(entry.OutputPath, sizeof(entry.OutputPath), plan.OutputDir, tile->MapX, tile->MapY);

		SedonaGeodataWriter writer;
		writer.ResetFlat(defaultHeight);
		if(writer.SaveToFile(entry.OutputPath))
		{
			entry.Written = true;
			report.WrittenFiles++;
			CopyExportString(entry.Message, sizeof(entry.Message), "Flat placeholder geodata written. Collision sampling is not connected yet.");
		}
		else
		{
			entry.Written = false;
			report.SkippedTiles++;
			allWritten = false;
			CopyExportString(entry.Message, sizeof(entry.Message), writer.GetLastError());
		}

		report.Entries.push_back(entry);
	}

	if(report.DirtyTiles == 0)
	{
		CopyExportString(report.Message, sizeof(report.Message), "No dirty geodata tiles are ready for export.");
		return false;
	}

	if(allWritten)
		CopyExportString(report.Message, sizeof(report.Message), "Dirty geodata tiles exported.");
	else
		CopyExportString(report.Message, sizeof(report.Message), "Some geodata tiles failed to export.");

	return allWritten;
}

void SedonaGeodataExport::BuildOutputPath(char* dest, size_t destSize, const char* outputDir, int mapX, int mapY)
{
	if(!dest || destSize == 0)
		return;

	if(!outputDir || !outputDir[0])
	{
		dest[0] = 0;
		return;
	}

	size_t len = strlen(outputDir);
	if(len > 0 && (outputDir[len - 1] == '\\' || outputDir[len - 1] == '/'))
		sprintf_s(dest, destSize, "%s%d_%d.l2j", outputDir, mapX, mapY);
	else
		sprintf_s(dest, destSize, "%s\\%d_%d.l2j", outputDir, mapX, mapY);
}
