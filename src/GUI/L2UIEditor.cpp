#include "L2UIEditor.h"

#include "../main.h"
#include "L2UIMap.h"
#include <shlobj.h>

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
	ui_showMapButton = 0;
	ui_loadDefaultButton = 0;
	ui_loadAreaButton = 0;
	ui_hideAllButton = 0;
}

L2UIEditor::~L2UIEditor()
{
}

void L2UIEditor::Init()
{
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
	MyGUI::MenuItem *ui_topMenu_Client_GeoExport = ui_topMenu_ClientMenu->addItem(L"Select geodata export...", MyGUI::MenuItemType::Normal, "TopMenu_Client_GeoExport");
	ui_topMenu_Client_GeoExport->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onSelectGeodataExportClick);
	MyGUI::MenuItem *ui_topMenu_Client_Status = ui_topMenu_ClientMenu->addItem(L"Show client status", MyGUI::MenuItemType::Normal, "TopMenu_Client_Status");
	ui_topMenu_Client_Status->eventMouseButtonClick += MyGUI::newDelegate(this, &L2UIEditor::onShowClientStatusClick);
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
		char args[4096];
		sprintf_s(args, sizeof(args), "--client=\"%s\" --donor-client=\"%s\" --geodata=\"%s\" --geodata-export=\"%s\" --asset-staging=\"%s\"", g_cfg.getClientBaseDir(), path, g_cfg.getGeodataBaseDir(), g_cfg.getGeodataExportDir(), g_cfg.getAssetStagingDir());
		restartWithArguments(args);
	}

	CoTaskMemFree(itemList);
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
