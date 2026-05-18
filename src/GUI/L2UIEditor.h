#pragma once

#include "L2UIBaseWidget.h"

class L2UIEditor : public L2UIBaseWidget
{
public:
	virtual void Init();

	virtual void update();
	virtual void onResize(int width, int height);

	virtual bool injectMouseDown(int x, int y, MyGUI::MouseButton btn);
	virtual bool injectMouseUp(int x, int y, MyGUI::MouseButton btn);

	void onTopMenu_Main_ShowMap(MyGUI::Widget* sender);
	void onTopMenu_Main_Exit(MyGUI::Widget* sender);
	void onBtn1MouseClick(MyGUI::Widget* sender);
	void onShowBspMouseClick(MyGUI::Widget* sender);
	void onShowMapClick(MyGUI::Widget* sender);
	void onLoadDefaultClick(MyGUI::Widget* sender);
	void onLoadAreaClick(MyGUI::Widget* sender);
	void onHideAllClick(MyGUI::Widget* sender);
	void onSelectClientClick(MyGUI::Widget* sender);
	void onSelectDonorClick(MyGUI::Widget* sender);
	void onRecentTarget1Click(MyGUI::Widget* sender);
	void onRecentTarget2Click(MyGUI::Widget* sender);
	void onRecentTarget3Click(MyGUI::Widget* sender);
	void onRecentDonor1Click(MyGUI::Widget* sender);
	void onRecentDonor2Click(MyGUI::Widget* sender);
	void onRecentDonor3Click(MyGUI::Widget* sender);
	void onSelectGeodataExportClick(MyGUI::Widget* sender);
	void onSelectAssetStagingClick(MyGUI::Widget* sender);
	void onShowClientStatusClick(MyGUI::Widget* sender);
	void onClientStatusWindowClose(MyGUI::Window* sender, const std::string& evt);
	void onShowStagingReportClick(MyGUI::Widget* sender);
	void onRefreshStagingCopyPreviewClick(MyGUI::Widget* sender);
	void onStagingReportWindowClose(MyGUI::Window* sender, const std::string& evt);
	void onProfileH5Click(MyGUI::Widget* sender);
	void onProfileFafurionClick(MyGUI::Widget* sender);
	void onProfileHomonkulusClick(MyGUI::Widget* sender);
	void onDonorProfileH5Click(MyGUI::Widget* sender);
	void onDonorProfileFafurionClick(MyGUI::Widget* sender);
	void onDonorProfileHomonkulusClick(MyGUI::Widget* sender);
protected:
	void refreshStatusText();
	void refreshClientStatusText();
	void refreshStagingReportText();
	void loadRecentClientHistory();
	void saveRecentClientHistory();
	void rememberRecentTarget(const char* path);
	void rememberRecentDonor(const char* path);
	void restartWithRecentTarget(int index);
	void restartWithRecentDonor(int index);
	void restartWithArguments(const char* arguments);
	void restartWithProfile(const char* profile);
	void restartWithDonorProfile(const char* profile);

	MyGUI::MenuBarPtr ui_topMenu;
	MyGUI::ImageBox *ui_leftPanel;
	MyGUI::Button *ui_sceneShowBsp;
	MyGUI::Window *ui_toolsWnd;
	MyGUI::TextBox *ui_statusText;
	MyGUI::Window *ui_clientStatusWnd;
	MyGUI::TextBox *ui_clientStatusText;
	MyGUI::Window *ui_stagingReportWnd;
	MyGUI::TextBox *ui_stagingReportText;
	MyGUI::Button *ui_refreshStagingCopyButton;
	MyGUI::Button *ui_showMapButton;
	MyGUI::Button *ui_loadDefaultButton;
	MyGUI::Button *ui_loadAreaButton;
	MyGUI::Button *ui_hideAllButton;
	char recentTargetClients[3][CM_SYSTEM_MAXNAME];
	char recentDonorClients[3][CM_SYSTEM_MAXNAME];
public:
	L2UIEditor();
	virtual ~L2UIEditor();
};

