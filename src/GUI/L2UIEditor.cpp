#include "L2UIEditor.h"

#include "../main.h"
#include "L2UIMap.h"
#include <shlobj.h>
#include <io.h>

static bool FileExistsA(const char* path)
{
	return path && path[0] && _access(path, 0) == 0;
}

static void NormalizeHistoryPath(char* path)
{
	if(!path)
		return;

	size_t len = strlen(path);
	for(size_t i = 0; i < len; i++)
	{
		if(path[i] == '\\')
			path[i] = '/';
	}

	len = strlen(path);
	if(len > 0 && path[len - 1] != '/' && len + 1 < CM_SYSTEM_MAXNAME)
	{
		path[len] = '/';
		path[len + 1] = 0;
	}
}

static void JoinPathA(char* out, size_t outSize, const char* left, const char* right)
{
	if(!out || outSize == 0)
		return;

	out[0] = 0;
	if(!left || !right)
		return;

	strcpy_s(out, outSize, left);
	size_t len = strlen(out);
	if(len > 0 && out[len - 1] != '\\' && out[len - 1] != '/')
		strcat_s(out, outSize, "\\");
	strcat_s(out, outSize, right);
}

static bool GetRecentClientHistoryPath(char* out, size_t outSize)
{
	if(!out || outSize == 0)
		return false;

	char localAppData[CM_SYSTEM_MAXNAME];
	if(FAILED(SHGetFolderPathA(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, localAppData)))
		return false;

	char folder[CM_SYSTEM_MAXNAME];
	JoinPathA(folder, sizeof(folder), localAppData, "Sedona-L2MapViewer");
	CreateDirectoryA(folder, 0);
	JoinPathA(out, outSize, folder, "recent-clients.txt");
	return true;
}

static const char* CompactPathLabel(const char* path)
{
	if(!path || !path[0])
		return "";

	const char* end = path + strlen(path);
	while(end > path && (end[-1] == '/' || end[-1] == '\\'))
		end--;

	const char* start = end;
	while(start > path && start[-1] != '/' && start[-1] != '\\')
		start--;

	static char label[96];
	size_t len = (size_t)(end - start);
	if(len >= sizeof(label))
		len = sizeof(label) - 1;
	memcpy(label, start, len);
	label[len] = 0;
	return label;
}

static bool FindStagedImportCopyScript(char* out, size_t outSize)
{
	if(!out || outSize == 0)
		return false;

	char moduleDir[CM_SYSTEM_MAXNAME];
	GetModuleFileNameA(0, moduleDir, sizeof(moduleDir));
	char* slash = strrchr(moduleDir, '\\');
	if(slash)
		*slash = 0;

	const char* relativeCandidates[] = {
		"..\\scripts\\Invoke-StagedImportCopy.ps1",
		"scripts\\Invoke-StagedImportCopy.ps1",
		"..\\..\\scripts\\Invoke-StagedImportCopy.ps1",
		0
	};

	for(int i = 0; relativeCandidates[i]; i++)
	{
		char candidate[CM_SYSTEM_MAXNAME];
		JoinPathA(candidate, sizeof(candidate), moduleDir, relativeCandidates[i]);
		if(FileExistsA(candidate))
		{
			strcpy_s(out, outSize, candidate);
			return true;
		}
	}

	const char* absoluteCandidate = "C:\\GITHUB\\Sedona-L2MapViewer\\scripts\\Invoke-StagedImportCopy.ps1";
	if(FileExistsA(absoluteCandidate))
	{
		strcpy_s(out, outSize, absoluteCandidate);
		return true;
	}

	return false;
}

static std::string RunCommandCapture(const char* commandLine, DWORD timeoutMs, DWORD* exitCode)
{
	if(exitCode)
		*exitCode = 1;

	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;

	HANDLE readPipe = 0;
	HANDLE writePipe = 0;
	if(!CreatePipe(&readPipe, &writePipe, &sa, 0))
		return "Could not create output pipe.";
	SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdOutput = writePipe;
	si.hStdError = writePipe;

	char mutableCommand[4096];
	strcpy_s(mutableCommand, sizeof(mutableCommand), commandLine);

	if(!CreateProcessA(0, mutableCommand, 0, 0, TRUE, CREATE_NO_WINDOW, 0, 0, &si, &pi))
	{
		CloseHandle(writePipe);
		CloseHandle(readPipe);
		return "Could not start PowerShell preview command.";
	}

	CloseHandle(writePipe);

	std::string output;
	DWORD startTick = GetTickCount();
	bool processDone = false;
	char buffer[1024];

	while(!processDone)
	{
		DWORD available = 0;
		if(PeekNamedPipe(readPipe, 0, 0, 0, &available, 0) && available > 0)
		{
			DWORD readBytes = 0;
			if(ReadFile(readPipe, buffer, min((DWORD)sizeof(buffer) - 1, available), &readBytes, 0) && readBytes > 0)
			{
				buffer[readBytes] = 0;
				output += buffer;
			}
		}
		else
		{
			DWORD waitResult = WaitForSingleObject(pi.hProcess, 50);
			processDone = waitResult == WAIT_OBJECT_0;
			if(!processDone && GetTickCount() - startTick > timeoutMs)
			{
				TerminateProcess(pi.hProcess, 1);
				output += "\nPreview command timed out.";
				processDone = true;
			}
		}
	}

	for(;;)
	{
		DWORD readBytes = 0;
		if(!ReadFile(readPipe, buffer, sizeof(buffer) - 1, &readBytes, 0) || readBytes == 0)
			break;
		buffer[readBytes] = 0;
		output += buffer;
	}

	DWORD processExit = 1;
	GetExitCodeProcess(pi.hProcess, &processExit);
	if(exitCode)
		*exitCode = processExit;

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	CloseHandle(readPipe);
	return output;
}

static int CountFilesInFolder(const char* root, const char* folder, const char* pattern)
{
	if(!root || !folder || !pattern || !root[0])
		return 0;

	char searchPath[CM_SYSTEM_MAXNAME];
	sprintf_s(searchPath, sizeof(searchPath), "%s%s/%s", root, folder, pattern);

	WIN32_FIND_DATAA findData;
	HANDLE findHandle = FindFirstFileA(searchPath, &findData);
	if(findHandle == INVALID_HANDLE_VALUE)
		return 0;

	int count = 0;
	do
	{
		if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			count++;
	}
	while(FindNextFileA(findHandle, &findData));

	FindClose(findHandle);
	return count;
}

static int CountFilesRecursive(const char* root)
{
	if(!root || !root[0])
		return 0;

	char searchPath[CM_SYSTEM_MAXNAME];
	sprintf_s(searchPath, sizeof(searchPath), "%s*", root);

	WIN32_FIND_DATAA findData;
	HANDLE findHandle = FindFirstFileA(searchPath, &findData);
	if(findHandle == INVALID_HANDLE_VALUE)
		return 0;

	int count = 0;
	do
	{
		if(strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
			continue;

		if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			char childPath[CM_SYSTEM_MAXNAME];
			sprintf_s(childPath, sizeof(childPath), "%s%s/", root, findData.cFileName);
			count += CountFilesRecursive(childPath);
		}
		else
			count++;
	}
	while(FindNextFileA(findHandle, &findData));

	FindClose(findHandle);
	return count;
}

static void FindLargestFileInFolder(const char* root, const char* folder, const char* pattern, char* fileName, size_t fileNameSize, double* sizeMb)
{
	if(fileName && fileNameSize > 0)
		fileName[0] = 0;
	if(sizeMb)
		*sizeMb = 0.0;
	if(!root || !folder || !pattern || !root[0] || !fileName || fileNameSize == 0 || !sizeMb)
		return;

	char searchPath[CM_SYSTEM_MAXNAME];
	sprintf_s(searchPath, sizeof(searchPath), "%s%s/%s", root, folder, pattern);

	WIN32_FIND_DATAA findData;
	HANDLE findHandle = FindFirstFileA(searchPath, &findData);
	if(findHandle == INVALID_HANDLE_VALUE)
		return;

	ULONGLONG largestSize = 0;
	do
	{
		if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		ULONGLONG fileSize = (((ULONGLONG)findData.nFileSizeHigh) << 32) | findData.nFileSizeLow;
		if(fileSize > largestSize)
		{
			largestSize = fileSize;
			strcpy_s(fileName, fileNameSize, findData.cFileName);
		}
	}
	while(FindNextFileA(findHandle, &findData));

	FindClose(findHandle);
	*sizeMb = (double)largestSize / (1024.0 * 1024.0);
}

static char* LoadTextFile(const char* path)
{
	FILE *file = fopen(path, "rb");
	if(!file)
		return 0;

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *data = new char[size + 1];
	fread(data, size, 1, file);
	data[size] = 0;
	fclose(file);
	return data;
}

L2UIEditor::L2UIEditor()
	: L2UIBaseWidget()
{
	ui_toolsWnd = 0;
	ui_statusText = 0;
	ui_clientStatusWnd = 0;
	ui_clientStatusText = 0;
	ui_stagingReportWnd = 0;
	ui_stagingReportText = 0;
	ui_refreshStagingCopyButton = 0;
	ui_showMapButton = 0;
	ui_loadDefaultButton = 0;
	ui_loadAreaButton = 0;
	ui_hideAllButton = 0;
	for(int i = 0; i < 3; i++)
	{
		recentTargetClients[i][0] = 0;
		recentDonorClients[i][0] = 0;
	}
}

L2UIEditor::~L2UIEditor()
{
}

void L2UIEditor::Init()
{
	loadRecentClientHistory();

	ui_topMenu = MyGUI::Gui::getInstance().createWidget<MyGUI::MenuBar>("MenuBar", 0, 0, 460, 24, MyGUI::Align::Default, "L2Editor", "TopMenu");
	ui_topMenu->setAlign(MyGUI::Align::HStretch | MyGUI::Align::Top);
	MyGUI::MenuItemPtr ui_topMenu_Main = ui_topMenu->addItem(L"Menu", MyGUI::MenuItemType::Popup);
	MyGUI::PopupMenuPtr ui_topMenu_MainMenu = ui_topMenu_Main->createItemChildT<MyGUI::PopupMenu>();
	MyGUI::MenuItemPtr ui_topMenu_Client = ui_topMenu->addItem(L"Client", MyGUI::MenuItemType::Popup);
	MyGUI::PopupMenuPtr ui_topMenu_ClientMenu = ui_topMenu_Client->createItemChildT<MyGUI::PopupMenu>();
	MyGUI::MenuItemPtr ui_topMenu_Editor = ui_topMenu->addItem(L"Editor", MyGUI::MenuItemType::Popup);
	MyGUI::PopupMenuPtr ui_topMenu_EditorMenu = ui_topMenu_Editor->createItemChildT<MyGUI::PopupMenu>();

	MyGUI::MenuItem *ui_topMenu_Main_Map = ui_topMenu_MainMenu->addItem(L"Map", MyGUI::MenuItemType::Normal, "TopMenu_Main_ShowMap");
	ui_topMenu_Main_Map->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onTopMenu_Main_ShowMap);
	MyGUI::MenuItem *ui_topMenu_Main_Load = ui_topMenu_MainMenu->addItem(L"Load 23_22", MyGUI::MenuItemType::Normal, "TopMenu_Main_LoadDefault");
	ui_topMenu_Main_Load->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onLoadDefaultClick);
	MyGUI::MenuItem *ui_topMenu_Main_LoadArea = ui_topMenu_MainMenu->addItem(L"Load 3x3 area", MyGUI::MenuItemType::Normal, "TopMenu_Main_LoadArea");
	ui_topMenu_Main_LoadArea->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onLoadAreaClick);
	MyGUI::MenuItem *ui_topMenu_Main_Hide = ui_topMenu_MainMenu->addItem(L"Hide all tiles", MyGUI::MenuItemType::Normal, "TopMenu_Main_HideAll");
	ui_topMenu_Main_Hide->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onHideAllClick);
	ui_topMenu_MainMenu->addItem("", MyGUI::MenuItemType::Separator);
	MyGUI::MenuItem *ui_topMenu_Main_Exit = ui_topMenu_MainMenu->addItem(L"Exit", MyGUI::MenuItemType::Normal, "TopMenu_Main_Exit");
	ui_topMenu_Main_Exit->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onTopMenu_Main_Exit);

	MyGUI::MenuItem *ui_topMenu_Client_Select = ui_topMenu_ClientMenu->addItem(L"Select target client...", MyGUI::MenuItemType::Normal, "TopMenu_Client_Select");
	ui_topMenu_Client_Select->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onSelectClientClick);
	MyGUI::MenuItem *ui_topMenu_Client_Donor = ui_topMenu_ClientMenu->addItem(L"Select donor client...", MyGUI::MenuItemType::Normal, "TopMenu_Client_Donor");
	ui_topMenu_Client_Donor->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onSelectDonorClick);
	if(recentTargetClients[0][0] || recentDonorClients[0][0])
		ui_topMenu_ClientMenu->addItem("", MyGUI::MenuItemType::Separator);
	if(recentTargetClients[0][0])
	{
		char caption[160];
		sprintf_s(caption, sizeof(caption), "Recent target: %s", CompactPathLabel(recentTargetClients[0]));
		MyGUI::MenuItem *recent = ui_topMenu_ClientMenu->addItem(caption, MyGUI::MenuItemType::Normal, "TopMenu_Client_RecentTarget1");
		recent->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onRecentTarget1Click);
	}
	if(recentTargetClients[1][0])
	{
		char caption[160];
		sprintf_s(caption, sizeof(caption), "Recent target: %s", CompactPathLabel(recentTargetClients[1]));
		MyGUI::MenuItem *recent = ui_topMenu_ClientMenu->addItem(caption, MyGUI::MenuItemType::Normal, "TopMenu_Client_RecentTarget2");
		recent->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onRecentTarget2Click);
	}
	if(recentTargetClients[2][0])
	{
		char caption[160];
		sprintf_s(caption, sizeof(caption), "Recent target: %s", CompactPathLabel(recentTargetClients[2]));
		MyGUI::MenuItem *recent = ui_topMenu_ClientMenu->addItem(caption, MyGUI::MenuItemType::Normal, "TopMenu_Client_RecentTarget3");
		recent->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onRecentTarget3Click);
	}
	if(recentDonorClients[0][0])
	{
		char caption[160];
		sprintf_s(caption, sizeof(caption), "Recent donor: %s", CompactPathLabel(recentDonorClients[0]));
		MyGUI::MenuItem *recent = ui_topMenu_ClientMenu->addItem(caption, MyGUI::MenuItemType::Normal, "TopMenu_Client_RecentDonor1");
		recent->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onRecentDonor1Click);
	}
	if(recentDonorClients[1][0])
	{
		char caption[160];
		sprintf_s(caption, sizeof(caption), "Recent donor: %s", CompactPathLabel(recentDonorClients[1]));
		MyGUI::MenuItem *recent = ui_topMenu_ClientMenu->addItem(caption, MyGUI::MenuItemType::Normal, "TopMenu_Client_RecentDonor2");
		recent->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onRecentDonor2Click);
	}
	if(recentDonorClients[2][0])
	{
		char caption[160];
		sprintf_s(caption, sizeof(caption), "Recent donor: %s", CompactPathLabel(recentDonorClients[2]));
		MyGUI::MenuItem *recent = ui_topMenu_ClientMenu->addItem(caption, MyGUI::MenuItemType::Normal, "TopMenu_Client_RecentDonor3");
		recent->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onRecentDonor3Click);
	}
	if(recentTargetClients[0][0] || recentDonorClients[0][0])
		ui_topMenu_ClientMenu->addItem("", MyGUI::MenuItemType::Separator);
	MyGUI::MenuItem *ui_topMenu_Client_GeoExport = ui_topMenu_ClientMenu->addItem(L"Select geodata export...", MyGUI::MenuItemType::Normal, "TopMenu_Client_GeoExport");
	ui_topMenu_Client_GeoExport->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onSelectGeodataExportClick);
	MyGUI::MenuItem *ui_topMenu_Client_AssetStaging = ui_topMenu_ClientMenu->addItem(L"Select asset staging...", MyGUI::MenuItemType::Normal, "TopMenu_Client_AssetStaging");
	ui_topMenu_Client_AssetStaging->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onSelectAssetStagingClick);
	MyGUI::MenuItem *ui_topMenu_Client_Status = ui_topMenu_ClientMenu->addItem(L"Show client status", MyGUI::MenuItemType::Normal, "TopMenu_Client_Status");
	ui_topMenu_Client_Status->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onShowClientStatusClick);
	MyGUI::MenuItem *ui_topMenu_Client_StagingReport = ui_topMenu_ClientMenu->addItem(L"Show staging report", MyGUI::MenuItemType::Normal, "TopMenu_Client_StagingReport");
	ui_topMenu_Client_StagingReport->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onShowStagingReportClick);
	ui_topMenu_ClientMenu->addItem("", MyGUI::MenuItemType::Separator);
	MyGUI::MenuItem *ui_topMenu_Client_H5 = ui_topMenu_ClientMenu->addItem(L"Target profile: H5", MyGUI::MenuItemType::Normal, "TopMenu_Client_H5");
	ui_topMenu_Client_H5->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onProfileH5Click);
	MyGUI::MenuItem *ui_topMenu_Client_Fafurion = ui_topMenu_ClientMenu->addItem(L"Target profile: Fafurion", MyGUI::MenuItemType::Normal, "TopMenu_Client_Fafurion");
	ui_topMenu_Client_Fafurion->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onProfileFafurionClick);
	MyGUI::MenuItem *ui_topMenu_Client_Homonkulus = ui_topMenu_ClientMenu->addItem(L"Target profile: Homonkulus", MyGUI::MenuItemType::Normal, "TopMenu_Client_Homonkulus");
	ui_topMenu_Client_Homonkulus->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onProfileHomonkulusClick);
	ui_topMenu_ClientMenu->addItem("", MyGUI::MenuItemType::Separator);
	MyGUI::MenuItem *ui_topMenu_Client_DonorH5 = ui_topMenu_ClientMenu->addItem(L"Donor profile: H5", MyGUI::MenuItemType::Normal, "TopMenu_Client_DonorH5");
	ui_topMenu_Client_DonorH5->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onDonorProfileH5Click);
	MyGUI::MenuItem *ui_topMenu_Client_DonorFafurion = ui_topMenu_ClientMenu->addItem(L"Donor profile: Fafurion", MyGUI::MenuItemType::Normal, "TopMenu_Client_DonorFafurion");
	ui_topMenu_Client_DonorFafurion->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onDonorProfileFafurionClick);
	MyGUI::MenuItem *ui_topMenu_Client_DonorHomonkulus = ui_topMenu_ClientMenu->addItem(L"Donor profile: Homonkulus", MyGUI::MenuItemType::Normal, "TopMenu_Client_DonorHomonkulus");
	ui_topMenu_Client_DonorHomonkulus->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onDonorProfileHomonkulusClick);

	MyGUI::MenuItem *ui_topMenu_Editor_Map = ui_topMenu_EditorMenu->addItem(L"Show map window", MyGUI::MenuItemType::Normal, "TopMenu_Editor_Map");
	ui_topMenu_Editor_Map->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onShowMapClick);
	MyGUI::MenuItem *ui_topMenu_Editor_Load = ui_topMenu_EditorMenu->addItem(L"Load 23_22", MyGUI::MenuItemType::Normal, "TopMenu_Editor_LoadDefault");
	ui_topMenu_Editor_Load->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onLoadDefaultClick);
	MyGUI::MenuItem *ui_topMenu_Editor_LoadArea = ui_topMenu_EditorMenu->addItem(L"Load 3x3 area", MyGUI::MenuItemType::Normal, "TopMenu_Editor_LoadArea");
	ui_topMenu_Editor_LoadArea->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onLoadAreaClick);
	MyGUI::MenuItem *ui_topMenu_Editor_Hide = ui_topMenu_EditorMenu->addItem(L"Hide all tiles", MyGUI::MenuItemType::Normal, "TopMenu_Editor_HideAll");
	ui_topMenu_Editor_Hide->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onHideAllClick);

	ui_leftPanel = MyGUI::Gui::getInstance().createWidget<MyGUI::ImageBox>("ImageBox", 0, 22, 40, 300, MyGUI::Align::Default, "L2Editor", "LeftPanel");
	ui_leftPanel->setAlign(MyGUI::Align::Left | MyGUI::Align::VStretch);
	ui_leftPanel->setProperty("ImageTexture", "ui_leftPanelBg.png");
	MyGUI::Button *terrainButton = ui_leftPanel->createWidget<MyGUI::Button>("Button", 4, 6, 32, 32, MyGUI::Align::Default, "TerrainBtn");
	terrainButton->setCaption(L"T");
	terrainButton->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onBtn1MouseClick);

	ui_toolsWnd = MyGUI::Gui::getInstance().createWidget<MyGUI::Window>("WindowCS", 760, 58, 244, 244, MyGUI::Align::Right | MyGUI::Align::Top, "L2WindowsLayer", "ToolsWnd");
	ui_toolsWnd->setCaption(L"Editor Tools");
	ui_statusText = ui_toolsWnd->createWidget<MyGUI::TextBox>("TextBox", 12, 12, 220, 54, MyGUI::Align::Top | MyGUI::Align::HStretch, "StatusText");

	ui_showMapButton = ui_toolsWnd->createWidget<MyGUI::Button>("Button", 12, 76, 102, 28, MyGUI::Align::Default, "ShowMapBtn");
	ui_showMapButton->setCaption(L"Map");
	ui_showMapButton->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onShowMapClick);

	ui_loadDefaultButton = ui_toolsWnd->createWidget<MyGUI::Button>("Button", 126, 76, 102, 28, MyGUI::Align::Default, "LoadDefaultBtn");
	ui_loadDefaultButton->setCaption(L"Load 23_22");
	ui_loadDefaultButton->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onLoadDefaultClick);

	ui_loadAreaButton = ui_toolsWnd->createWidget<MyGUI::Button>("Button", 12, 114, 102, 28, MyGUI::Align::Default, "LoadAreaBtn");
	ui_loadAreaButton->setCaption(L"Load 3x3");
	ui_loadAreaButton->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onLoadAreaClick);

	ui_hideAllButton = ui_toolsWnd->createWidget<MyGUI::Button>("Button", 126, 114, 102, 28, MyGUI::Align::Default, "HideAllBtn");
	ui_hideAllButton->setCaption(L"Hide all");
	ui_hideAllButton->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onHideAllClick);

	MyGUI::TextBox *helpText = ui_toolsWnd->createWidget<MyGUI::TextBox>("TextBox", 12, 154, 220, 54, MyGUI::Align::Default, "HelpText");
	helpText->setCaption("RMB rotate, wheel zoom\nWASD move, Space/Ctrl height\nClick mesh/BSP to inspect bounds");

	ui_sceneShowBsp = ui_toolsWnd->createWidget<MyGUI::Button>("CheckBox", 12, 212, 150, 20, MyGUI::Align::Default, "ShowBsp");
	ui_sceneShowBsp->setCaption(L"Draw BSP");
	ui_sceneShowBsp->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onShowBspMouseClick);

	ui_clientStatusWnd = MyGUI::Gui::getInstance().createWidget<MyGUI::Window>("WindowCSX", 96, 58, 560, 260, MyGUI::Align::Default, "L2WindowsLayer", "ClientStatusWnd");
	ui_clientStatusWnd->setCaption(L"Client Status");
	ui_clientStatusWnd->eventWindowButtonPressed += MyGUI::newDelegate(this, &L2UIEditor::onClientStatusWindowClose);
	ui_clientStatusText = ui_clientStatusWnd->createWidget<MyGUI::TextBox>("TextBox", 12, 12, 536, 226, MyGUI::Align::Stretch, "ClientStatusText");
	ui_clientStatusWnd->setVisible(false);

	ui_stagingReportWnd = MyGUI::Gui::getInstance().createWidget<MyGUI::Window>("WindowCSX", 128, 82, 680, 430, MyGUI::Align::Default, "L2WindowsLayer", "StagingReportWnd");
	ui_stagingReportWnd->setCaption(L"Asset Staging Report");
	ui_stagingReportWnd->eventWindowButtonPressed += MyGUI::newDelegate(this, &L2UIEditor::onStagingReportWindowClose);
	ui_stagingReportText = ui_stagingReportWnd->createWidget<MyGUI::TextBox>("TextBox", 12, 12, 656, 352, MyGUI::Align::Stretch, "StagingReportText");
	ui_refreshStagingCopyButton = ui_stagingReportWnd->createWidget<MyGUI::Button>("Button", 12, 372, 180, 28, MyGUI::Align::Left | MyGUI::Align::Bottom, "RefreshStagingCopyBtn");
	ui_refreshStagingCopyButton->setCaption(L"Refresh copy preview");
	ui_refreshStagingCopyButton->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onRefreshStagingCopyPreviewClick);
	ui_stagingReportWnd->setVisible(false);

	refreshStatusText();
	refreshClientStatusText();
}

void L2UIEditor::update()
{
	refreshStatusText();
	refreshClientStatusText();
}

void L2UIEditor::onResize(int width, int height)
{
	if(ui_toolsWnd)
		ui_toolsWnd->setPosition(width - ui_toolsWnd->getWidth() - 12, 36);
}

bool L2UIEditor::injectMouseDown(int x, int y, MyGUI::MouseButton btn)
{
	return false;
}

bool L2UIEditor::injectMouseUp(int x, int y, MyGUI::MouseButton btn)
{
	return false;
}

void L2UIEditor::onTopMenu_Main_ShowMap(MyGUI::Widget* sender)
{
	g_ui.getL2Map()->setVisible(true);
}

void L2UIEditor::onTopMenu_Main_Exit(MyGUI::Widget* sender)
{
	PostQuitMessage(0);
}

void L2UIEditor::onBtn1MouseClick(MyGUI::Widget* sender)
{
	char *terrain_vert_shader = LoadTextFile("data/shaders/terrain.vert");
	char *terrain_frag_shader = LoadTextFile("data/shaders/terrain.frag");
	if(terrain_vert_shader && terrain_frag_shader)
		g_shader.Add(terrain_vert_shader, terrain_frag_shader, "L2Terrain");
}

void L2UIEditor::onShowBspMouseClick(MyGUI::Widget* sender)
{
	sender->castType<MyGUI::Button>()->setStateCheck(!sender->castType<MyGUI::Button>()->getStateCheck());
	g_levelMgr.getVars()->showBsp = sender->castType<MyGUI::Button>()->getStateCheck();
}

void L2UIEditor::onShowMapClick(MyGUI::Widget* sender)
{
	g_ui.getL2Map()->setVisible(true);
}

void L2UIEditor::onLoadDefaultClick(MyGUI::Widget* sender)
{
	g_ui.getL2Map()->loadTile(23, 22);
}

void L2UIEditor::onLoadAreaClick(MyGUI::Widget* sender)
{
	g_ui.getL2Map()->loadTileArea(23, 22, 1);
}

void L2UIEditor::onHideAllClick(MyGUI::Widget* sender)
{
	g_ui.getL2Map()->hideAllTiles();
}

void L2UIEditor::onSelectClientClick(MyGUI::Widget* sender)
{
	BROWSEINFOA browseInfo;
	memset(&browseInfo, 0, sizeof(browseInfo));
	browseInfo.hwndOwner = g_window.getHWND();
	browseInfo.lpszTitle = "Select Lineage II client root folder";
	browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST itemList = SHBrowseForFolderA(&browseInfo);
	if(!itemList)
		return;

	char path[CM_SYSTEM_MAXNAME];
	if(SHGetPathFromIDListA(itemList, path))
	{
		rememberRecentTarget(path);
		char args[4096];
		sprintf_s(args, sizeof(args), "--client=\"%s\" --donor-client=\"%s\" --geodata=\"%s\" --geodata-export=\"%s\" --asset-staging=\"%s\"", path, g_cfg.getDonorClientBaseDir(), g_cfg.getGeodataBaseDir(), g_cfg.getGeodataExportDir(), g_cfg.getAssetStagingDir());
		restartWithArguments(args);
	}

	CoTaskMemFree(itemList);
}

void L2UIEditor::onSelectDonorClick(MyGUI::Widget* sender)
{
	BROWSEINFOA browseInfo;
	memset(&browseInfo, 0, sizeof(browseInfo));
	browseInfo.hwndOwner = g_window.getHWND();
	browseInfo.lpszTitle = "Select donor Lineage II client root folder";
	browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST itemList = SHBrowseForFolderA(&browseInfo);
	if(!itemList)
		return;

	char path[CM_SYSTEM_MAXNAME];
	if(SHGetPathFromIDListA(itemList, path))
	{
		rememberRecentDonor(path);
		char args[4096];
		sprintf_s(args, sizeof(args), "--client=\"%s\" --donor-client=\"%s\" --geodata=\"%s\" --geodata-export=\"%s\" --asset-staging=\"%s\"", g_cfg.getClientBaseDir(), path, g_cfg.getGeodataBaseDir(), g_cfg.getGeodataExportDir(), g_cfg.getAssetStagingDir());
		restartWithArguments(args);
	}

	CoTaskMemFree(itemList);
}

void L2UIEditor::onRecentTarget1Click(MyGUI::Widget* sender)
{
	restartWithRecentTarget(0);
}

void L2UIEditor::onRecentTarget2Click(MyGUI::Widget* sender)
{
	restartWithRecentTarget(1);
}

void L2UIEditor::onRecentTarget3Click(MyGUI::Widget* sender)
{
	restartWithRecentTarget(2);
}

void L2UIEditor::onRecentDonor1Click(MyGUI::Widget* sender)
{
	restartWithRecentDonor(0);
}

void L2UIEditor::onRecentDonor2Click(MyGUI::Widget* sender)
{
	restartWithRecentDonor(1);
}

void L2UIEditor::onRecentDonor3Click(MyGUI::Widget* sender)
{
	restartWithRecentDonor(2);
}

void L2UIEditor::onSelectGeodataExportClick(MyGUI::Widget* sender)
{
	BROWSEINFOA browseInfo;
	memset(&browseInfo, 0, sizeof(browseInfo));
	browseInfo.hwndOwner = g_window.getHWND();
	browseInfo.lpszTitle = "Select generated geodata export folder";
	browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST itemList = SHBrowseForFolderA(&browseInfo);
	if(!itemList)
		return;

	char path[CM_SYSTEM_MAXNAME];
	if(SHGetPathFromIDListA(itemList, path))
	{
		char args[4096];
		sprintf_s(args, sizeof(args), "--client=\"%s\" --donor-client=\"%s\" --geodata=\"%s\" --geodata-export=\"%s\" --asset-staging=\"%s\"", g_cfg.getClientBaseDir(), g_cfg.getDonorClientBaseDir(), g_cfg.getGeodataBaseDir(), path, g_cfg.getAssetStagingDir());
		restartWithArguments(args);
	}

	CoTaskMemFree(itemList);
}

void L2UIEditor::onSelectAssetStagingClick(MyGUI::Widget* sender)
{
	BROWSEINFOA browseInfo;
	memset(&browseInfo, 0, sizeof(browseInfo));
	browseInfo.hwndOwner = g_window.getHWND();
	browseInfo.lpszTitle = "Select asset import/export staging folder";
	browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST itemList = SHBrowseForFolderA(&browseInfo);
	if(!itemList)
		return;

	char path[CM_SYSTEM_MAXNAME];
	if(SHGetPathFromIDListA(itemList, path))
	{
		char args[4096];
		sprintf_s(args, sizeof(args), "--client=\"%s\" --donor-client=\"%s\" --geodata=\"%s\" --geodata-export=\"%s\" --asset-staging=\"%s\"", g_cfg.getClientBaseDir(), g_cfg.getDonorClientBaseDir(), g_cfg.getGeodataBaseDir(), g_cfg.getGeodataExportDir(), path);
		restartWithArguments(args);
	}

	CoTaskMemFree(itemList);
}

void L2UIEditor::onShowClientStatusClick(MyGUI::Widget* sender)
{
	if(!ui_clientStatusWnd)
		return;

	refreshClientStatusText();
	MyGUI::LayerManager::getInstance().upLayerItem(ui_clientStatusWnd);
	ui_clientStatusWnd->setVisible(true);
}

void L2UIEditor::onClientStatusWindowClose(MyGUI::Window* sender, const std::string& evt)
{
	if(ui_clientStatusWnd)
		ui_clientStatusWnd->setVisible(false);
}

void L2UIEditor::onShowStagingReportClick(MyGUI::Widget* sender)
{
	if(!ui_stagingReportWnd)
		return;

	refreshStagingReportText();
	MyGUI::LayerManager::getInstance().upLayerItem(ui_stagingReportWnd);
	ui_stagingReportWnd->setVisible(true);
}

void L2UIEditor::onRefreshStagingCopyPreviewClick(MyGUI::Widget* sender)
{
	if(!ui_stagingReportText)
		return;

	char scriptPath[CM_SYSTEM_MAXNAME];
	if(!FindStagedImportCopyScript(scriptPath, sizeof(scriptPath)))
	{
		ui_stagingReportText->setCaption("Invoke-StagedImportCopy.ps1 was not found beside the repo scripts folder.");
		return;
	}

	char command[4096];
	sprintf_s(command, sizeof(command),
		"powershell -NoProfile -ExecutionPolicy Bypass -File \"%s\" -TargetProfile %s -DonorProfile %s -Kind Maps -Package 23_22.unr -StagingPath \"%s\" -Limit 250",
		scriptPath,
		g_cfg.getClientProfileName(),
		g_cfg.getDonorProfileName(),
		g_cfg.getAssetStagingDir());

	ui_stagingReportText->setCaption("Running dry-run copy preview...");
	DWORD exitCode = 1;
	std::string output = RunCommandCapture(command, 30000, &exitCode);
	std::string report = "Dry-run staging copy preview for Maps/23_22.unr\n";
	report += "No files are copied from this button. Use the script with -Execute when the plan is correct.\n\n";
	report += output;
	if(exitCode != 0)
	{
		report += "\nPreview command failed with exit code ";
		char code[32];
		sprintf_s(code, sizeof(code), "%lu", exitCode);
		report += code;
		report += ".";
	}

	ui_stagingReportText->setCaption(MyGUI::UString(report.c_str()));
}

void L2UIEditor::onStagingReportWindowClose(MyGUI::Window* sender, const std::string& evt)
{
	if(ui_stagingReportWnd)
		ui_stagingReportWnd->setVisible(false);
}

void L2UIEditor::onProfileH5Click(MyGUI::Widget* sender)
{
	restartWithProfile("H5");
}

void L2UIEditor::onProfileFafurionClick(MyGUI::Widget* sender)
{
	restartWithProfile("Fafurion");
}

void L2UIEditor::onProfileHomonkulusClick(MyGUI::Widget* sender)
{
	restartWithProfile("Homonkulus");
}

void L2UIEditor::onDonorProfileH5Click(MyGUI::Widget* sender)
{
	restartWithDonorProfile("H5");
}

void L2UIEditor::onDonorProfileFafurionClick(MyGUI::Widget* sender)
{
	restartWithDonorProfile("Fafurion");
}

void L2UIEditor::onDonorProfileHomonkulusClick(MyGUI::Widget* sender)
{
	restartWithDonorProfile("Homonkulus");
}

void L2UIEditor::restartWithProfile(const char* profile)
{
	char args[4096];
	sprintf_s(args, sizeof(args), "--profile=%s --donor-client=\"%s\" --geodata=\"%s\" --geodata-export=\"%s\" --asset-staging=\"%s\"", profile, g_cfg.getDonorClientBaseDir(), g_cfg.getGeodataBaseDir(), g_cfg.getGeodataExportDir(), g_cfg.getAssetStagingDir());
	restartWithArguments(args);
}

void L2UIEditor::restartWithDonorProfile(const char* profile)
{
	char args[4096];
	sprintf_s(args, sizeof(args), "--profile=%s --client=\"%s\" --donor-profile=%s --geodata=\"%s\" --geodata-export=\"%s\" --asset-staging=\"%s\"", g_cfg.getClientProfileName(), g_cfg.getClientBaseDir(), profile, g_cfg.getGeodataBaseDir(), g_cfg.getGeodataExportDir(), g_cfg.getAssetStagingDir());
	restartWithArguments(args);
}

void L2UIEditor::restartWithArguments(const char* arguments)
{
	char exePath[CM_SYSTEM_MAXNAME];
	GetModuleFileNameA(0, exePath, sizeof(exePath));
	ShellExecuteA(g_window.getHWND(), "open", exePath, arguments, 0, SW_SHOWNORMAL);
	PostQuitMessage(0);
}

void L2UIEditor::refreshStatusText()
{
	if(!ui_statusText)
		return;

	char status[512];
	sprintf_s(status, sizeof(status), "Target: %s\nDonor: %s\nUse Client menu for paths.", g_cfg.getClientProfileName(), g_cfg.getDonorProfileName());
	ui_statusText->setCaption(MyGUI::UString(status));
}

void L2UIEditor::refreshClientStatusText()
{
	if(!ui_clientStatusText)
		return;

	char status[4096];
	sprintf_s(status, sizeof(status), "Target profile: %s\nTarget client:\n%s\n\nDonor profile: %s\nDonor client:\n%s\n\nGeodata input:\n%s\n\nGeodata export:\n%s\n\nAsset staging:\n%s", g_cfg.getClientProfileName(), g_cfg.getClientBaseDir(), g_cfg.getDonorProfileName(), g_cfg.getDonorClientBaseDir(), g_cfg.getGeodataBaseDir(), g_cfg.getGeodataExportDir(), g_cfg.getAssetStagingDir());
	ui_clientStatusText->setCaption(MyGUI::UString(status));
}

void L2UIEditor::loadRecentClientHistory()
{
	for(int i = 0; i < 3; i++)
	{
		recentTargetClients[i][0] = 0;
		recentDonorClients[i][0] = 0;
	}

	char historyPath[CM_SYSTEM_MAXNAME];
	if(!GetRecentClientHistoryPath(historyPath, sizeof(historyPath)))
		return;

	FILE* file = fopen(historyPath, "rt");
	if(!file)
		return;

	int targetIndex = 0;
	int donorIndex = 0;
	char line[CM_SYSTEM_MAXNAME + 32];
	while(fgets(line, sizeof(line), file))
	{
		char* newline = strpbrk(line, "\r\n");
		if(newline)
			*newline = 0;

		if(strncmp(line, "target=", 7) == 0 && targetIndex < 3)
		{
			strcpy_s(recentTargetClients[targetIndex], sizeof(recentTargetClients[targetIndex]), line + 7);
			NormalizeHistoryPath(recentTargetClients[targetIndex]);
			targetIndex++;
		}
		else if(strncmp(line, "donor=", 6) == 0 && donorIndex < 3)
		{
			strcpy_s(recentDonorClients[donorIndex], sizeof(recentDonorClients[donorIndex]), line + 6);
			NormalizeHistoryPath(recentDonorClients[donorIndex]);
			donorIndex++;
		}
	}

	fclose(file);
}

void L2UIEditor::saveRecentClientHistory()
{
	char historyPath[CM_SYSTEM_MAXNAME];
	if(!GetRecentClientHistoryPath(historyPath, sizeof(historyPath)))
		return;

	FILE* file = fopen(historyPath, "wt");
	if(!file)
		return;

	for(int i = 0; i < 3; i++)
	{
		if(recentTargetClients[i][0])
			fprintf(file, "target=%s\n", recentTargetClients[i]);
	}
	for(int i = 0; i < 3; i++)
	{
		if(recentDonorClients[i][0])
			fprintf(file, "donor=%s\n", recentDonorClients[i]);
	}

	fclose(file);
}

static void RememberPathInList(char entries[3][CM_SYSTEM_MAXNAME], const char* path)
{
	if(!path || !path[0])
		return;

	char normalized[CM_SYSTEM_MAXNAME];
	strcpy_s(normalized, sizeof(normalized), path);
	NormalizeHistoryPath(normalized);

	int existing = -1;
	for(int i = 0; i < 3; i++)
	{
		if(entries[i][0] && _stricmp(entries[i], normalized) == 0)
			existing = i;
	}

	if(existing == 0)
		return;

	if(existing > 0)
	{
		char previous[CM_SYSTEM_MAXNAME];
		strcpy_s(previous, sizeof(previous), entries[existing]);
		for(int i = existing; i > 0; i--)
			strcpy_s(entries[i], sizeof(entries[i]), entries[i - 1]);
		strcpy_s(entries[0], sizeof(entries[0]), previous);
		return;
	}

	for(int i = 2; i > 0; i--)
		strcpy_s(entries[i], sizeof(entries[i]), entries[i - 1]);
	strcpy_s(entries[0], sizeof(entries[0]), normalized);
}

void L2UIEditor::rememberRecentTarget(const char* path)
{
	RememberPathInList(recentTargetClients, path);
	saveRecentClientHistory();
}

void L2UIEditor::rememberRecentDonor(const char* path)
{
	RememberPathInList(recentDonorClients, path);
	saveRecentClientHistory();
}

void L2UIEditor::restartWithRecentTarget(int index)
{
	if(index < 0 || index >= 3 || !recentTargetClients[index][0])
		return;

	char args[4096];
	sprintf_s(args, sizeof(args), "--client=\"%s\" --donor-client=\"%s\" --geodata=\"%s\" --geodata-export=\"%s\" --asset-staging=\"%s\"", recentTargetClients[index], g_cfg.getDonorClientBaseDir(), g_cfg.getGeodataBaseDir(), g_cfg.getGeodataExportDir(), g_cfg.getAssetStagingDir());
	restartWithArguments(args);
}

void L2UIEditor::restartWithRecentDonor(int index)
{
	if(index < 0 || index >= 3 || !recentDonorClients[index][0])
		return;

	char args[4096];
	sprintf_s(args, sizeof(args), "--client=\"%s\" --donor-client=\"%s\" --geodata=\"%s\" --geodata-export=\"%s\" --asset-staging=\"%s\"", g_cfg.getClientBaseDir(), recentDonorClients[index], g_cfg.getGeodataBaseDir(), g_cfg.getGeodataExportDir(), g_cfg.getAssetStagingDir());
	restartWithArguments(args);
}

void L2UIEditor::refreshStagingReportText()
{
	if(!ui_stagingReportText)
		return;

	const char* target = g_cfg.getClientBaseDir();
	const char* donor = g_cfg.getDonorClientBaseDir();
	int targetMaps = CountFilesInFolder(target, "Maps", "*.unr");
	int donorMaps = CountFilesInFolder(donor, "Maps", "*.unr");
	int targetMeshes = CountFilesInFolder(target, "StaticMeshes", "*.usx");
	int donorMeshes = CountFilesInFolder(donor, "StaticMeshes", "*.usx");
	int targetSysTextures = CountFilesInFolder(target, "SysTextures", "*.utx");
	int donorSysTextures = CountFilesInFolder(donor, "SysTextures", "*.utx");
	int targetTextures = CountFilesInFolder(target, "Textures", "*.utx");
	int donorTextures = CountFilesInFolder(donor, "Textures", "*.utx");
	int targetAnimations = CountFilesInFolder(target, "Animations", "*.ukx");
	int donorAnimations = CountFilesInFolder(donor, "Animations", "*.ukx");
	int targetSounds = CountFilesInFolder(target, "Sounds", "*.uax");
	int donorSounds = CountFilesInFolder(donor, "Sounds", "*.uax");
	int targetSystem = CountFilesInFolder(target, "system", "*.dat");
	int donorSystem = CountFilesInFolder(donor, "system", "*.dat");
	int stagedFiles = CountFilesRecursive(g_cfg.getAssetStagingDir());
	char largestMap[128];
	char largestMesh[128];
	char largestSysTexture[128];
	char largestTexture[128];
	char largestAnimation[128];
	char largestSound[128];
	double largestMapMb;
	double largestMeshMb;
	double largestSysTextureMb;
	double largestTextureMb;
	double largestAnimationMb;
	double largestSoundMb;
	FindLargestFileInFolder(donor, "Maps", "*.unr", largestMap, sizeof(largestMap), &largestMapMb);
	FindLargestFileInFolder(donor, "StaticMeshes", "*.usx", largestMesh, sizeof(largestMesh), &largestMeshMb);
	FindLargestFileInFolder(donor, "SysTextures", "*.utx", largestSysTexture, sizeof(largestSysTexture), &largestSysTextureMb);
	FindLargestFileInFolder(donor, "Textures", "*.utx", largestTexture, sizeof(largestTexture), &largestTextureMb);
	FindLargestFileInFolder(donor, "Animations", "*.ukx", largestAnimation, sizeof(largestAnimation), &largestAnimationMb);
	FindLargestFileInFolder(donor, "Sounds", "*.uax", largestSound, sizeof(largestSound), &largestSoundMb);

	char warnings[512];
	warnings[0] = 0;
	if(donorMaps + donorMeshes + donorSysTextures + donorTextures + donorAnimations + donorSounds + donorSystem == 0)
		strcat_s(warnings, sizeof(warnings), "Warning: donor client has no visible supported packages.\n");
	if(targetMaps + targetMeshes + targetSysTextures + targetTextures + targetAnimations + targetSounds + targetSystem == 0)
		strcat_s(warnings, sizeof(warnings), "Warning: target client has no visible supported packages.\n");
	if(stagedFiles == 0)
		strcat_s(warnings, sizeof(warnings), "Warning: staging folder is empty or missing.\n");
	if(_stricmp(g_cfg.getClientBaseDir(), g_cfg.getDonorClientBaseDir()) == 0)
		strcat_s(warnings, sizeof(warnings), "Warning: target and donor client paths are identical.\n");
	if(warnings[0] == 0)
		strcpy_s(warnings, sizeof(warnings), "No blocking warnings in this preview.\n");

	char status[4096];
	sprintf_s(status, sizeof(status),
		"Target: %s\nDonor: %s\nStaging files: %d\n\nKind             Target  Donor\nMaps             %6d %6d\nStaticMeshes     %6d %6d\nSysTextures      %6d %6d\nTextures         %6d %6d\nAnimations       %6d %6d\nSounds           %6d %6d\nSystem dat       %6d %6d\n\nLargest donor candidates\nMap:        %s (%.2f MB)\nStaticMesh: %s (%.2f MB)\nSysTexture: %s (%.2f MB)\nTexture:    %s (%.2f MB)\nAnimation:  %s (%.2f MB)\nSound:      %s (%.2f MB)\n\n%s\nPreview only: no client files are copied yet.",
		g_cfg.getClientProfileName(), g_cfg.getDonorProfileName(), stagedFiles,
		targetMaps, donorMaps,
		targetMeshes, donorMeshes,
		targetSysTextures, donorSysTextures,
		targetTextures, donorTextures,
		targetAnimations, donorAnimations,
		targetSounds, donorSounds,
		targetSystem, donorSystem,
		largestMap, largestMapMb,
		largestMesh, largestMeshMb,
		largestSysTexture, largestSysTextureMb,
		largestTexture, largestTextureMb,
		largestAnimation, largestAnimationMb,
		largestSound, largestSoundMb,
		warnings);
	ui_stagingReportText->setCaption(MyGUI::UString(status));
}
