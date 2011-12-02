#include "stdafx.h"
#include "regRW.h"
#include "common/nclog.h"

/*
#include <itc50.h>
#pragma comment (lib, "itc50.lib")

#include "log2file.h"
*/
const __int64 nano100SecInDay=(__int64)10000000*60*60*24;
const __int64 nano10Minutes=(__int64)10000000*60*10;

SYSTEMTIME DT_AddDiff(const __int64 datepart, const __int64 num, const SYSTEMTIME* pst);

SYSTEMTIME DT_Add(SYSTEMTIME& Date, short Years, short Months, short Days, short Hours, short Minutes, short Seconds, short Milliseconds);

SYSTEMTIME AddDiff(SYSTEMTIME* pst, int minutes);

HRESULT ScheduleRunApp(
  LPCTSTR szExeName,
  LPCTSTR szArgs);

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
int RunAppAtTime(TCHAR FileName[MAX_PATH+1], SYSTEMTIME nextTime);

int ClearRunAppAtTimeChangeEvents(TCHAR FileName[MAX_PATH+1]);
int RunAppAtTimeChangeEvents(TCHAR FileName[MAX_PATH+1]);

HRESULT ScheduleRunApp(LPCTSTR szExeName, LPCTSTR szArgs, SYSTEMTIME stTime);

int doProcessCommandLine(TCHAR* strCommandLine, TASK* _task);
BOOL ProcessCommandLine(int argc, TCHAR *argv[], TASK* _task);

int runExe(TCHAR* szExe, TCHAR* szArg);
DWORD killExe(TCHAR* szExe);

SYSTEMTIME DT_AddDay(const SYSTEMTIME st);
int stDeltaMinutes(const SYSTEMTIME st1, const SYSTEMTIME st2);
BOOL isNewer(SYSTEMTIME stNew, SYSTEMTIME stCompare);

void listNotifications();

