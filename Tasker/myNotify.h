//myNotify.h

#ifndef MYNOTIFY_H
#define MYNOTIFY_H

//if for emulator test, then undef INTERMEC
//#define INTERMEC

#include "stdafx.h"
#include "regRW.h"
#include "common/nclog.h"

//#include "time.h"
#include <altcecrt.h>
//#pragma comment(lib, "crt.lib")

/*
#include <itc50.h>
#pragma comment (lib, "itc50.lib")

#include "log2file.h"
*/

extern struct tm  g_tmCurrentStartTime;

struct tm getLocalTime(tm* pLocalTime);

SYSTEMTIME convertTM2SYSTEMTIME(SYSTEMTIME *systemTime, struct tm *tmTime);
struct tm convertSystemTime2TM( struct tm *tmTime, SYSTEMTIME *systemTime);

struct tm *createNextSchedule(struct tm *tmNext, short shDays, short shHour, short shMin);
struct tm createNextSchedule(struct tm tmNext, short shDays, short shHour, short shMin);

void dumpST(SYSTEMTIME st);
void dumpST(TCHAR* szNote, SYSTEMTIME st);
void dumpTM(TCHAR* szNote, struct tm tmTime);

HRESULT ScheduleRunApp(LPCTSTR szExeName, LPCTSTR szArgs);

//a enum to hold the action to take
enum _DoAction{
	start,
	kill,
	remove,
	clear
};

extern _DoAction _ActionCode;

HRESULT notiClearRunApp(LPCTSTR , LPCTSTR );
HRESULT notiClearRunApp(LPCTSTR );

int RunAppAtTime(TCHAR FileName[MAX_PATH+1]);

int ClearRunAppAtTimeChangeEvents(TCHAR FileName[MAX_PATH+1]);
int RunAppAtTimeChangeEvents(TCHAR FileName[MAX_PATH+1]);

HRESULT ScheduleRunApp(LPCTSTR szExeName, LPCTSTR szArgs, struct tm tmTime);

int doProcessCommandLine(TCHAR* strCommandLine, TASK* _task);
BOOL ProcessCommandLine(int argc, TCHAR *argv[], TASK* _task);

int runExe(TCHAR* szExe, TCHAR* szArg);
DWORD killExe(TCHAR* szExe);

void listNotifications();

BOOL isACpowered();

TCHAR* wasctime_s(TCHAR* buf, const __time64_t* time64, int dwSize);

#endif //MYNOTIFY_H