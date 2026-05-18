#include "config.h"

#include "main.h"

config::config(void)
{
	CFG_FULLSCREEN = false;
	CFG_SCREENRES_X = 1024;
	CFG_SCREENRES_Y = 700;
	CFG_SCREENBITS = 32;
	CFG_ANTIALIASING = 0;
	strcpy(CFG_CLIENT_PROFILE, "Custom");
	strcpy(CFG_CLIENT_BASEDIR, "../");
	strcpy(CFG_GEODATA_BASEDIR, "../geodata/");
}

config::~config(void)
{
}

static void NormalizeDir(char* path)
{
	size_t len = strlen(path);
	if(len == 0)
		return;

	if(path[0] == '"' && path[len - 1] == '"' && len > 1)
	{
		memmove(path, path + 1, len - 2);
		path[len - 2] = 0;
		len -= 2;
	}

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

static void CopyOptionValue(char* dest, size_t destSize, const char* value)
{
	if(!value || !dest || destSize == 0)
		return;

	while(*value == ' ' || *value == '\t')
		value++;

	if(*value == '"')
	{
		value++;
		size_t len = 0;
		while(value[len] && value[len] != '"' && len + 1 < destSize)
		{
			dest[len] = value[len];
			len++;
		}
		dest[len] = 0;
	}
	else
	{
		size_t len = 0;
		while(value[len] && value[len] != ' ' && value[len] != '\t' && len + 1 < destSize)
		{
			dest[len] = value[len];
			len++;
		}
		dest[len] = 0;
	}

	NormalizeDir(dest);
}

static const char* FindOptionValue(const char* commandLine, const char* optionName)
{
	if(!commandLine || !optionName)
		return 0;

	const char* found = strstr(commandLine, optionName);
	if(!found)
		return 0;

	return found + strlen(optionName);
}

static void CopyProfileName(char* dest, size_t destSize, const char* value)
{
	if(!value || !dest || destSize == 0)
		return;

	while(*value == ' ' || *value == '\t')
		value++;

	if(_strnicmp(value, "Homonkulus", 10) == 0 || _strnicmp(value, "Homunculus", 10) == 0)
		strcpy_s(dest, destSize, "Homonkulus");
	else if(_strnicmp(value, "Fafurion", 8) == 0)
		strcpy_s(dest, destSize, "Fafurion");
	else if(_strnicmp(value, "H5", 2) == 0 || _strnicmp(value, "HighFive", 8) == 0)
		strcpy_s(dest, destSize, "H5");
	else
	{
		size_t len = 0;
		while(value[len] && value[len] != ' ' && value[len] != '\t' && value[len] != '"' && len + 1 < destSize)
		{
			dest[len] = value[len];
			len++;
		}
		dest[len] = 0;
	}
}

static void ApplyKnownProfile(char* profile, size_t profileSize, char* clientDir, size_t clientDirSize)
{
	if(_stricmp(profile, "H5") == 0 || _stricmp(profile, "HighFive") == 0)
	{
		strcpy_s(profile, profileSize, "H5");
		strcpy_s(clientDir, clientDirSize, "C:/GITHUB/L2Modder_V2/Kliensek/Lineage II H5 Custom/");
	}
	else if(_stricmp(profile, "Fafurion") == 0)
	{
		strcpy_s(profile, profileSize, "Fafurion");
		strcpy_s(clientDir, clientDirSize, "C:/GITHUB/L2Modder_V2/Kliensek/FULL CLIENT LINEAGE2 FAFURION REV 166 EU/");
	}
	else if(_stricmp(profile, "Homonkulus") == 0 || _stricmp(profile, "Homunculus") == 0)
	{
		strcpy_s(profile, profileSize, "Homonkulus");
		strcpy_s(clientDir, clientDirSize, "C:/GITHUB/L2Modder_V2/Kliensek/L2NAP286D20201216G269/");
	}
}

static void InferProfileFromClientPath(char* profile, size_t profileSize, const char* clientDir)
{
	if(!clientDir || !profile)
		return;

	if(strstr(clientDir, "Lineage II H5 Custom") || strstr(clientDir, "/H5/"))
		strcpy_s(profile, profileSize, "H5");
	else if(strstr(clientDir, "FAFURION") || strstr(clientDir, "Fafurion"))
		strcpy_s(profile, profileSize, "Fafurion");
	else if(strstr(clientDir, "L2NAP286D20201216G269") || strstr(clientDir, "Homunculus") || strstr(clientDir, "Homonkulus"))
		strcpy_s(profile, profileSize, "Homonkulus");
}

void config::InitFromCommandLine(LPSTR commandLine)
{
	char envProfile[64];
	DWORD envProfileLen = GetEnvironmentVariableA("SEDONA_L2_PROFILE", envProfile, sizeof(envProfile));
	if(envProfileLen > 0 && envProfileLen < sizeof(envProfile))
	{
		CopyProfileName(CFG_CLIENT_PROFILE, sizeof(CFG_CLIENT_PROFILE), envProfile);
		ApplyKnownProfile(CFG_CLIENT_PROFILE, sizeof(CFG_CLIENT_PROFILE), CFG_CLIENT_BASEDIR, sizeof(CFG_CLIENT_BASEDIR));
	}

	char envClient[CM_SYSTEM_MAXNAME];
	DWORD envClientLen = GetEnvironmentVariableA("SEDONA_L2_CLIENT", envClient, sizeof(envClient));
	if(envClientLen > 0 && envClientLen < sizeof(envClient))
	{
		strcpy(CFG_CLIENT_BASEDIR, envClient);
		NormalizeDir(CFG_CLIENT_BASEDIR);
	}

	char envGeodata[CM_SYSTEM_MAXNAME];
	DWORD envGeoLen = GetEnvironmentVariableA("SEDONA_L2_GEODATA", envGeodata, sizeof(envGeodata));
	if(envGeoLen > 0 && envGeoLen < sizeof(envGeodata))
	{
		strcpy(CFG_GEODATA_BASEDIR, envGeodata);
		NormalizeDir(CFG_GEODATA_BASEDIR);
	}

	const char* profileArg = FindOptionValue(commandLine, "--profile=");
	if(!profileArg)
		profileArg = FindOptionValue(commandLine, "/profile=");
	if(profileArg)
	{
		CopyProfileName(CFG_CLIENT_PROFILE, sizeof(CFG_CLIENT_PROFILE), profileArg);
		ApplyKnownProfile(CFG_CLIENT_PROFILE, sizeof(CFG_CLIENT_PROFILE), CFG_CLIENT_BASEDIR, sizeof(CFG_CLIENT_BASEDIR));
	}

	const char* clientArg = FindOptionValue(commandLine, "--client=");
	if(!clientArg)
		clientArg = FindOptionValue(commandLine, "--client-path=");
	if(!clientArg)
		clientArg = FindOptionValue(commandLine, "/client=");
	if(clientArg)
	{
		CopyOptionValue(CFG_CLIENT_BASEDIR, sizeof(CFG_CLIENT_BASEDIR), clientArg);
		InferProfileFromClientPath(CFG_CLIENT_PROFILE, sizeof(CFG_CLIENT_PROFILE), CFG_CLIENT_BASEDIR);
	}

	const char* geodataArg = FindOptionValue(commandLine, "--geodata=");
	if(!geodataArg)
		geodataArg = FindOptionValue(commandLine, "--geodata-path=");
	if(!geodataArg)
		geodataArg = FindOptionValue(commandLine, "/geodata=");
	if(geodataArg)
		CopyOptionValue(CFG_GEODATA_BASEDIR, sizeof(CFG_GEODATA_BASEDIR), geodataArg);
}

char* config::getClientBaseDir()
{
	return CFG_CLIENT_BASEDIR;
}

char* config::getClientProfileName()
{
	return CFG_CLIENT_PROFILE;
}

char* config::getGeodataBaseDir()
{
	return CFG_GEODATA_BASEDIR;
}

void config::makeWindowTitle(char* dest, size_t destSize)
{
	sprintf_s(dest, destSize, "Sedona L2 Map Viewer - %s - %s", CFG_CLIENT_PROFILE, CFG_CLIENT_BASEDIR);
}

void config::makeGeodataPath(char* dest, size_t destSize, int mapX, int mapY)
{
	sprintf_s(dest, destSize, "%s%d_%d.l2j", CFG_GEODATA_BASEDIR, mapX, mapY);
}

static BOOL CALLBACK DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndDlgItem;
	int sel;

	switch (iMsg)
	{
		case WM_INITDIALOG:
			hwndDlgItem = GetDlgItem(hDlg, IDC_FULLSCREEN);
			SendMessageA(hwndDlgItem, CB_ADDSTRING, 0, (LPARAM)"No");
			SendMessageA(hwndDlgItem, CB_ADDSTRING, 0, (LPARAM)"Yes");
			SendMessageA(hwndDlgItem, CB_SETCURSEL, (WPARAM)0, 0);

			hwndDlgItem = GetDlgItem(hDlg, IDC_SCREENRES);
			SendMessageA(hwndDlgItem, CB_ADDSTRING, 0, (LPARAM)"1024x768");
			SendMessageA(hwndDlgItem, CB_ADDSTRING, 0, (LPARAM)"1280x1024");
			SendMessageA(hwndDlgItem, CB_ADDSTRING, 0, (LPARAM)"1366x768");
			SendMessageA(hwndDlgItem, CB_SETCURSEL, (WPARAM)0, 0);

			hwndDlgItem = GetDlgItem(hDlg, IDC_ANTIALIASING);
			SendMessageA(hwndDlgItem, CB_ADDSTRING, 0, (LPARAM)"no");
			SendMessageA(hwndDlgItem, CB_ADDSTRING, 0, (LPARAM)"x2");
			SendMessageA(hwndDlgItem, CB_ADDSTRING, 0, (LPARAM)"x4");
			SendMessageA(hwndDlgItem, CB_SETCURSEL, (WPARAM)0, 0);

			// move to center
			int x, y, screenWidth, screenHeight;
            RECT rcDlg;
            GetWindowRect(hDlg, &rcDlg);
            screenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
            screenHeight = GetSystemMetrics(SM_CYFULLSCREEN);

            x = (screenWidth / 2) - ((rcDlg.right - rcDlg.left) / 2);
            y = (screenHeight / 2) - ((rcDlg.bottom - rcDlg.top) / 2);

            MoveWindow(hDlg, x, y, (rcDlg.right - rcDlg.left),
                (rcDlg.bottom - rcDlg.top), TRUE);

			return true;
		case WM_COMMAND:
            switch(LOWORD(wParam))
            {
				/*case IDC_SCREENRES:
					if(HIWORD(wParam) == CBN_SELCHANGE )
					{
						hwndDlgItem = GetDlgItem(hDlg, IDC_SCREENRES);
						sel = SendMessage(hwndDlgItem, LB_GETCURSEL, 0, 0);

						CFG_FULLSCREEN = sel == 1 ? true : false;
					}
					break;*/
				case IDOK:
					sel = SendDlgItemMessage(hDlg , IDC_FULLSCREEN, CB_GETCURSEL, 0, 0);

					g_cfg.CFG_FULLSCREEN = sel == 1 ? true : false;

					sel = SendDlgItemMessage(hDlg , IDC_SCREENRES, CB_GETCURSEL, 0, 0);

					switch(sel)
					{
						case 0:
							g_cfg.CFG_SCREENRES_X = 1024;
							g_cfg.CFG_SCREENRES_Y = 768;
							break;
						case 1:
							g_cfg.CFG_SCREENRES_X = 1280;
							g_cfg.CFG_SCREENRES_Y = 1024;
							break;
						case 2:
							g_cfg.CFG_SCREENRES_X = 1366;
							g_cfg.CFG_SCREENRES_Y = 768;
							break;
					}

					sel = SendDlgItemMessage(hDlg , IDC_ANTIALIASING, CB_GETCURSEL, 0, 0);

					switch(sel)
					{
						case 0:
							g_cfg.CFG_ANTIALIASING = 0;
							break;
						case 1:
							g_cfg.CFG_ANTIALIASING = 2;
							break;
						case 2:
							g_cfg.CFG_ANTIALIASING = 4;
							break;
					}

					EndDialog(hDlg, true);
					return true;

				case IDCANCEL:
					EndDialog(hDlg, false);
					return true;
			}
			break;
	}

	return false;
}

bool config::showDialog()
{
	HINSTANCE mHInstance = GetModuleHandle(NULL);
	int rs = DialogBoxA(mHInstance, MAKEINTRESOURCEA(IDD_STARTUPDLG), NULL, (DLGPROC)DlgProc);

	if (rs == -1)
    {
        int winError = GetLastError();
        char* errDesc;
        int i;

        errDesc = new char[255];

        i = FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            winError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPSTR) errDesc,
            255,
            NULL
        );
		MessageBoxA(0, errDesc, 0, 0);
    }

	return rs;
}
