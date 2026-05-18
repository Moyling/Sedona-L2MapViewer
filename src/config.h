#pragma once

#include "_afx.h"

class config
{
public:
	bool showDialog();
	void InitFromCommandLine(LPSTR commandLine);
	char* getClientBaseDir();
	char* getClientProfileName();
	char* getGeodataBaseDir();
	void makeWindowTitle(char* dest, size_t destSize);
	void makeGeodataPath(char* dest, size_t destSize, int mapX, int mapY);
public:
	bool CFG_FULLSCREEN;
	int CFG_SCREENRES_X;
	int CFG_SCREENRES_Y;
	int CFG_SCREENBITS;
	int CFG_ANTIALIASING;
	char CFG_CLIENT_PROFILE[64];
	char CFG_CLIENT_BASEDIR[CM_SYSTEM_MAXNAME];
	char CFG_GEODATA_BASEDIR[CM_SYSTEM_MAXNAME];
public:
	config(void);
	~config(void);
};
