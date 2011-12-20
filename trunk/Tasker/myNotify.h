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

extern SYSTEMTIME g_CurrentStartTime;
//extern __time64_t g_tmCurrentStartTime;
extern struct tm  g_tmCurrentStartTime;

const __int64 nano100SecInDay=(__int64)10000000*60*60*24;
const __int64 nano10Minutes=(__int64)10000000*60*10;

struct tm getLocalTime(tm* pLocalTime);

SYSTEMTIME convertTM2SYSTEMTIME(SYSTEMTIME *systemTime, struct tm *tmTime);
struct tm convertSystemTime2TM( struct tm *tmTime, SYSTEMTIME *systemTime);

SYSTEMTIME DT_Add(SYSTEMTIME& Date, short Years, short Months, short Days, short Hours, short Minutes, short Seconds, short Milliseconds);

SYSTEMTIME AddDiff(SYSTEMTIME* pst, int minutes);

SYSTEMTIME DT_AddDay(const SYSTEMTIME st);

SYSTEMTIME DT_AddDiff (const __int64 datepart, /*datepart with we want to manipulate, {nano100SecInDay ...}*/
			const __int64 num, /*value used to increment/decrement datepart*/
			const SYSTEMTIME* pst /*valid datetime which we want change*/);

SYSTEMTIME& newSystemTime(SYSTEMTIME& systemTime, LPCWSTR strDateTime);

SYSTEMTIME createDelayedNextSchedule(SYSTEMTIME stNext, short shDays, short shHour, short shMin);
SYSTEMTIME getNextTime(SYSTEMTIME stStart, SYSTEMTIME stBegin, int iIntervalDays, int iIntervalHours, int iIntervalMinutes);

SYSTEMTIME createNextSchedule(SYSTEMTIME stNext, short shDays, short shHour, short shMin);
struct tm *createNextSchedule(struct tm *tmNext, short shDays, short shHour, short shMin);
struct tm createNextSchedule(struct tm tmNext, short shDays, short shHour, short shMin);

void dumpST(SYSTEMTIME st);
void dumpST(TCHAR* szNote, SYSTEMTIME st);
void dumpTM(TCHAR* szNote, struct tm tmTime);

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

int isNewer2(SYSTEMTIME stFirst, SYSTEMTIME stSecond);
BOOL isNewer(SYSTEMTIME stNew, SYSTEMTIME stCompare);

void listNotifications();

BOOL isACpowered();


TCHAR* wasctime_s(TCHAR* buf, const __time64_t* time64, int dwSize);



