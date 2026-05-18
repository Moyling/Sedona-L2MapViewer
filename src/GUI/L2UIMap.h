#pragma once

#include "L2UIBaseWidget.h"
#include "../scene/L2LevelManager.h"

enum L2UIMapTileState
{
	L2UIMTS_SELECTED = 0x0001,
	L2UIMTS_LOADED = 0x0002,
	L2UIMTS_ACTIVE = 0x0004
};

class L2UIMap : public L2UIBaseWidget
{
public:
	virtual void Init();

	virtual void update();

	virtual bool injectMouseDown(int x, int y, MyGUI::MouseButton btn);
	virtual bool injectMouseUp(int x, int y, MyGUI::MouseButton btn);

	virtual void setVisible(bool value);

	void onTilesLoaded();
	void loadTile(int mapX, int mapY);
	void loadTileArea(int centerX, int centerY, int radius);
	void hideAllTiles();

	void onMapMouseDown(MyGUI::Widget* sender, int x, int y, MyGUI::MouseButton btn);
	void onMapMouseUp(MyGUI::Widget* sender, int x, int y, MyGUI::MouseButton btn);
	void onMapMouseClick(MyGUI::Widget* sender, MyGUI::MouseButton btn);
	void onMapWindowClose(MyGUI::Window* sender, const std::string& evt);
	void onMapTileLoadClick(MyGUI::Widget* sender);
	void onMapTileHideClick(MyGUI::Widget* sender);

	int16 getTileState(int x, int y);
	void setTileState(int x, int y, int16 state);
protected:
	void loadQueuedTiles(jfArray<L2MapTileInfo*, uint16> *tiles);
public:
	static const uint16 MAP_TILES_X = 30;
	static const uint16 MAP_TILES_Y = 30;
protected:
	MyGUI::Window *ui_mapWnd;
	MyGUI::ScrollView *ui_mapView;
	MyGUI::ImageBox *ui_map;
	MyGUI::ImageBox ***ui_mapTiles;
	int16 **tileStates;
	MyGUI::Button *ui_mapTileLoad;
	MyGUI::Button *ui_mapTileHide;

	bool isDraggingMap;
	bool hasDraggedMap;
	_vector2 mapDragStartMousePos;
	_vector2 mapDragStartContentPos;
public:
	L2UIMap(void);
	~L2UIMap(void);
};

