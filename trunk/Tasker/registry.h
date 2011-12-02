//registry.h

#pragma once

#ifndef REGISTRYTOOLLOADED
#pragma message("registry.h included")
#define REGISTRYTOOLLOADED

//#include<windows.h>
//#include "stdafx.h"

//global extern HKEY g_hkey;
//extern HKEY g_hkey;
//extern TCHAR g_subkey;

//Create, write and read registry keys and values
extern TCHAR g_subkey[MAX_PATH+1];


//functions
int OpenKey();
int OpenKey(TCHAR *subkey);
int OpenCreateKey(TCHAR *subkey);
int RegDelValue(TCHAR *valuename);
int CloseKey();
int CreateSubKey(TCHAR *subkey);
int RegReadDword(TCHAR *valuename, DWORD *value);
int RegReadStr(TCHAR *valuename, TCHAR *value);

int RegWriteDword(TCHAR *valuename, DWORD *value);
int RegWriteStr(TCHAR *valuename, TCHAR *str);
int RegWriteByte(TCHAR *valuename, byte value);

int regCountSubKeys();

void ShowError(LONG er);

int IsIntermec(void);
int ReadBuildNumber(TCHAR *szBuildNumber);
int ReadPlatformName(TCHAR *szPlatformName);

int RegWriteBytes(TCHAR *valuename, byte* value, int iSize);
int RegReadByteSize(TCHAR *valuename, int &iSize);
int RegReadBytes(TCHAR *valuename, byte *value, int iSize);

//===============================================================
#else
#pragma message("registry.h was already included")
#endif //REGISTRYTOOLLOADED