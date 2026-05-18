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
	void onProfileH5Click(MyGUI::Widget* sender);
	void onProfileFafurionClick(MyGUI::Widget* sender);
	void onProfileHomonkulusClick(MyGUI::Widget* sender);
protected:
	void refreshStatusText();
	void restartWithArguments(const char* arguments);
	void restartWithProfile(const char* profile);

	MyGUI::MenuBarPtr ui_topMenu;
	MyGUI::ImageBox *ui_leftPanel;
	MyGUI::Button *ui_sceneShowBsp;
	MyGUI::Window *ui_toolsWnd;
	MyGUI::TextBox *ui_statusText;
	MyGUI::Button *ui_showMapButton;
	MyGUI::Button *ui_loadDefaultButton;
	MyGUI::Button *ui_loadAreaButton;
	MyGUI::Button *ui_hideAllButton;
	MyGUI::Button *ui_selectClientButton;
	MyGUI::Button *ui_profileH5Button;
	MyGUI::Button *ui_profileFafurionButton;
	MyGUI::Button *ui_profileHomonkulusButton;
public:
	L2UIEditor();
	virtual ~L2UIEditor();
};

