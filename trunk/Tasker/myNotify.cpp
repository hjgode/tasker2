#pragma once

#include "stdafx.h"
#include "myNotify.h"
#include "task.h"
#include "notify.h"
#include "tlhelp32.h"

#pragma comment(lib, "toolhelp.lib")

//#include "pkfuncs.h"

TCHAR str[MAX_PATH];

// save inTime, get LocalTime and then set DAY, hour and minute of inTime
// added support for negaitive days, hours and minutes in v2.34
SYSTEMTIME DT_Add(SYSTEMTIME& Date, short Years, short Months, short Days, short Hours, short Minutes, short Seconds, short Milliseconds) {
	FILETIME ft; SYSTEMTIME st; ULARGE_INTEGER ul1;
	
	SYSTEMTIME inTime;
	//v2.28 GetLocalTime(&inTime); //actual time and date
	extern SYSTEMTIME g_CurrentStartTime;
	memcpy(&inTime, &g_CurrentStartTime, sizeof(SYSTEMTIME));
	inTime.wDay = Date.wDay;
	inTime.wHour = Date.wHour;
	inTime.wMinute = Date.wMinute;
	inTime.wSecond=0;
	inTime.wMilliseconds=0;

	memcpy((void*)&Date, &inTime, sizeof(SYSTEMTIME));

	if (!SystemTimeToFileTime(&Date, &ft))
	{
		DEBUGMSG(1, (L"DT_Add: error in SystemTimeToFileTime: %i\n", GetLastError()));
		return Date;
	}
	ul1.HighPart = ft.dwHighDateTime;
	ul1.LowPart = ft.dwLowDateTime;
	 
	if (Milliseconds) 
		ul1.QuadPart += (Milliseconds * 10000); 

	if (Seconds)
		ul1.QuadPart += (Seconds * (__int64)10000000); 

	if (Minutes>0)
		ul1.QuadPart += (Minutes * (__int64)10000000 * 60); 
	else if (Minutes<0)
		ul1.QuadPart += (Minutes * (__int64)10000000 * 60); 

	if (Hours>0) 
		ul1.QuadPart += (Hours * (__int64)10000000 * 60 * 60);
	else if (Hours<0)
		ul1.QuadPart += (Hours * (__int64)10000000 * 60 * 60);

	if (Days>0)
		ul1.QuadPart += (Days * (__int64)10000000 * 60 * 60 * 24); 
	else if (Days<0)
		ul1.QuadPart += (Days * (__int64)10000000 * 60 * 60 * 24); 
	 
	ft.dwHighDateTime = ul1.HighPart;
	ft.dwLowDateTime = ul1.LowPart;
	 
	if (!FileTimeToSystemTime(&ft,&st)) {
		return Date;
	}
	 
	if (Months>0) {
		if ((Months += st.wMonth) <= 0) {
			Months *= (-1);
			st.wYear -= ((Months / 12) + 1);
			st.wMonth = 12 - (Months % 12);
		} else {
			st.wMonth = Months % 12;
			st.wYear += Months / 12;
		}
		while (!SystemTimeToFileTime(&st, &ft)) {
			st.wDay -= 1;
		}
	}
	return st;
}

SYSTEMTIME DT_AddDay(const SYSTEMTIME st){
	SYSTEMTIME stNow;
	extern SYSTEMTIME g_CurrentStartTime;//v2.28 
	memcpy(&stNow, &g_CurrentStartTime, sizeof(SYSTEMTIME));
	//GetLocalTime(&stNow);
	
	stNow.wHour=st.wHour;
	stNow.wMinute=st.wMinute;
	stNow.wSecond=st.wSecond;

	SYSTEMTIME stNew = DT_AddDiff(nano100SecInDay, 1, &stNow);

	return stNew;
}

////////////
//equivalent of DATEADD function from SQLServer
//Returns a new datetime value based on adding an interval
// to the specified date.
////////////*/
SYSTEMTIME /*new datetime*/
DT_AddDiff
			(	const __int64 datepart, /*datepart with we want to manipulate, 
			{nano100SecInDay ...}*/
			const __int64 num, /*value used to increment/decrement datepart*/
			const SYSTEMTIME* pst /*valid datetime which we want change*/
			)
{
	FILETIME ft;
	SYSTEMTIME st;
	__int64* pi; 

	SystemTimeToFileTime (pst,&ft); 
	pi = (__int64*)&ft; 
	(*pi) += (__int64)num*datepart; 

	/*convert FILETIME to SYSTEMTIME*/
	FileTimeToSystemTime (&ft,&st); 

	/*now, st contain new valid datetime, so return it*/
	return st;
}

SYSTEMTIME AddDiff(SYSTEMTIME* pst, int minutes){
	FILETIME ft;
	SYSTEMTIME st;
	__int64* pi; 
	//LARGE_INTEGER li;

	/*convert SYSTEMTIME to FILETIME*/
	SystemTimeToFileTime (pst,&ft); 

	SystemTimeToFileTime (pst,&ft); 

	pi = (__int64*)&ft; 
	(*pi) += (__int64)minutes*(__int64)10000000*60; 
    
	//li.LowPart = ft.dwLowDateTime;
    //li.HighPart = ft.dwHighDateTime;

	/*convert FAILETIME to SYSTEMTIME*/
	FileTimeToSystemTime (&ft,&st); 

	/*now, st contain new valid datetime, so return it*/
	return st;

}

double minutes_between(const FILETIME & from, const FILETIME & to)
{
	/*
	stStartTime = '201111160615', current time = '201111160611'
	interval is: 0d00h10m
	delta is -8 minutes
	*/
	enum {
		TICKS_PER_DAY = 60 * 10000000,
	};
	if( ((__int64 *) & to) > ((__int64 *) & from) )
		return double((*(__int64 *) & to) - (*(__int64 *) & from)) / TICKS_PER_DAY;
	else
		return double((*(__int64 *) & from) - (*(__int64 *) & to)) / TICKS_PER_DAY;
}

// test if st1 is before st2(current time)
int /*int*/ stDeltaMinutes(const SYSTEMTIME st1, const SYSTEMTIME st2)
{
	int iDelta=0;
	if(st1.wYear != st2.wYear)
		return 9999;
	if(st1.wMonth!=st2.wMonth)
		return 8888;
	if(st1.wDay != st2.wDay)
		return 7777;

	DWORD dwMin1 = st1.wHour * 60 + st1.wMinute;
	DWORD dwMin2 = st2.wHour * 60 + st2.wMinute;
	//if(dwMin1>dwMin2)
	//	iDelta = dwMin1-dwMin2;
	//else
	//	iDelta = dwMin2-dwMin1;
	iDelta = dwMin2 - dwMin1; //uhrzeit 2 ist sp�ter als uhrzeit 1 dann positiv (delayed schedule)

	DEBUGMSG(1, (L"DEBUG: Time diff in minutes: %i\n", iDelta));
    return iDelta;// abs(iTimeDelta/60);
}

/*
	compare two SYSTEMTIME values
	return 0, if both are equal
	return -1, if first time is before second time
	return 1, if first time is after second time
	        ............stFirst..........
	stSecond    -1         0        +1
	like wcscmp(...)
*/
int isNewer2(SYSTEMTIME stFirst, SYSTEMTIME stSecond){
	int iReturn=0;
	// 201112011415 01.12.2011 14:15 , 12 digits
	double dFirst = stFirst.wYear * 100000000 + stFirst.wMonth * 1000000 + stFirst.wDay * 10000 + stFirst.wHour * 100 + stFirst.wMinute;
	double dSecond = stSecond.wYear * 100000000 + stSecond.wMonth * 1000000 + stSecond.wDay * 10000 + stSecond.wHour * 100 + stSecond.wMinute;
	if(dSecond==dFirst)
		iReturn=0;
	if(dFirst<dSecond)
		iReturn=-1;
	else if (dFirst>dSecond)
		iReturn=1;
	return iReturn;
}

BOOL isNewer(SYSTEMTIME stNew, SYSTEMTIME stCompare){
	BOOL bRet=FALSE;
	//compare two dates by converting them first to a number like yyyyMMdd
	UINT dateNew = stNew.wYear * 10000 + stNew.wMonth * 100 + stNew.wDay;
	UINT dateCompare = stCompare.wYear * 10000 + stCompare.wMonth * 100 + stCompare.wDay;
	if(dateNew > dateCompare)
		return TRUE;
	
	//compare two times by converting them first to a number like hhmm
	UINT timeNew		= stNew.wHour * 100 + stNew.wMinute ;// + stNew.wSecond;
	UINT timeCompare	= stCompare.wHour * 100 + stCompare.wMinute ;// + stCompare.wSecond;

	if(timeNew > timeCompare){
		DEBUGMSG(1, (L"stNew is newer than stCompare\n"));
		return TRUE;
	}
	DEBUGMSG(1, (L"stNew is newer than stCompare\n"));
	return FALSE;
}

BOOL isNewerOLD(SYSTEMTIME stNew, SYSTEMTIME stCompare){
	BOOL bRet=FALSE;
	//compare two systimes by converting them first to a number like yyyyMMddhhmmss
	UINT dateNew = stNew.wYear * 10000 + stNew.wMonth * 100 + stNew.wDay;
	UINT dateCompare = stCompare.wYear * 10000 + stCompare.wMonth * 100 + stCompare.wDay;
	if(dateNew > dateCompare)
		return TRUE;

	UINT timeNew		= stNew.wHour * 60 * 60 + stNew.wMinute * 60;// + stNew.wSecond;
	UINT timeCompare	= stCompare.wHour * 60 * 60 + stCompare.wMinute * 60;// + stCompare.wSecond;

	if(timeNew > timeCompare){
		DEBUGMSG(1, (L"stNew is newer than stCompare\n"));
		return TRUE;
	}
	DEBUGMSG(1, (L"stNew is newer than stCompare\n"));
	return FALSE;
}


HRESULT ScheduleRunApp(
  LPCTSTR szExeName,
  LPCTSTR szArgs,
  SYSTEMTIME stTime)
{
	HRESULT hr = S_OK;
	HANDLE hNotify = NULL;

	// set a CE_NOTIFICATION_TRIGGER
	CE_NOTIFICATION_TRIGGER notifTrigger;
	memset(&notifTrigger, 0, sizeof(CE_NOTIFICATION_TRIGGER));
	notifTrigger.dwSize = sizeof(CE_NOTIFICATION_TRIGGER);

	// calculate time
	SYSTEMTIME st = {0};
	//GetLocalTime(&st); //v.2.28

	/*
	st = DT_AddDiff(nano100SecInDay, 1, &st);
	st.wHour = 3;
	st.wMinute=0;
	st.wSecond=33;
	*/
	//st = AddDiff(&st, 3); //wake in 3 minutes
	//st = DT_AddDiff(nano100SecInDay, 1, &stTime); //add one day
	st = stTime; //use provided new datetime

	wsprintf(str, L"Next run at: %02i.%02i.%02i %02i:%02i:%02i", 
										st.wDay, st.wMonth , st.wYear, 
										st.wHour , st.wMinute , st.wSecond );
	nclog(L"\tScheduleRunApp: %s\n", str);
	
	notifTrigger.dwType = CNT_TIME;
	notifTrigger.stStartTime = st;

	// timer: execute an exe at specified time
	notifTrigger.lpszApplication = (LPTSTR)szExeName;
	notifTrigger.lpszArguments = (LPTSTR)szArgs;

	hNotify = CeSetUserNotificationEx(0, &notifTrigger, NULL);
	// NULL because we do not care the action
	if (!hNotify) {
		hr = E_FAIL;
		nclog(L"\tScheduleRunApp: CeSetUserNotificationEx FAILED...\n");
	} else {
		// close the handle as we do not need to use it further
		CloseHandle(hNotify);
		nclog(L"\tScheduleRunApp: CeSetUserNotificationEx succeeded...\n");
	}  
	return hr;
} 

HRESULT ScheduleRunApp(
  LPCTSTR szExeName,
  LPCTSTR szArgs)
{
	//do not add a schedule if actual date is 21.3.2003
	SYSTEMTIME t;
	memset(&t, 0, sizeof(SYSTEMTIME));
	extern SYSTEMTIME g_CurrentStartTime;
	memcpy(&t, &g_CurrentStartTime, sizeof(SYSTEMTIME));
	//GetLocalTime(&t); //v.2.28
	//check if the system clock is at factory default, device specific!
	if ( (t.wYear == 2003) && (t.wMonth == 3) && (t.wDay == 21) )
	{
		nclog(L"ScheduleRunApp2: # no next run schedule as date is 21.03.2003!\n");
		return NOERROR;
	}

	

	HRESULT hr = S_OK;
	HANDLE hNotify = NULL;

	// set a CE_NOTIFICATION_TRIGGER
	CE_NOTIFICATION_TRIGGER notifTrigger;
	memset(&notifTrigger, 0, sizeof(CE_NOTIFICATION_TRIGGER));
	notifTrigger.dwSize = sizeof(CE_NOTIFICATION_TRIGGER);

	// calculate time
	SYSTEMTIME st = {0};
	extern SYSTEMTIME g_CurrentStartTime;
	memcpy(&st, &g_CurrentStartTime, sizeof(SYSTEMTIME));
	//GetLocalTime(&st);

	/*
	st = DT_AddDiff(nano100SecInDay, 1, &st);
	st.wHour = 3;
	st.wMinute=0;
	st.wSecond=33;
	*/
	st = AddDiff(&st, 3); //wake in 3 minutes
	wsprintf(str, L"ScheduleRunApp2: Next run at: %02i.%02i.%02i %02i:%02i:%02i", 
										st.wDay, st.wMonth , st.wYear, 
										st.wHour , st.wMinute , st.wSecond );
	nclog(L"ScheduleRunApp2: %s\n", str);
	
	notifTrigger.dwType = CNT_TIME;
	notifTrigger.stStartTime = st;

	// timer: execute an exe at specified time
	notifTrigger.lpszApplication = (LPTSTR)szExeName;
	notifTrigger.lpszArguments = (LPTSTR)szArgs;

	hNotify = CeSetUserNotificationEx(0, &notifTrigger, NULL);
	// NULL because we do not care the action
	if (!hNotify) {
		hr = E_FAIL;
		nclog(L"ScheduleRunApp2: CeSetUserNotificationEx FAILED...\n");
	} else {
		// close the handle as we do not need to use it further
		CloseHandle(hNotify);
		nclog(L"ScheduleRunApp2: CeSetUserNotificationEx succeeded...\n");
	}  
	return hr;
} 

HRESULT notiClearRunApp(LPCTSTR szExeName, LPCTSTR szArgs)
{
	HRESULT hr = S_OK;

	// hold a notification
	PBYTE pBuff = (PBYTE)LocalAlloc(LPTR, 8192);

	if (!pBuff) {
		return E_OUTOFMEMORY;
	}
	
	LONG lRes=0;

	TCHAR mExeName[MAX_PATH];
	wsprintf(mExeName, L"%s", szExeName);
	TCHAR mArgs[MAX_PATH];
	wsprintf(mArgs, L"%s", szArgs);

	// at most 256 notification handles
	HANDLE hNotifHandlers[256];
	DWORD nNumHandlers, nNumClearedHandlers = 0;
	DWORD i = 0;
	int rc = CeGetUserNotificationHandles(hNotifHandlers, 255, &nNumHandlers);
	if (!rc) {
		ULONG uErr = GetLastError();
		hr = E_FAIL;
		nclog(L"\tClearRunApp2: no more handles? in CeGetUserNotificationHandles()? GetLastError()=%u\n", uErr);
		lRes=-1;
		goto FuncExit;
	}
  
	// iterate all notifications
	// Notice: We do not care about the status of the notification.
	// Just clear it even if it is not filed??
	nclog(L"\tClearRunApp2: ...\n");
	for (i=0; i<nNumHandlers; i++) {
		// query info for this specific handler
		BOOL bClearThis = FALSE;
		DWORD dwSize = 0;
		//get size for buffer first?
		rc = CeGetUserNotification(hNotifHandlers[i], 8192, &dwSize, pBuff);
		if (!rc) continue;

		PCE_NOTIFICATION_INFO_HEADER pnih = (PCE_NOTIFICATION_INFO_HEADER)pBuff;

		PCE_NOTIFICATION_TRIGGER pNotifTrigger = pnih->pcent;

		//nclog(L"RunAtTimes, ClearRunApp(): %s\n", pNotifTrigger->lpszApplication);

		// Notice some events with NULL lpszApplication might be inserted!
		if ( pNotifTrigger && pNotifTrigger->lpszApplication ){
			if(pNotifTrigger->lpszApplication != NULL){
				if (wcsicmp(pNotifTrigger->lpszApplication, mExeName)==0) {
					if (wcsicmp(pNotifTrigger->lpszArguments, mArgs)==0) {
						nclog(L"\tClearRunApp2: CeClearUserNotification for handle: 0x%0x\n", pnih->hNotification);
					if(CeClearUserNotification(pnih->hNotification))
						lRes++; //how many entries deleted?
					else
						nclog(L"\tFailed to clear schedule for '%s'\n", pNotifTrigger->lpszApplication);
					}
				}
			}
		}
	}
  
	FuncExit:
	//nclog(L"##### RunAtTimes, ClearRunApp2():FuncExit ############\n");
	if (pBuff) {
		LocalFree(pBuff);
	}
	nclog(L"\tClearRunApp2: returning with %i\n", lRes);
	return lRes;
}

HRESULT notiClearRunApp(LPCTSTR szExeName)
{
	HRESULT hr = S_OK;
	LONG lRes=0; //count deleted entries

	// hold a notification
	PBYTE pBuff = (PBYTE)LocalAlloc(LPTR, 8192);

	if (!pBuff) {
		return E_OUTOFMEMORY;
	}

	TCHAR mExeName[MAX_PATH];
	wsprintf(mExeName, L"%s", szExeName);

	// at most 256 notification handles
	HANDLE hNotifHandlers[256];
	DWORD nNumHandlers, nNumClearedHandlers = 0;
	DWORD i = 0;
	int rc = CeGetUserNotificationHandles(hNotifHandlers, 255, &nNumHandlers);
	if (!rc) {
		ULONG uErr = GetLastError();
		hr = E_FAIL;
		nclog(L"ClearRunApp(): no more handles? in CeGetUserNotificationHandles()? GetLastError()=%u\n", uErr);
		goto FuncExit;
	}
  
	// iterate all notifications
	// Notice: We do not care about the status of the notification.
	// Just clear it even if it is not filed??
	nclog(L"ClearRunApp(): ...\n");
	for (i=0; i<nNumHandlers; i++) {
		// query info for this specific handler
		BOOL bClearThis = FALSE;
		DWORD dwSize = 0;
		//get size for buffer first?
		rc = CeGetUserNotification(hNotifHandlers[i], 8192, &dwSize, pBuff);
		if (!rc) continue;

		PCE_NOTIFICATION_INFO_HEADER pnih = (PCE_NOTIFICATION_INFO_HEADER)pBuff;

		PCE_NOTIFICATION_TRIGGER pNotifTrigger = pnih->pcent;

		//nclog(L"RunAtTimes, ClearRunApp(): %s\n", pNotifTrigger->lpszApplication);

		// Notice some events with NULL lpszApplication might be inserted!
		if ( pNotifTrigger && pNotifTrigger->lpszApplication ){
			if(pNotifTrigger->lpszApplication != NULL){
				if (wcsicmp(pNotifTrigger->lpszApplication, mExeName)==0) {
					nclog(L"ClearRunApp(): CeClearUserNotification for handle: 0x%0x\n", pnih->hNotification);
					if(CeClearUserNotification(pnih->hNotification))
						lRes++;
					else
						nclog(L"ClearRunApp(): Failed to clear schedule for '%s'\n", pNotifTrigger->lpszApplication);
				}
			}
		}
	}
  
	FuncExit:
	if (pBuff) {
		LocalFree(pBuff);
	}

	nclog(L"ClearRunApp(): returns %i \n",lRes);
	return lRes; //hr;
}

/*
	clear all event based schedules
*/
int ClearRunAppAtTimeChangeEvents(TCHAR FileName[MAX_PATH+1]){
	int iRet = 0;
	nclog(L"Clearing Event Notifications...");
	if(CeRunAppAtEvent(FileName, NOTIFICATION_EVENT_NONE)){
		nclog(L"OK\n");
	}
	else{
		iRet=GetLastError();
		nclog(L"Failed: %i\n", iRet);
	}
	return iRet;
}

/*
	delete and create new Time_Change and TZ_Change notifications
*/
int RunAppAtTimeChangeEvents(TCHAR FileName[MAX_PATH+1]){
	int iRet = 0;
	nclog(L"Clearing Event Notifications...");
	ClearRunAppAtTimeChangeEvents(FileName);

	nclog(L"Adding Time_Change Event Notification...");
	if(CeRunAppAtEvent(FileName, NOTIFICATION_EVENT_TIME_CHANGE))
		nclog(L"OK\n");
	else
		nclog(L"Failed: %i\n", GetLastError());

	nclog(L"Adding TZ_Change Event Notification...");
	if(CeRunAppAtEvent(FileName, NOTIFICATION_EVENT_TZ_CHANGE))
		nclog(L"OK\n");
	else
		nclog(L"Failed: %i\n", GetLastError());
	
	return iRet;
}

/*
void ClearRunAppAtTime(TCHAR FileName[MAX_PATH+1])
{
	CeRunAppAtTime(FileName, 0);
}
*/

///start app next day at 00:59:59
int RunAppAtTime(TCHAR FileName[MAX_PATH+1])
{
	//get actual time
	SYSTEMTIME t;
	memset(&t, 0, sizeof(SYSTEMTIME));
	extern SYSTEMTIME g_CurrentStartTime;
	memcpy(&t, &g_CurrentStartTime, sizeof(SYSTEMTIME));
	//GetLocalTime(&t); //v2.28

	SYSTEMTIME newTime;
	//add one day
	newTime = DT_AddDiff(nano100SecInDay, 1, &t);

	newTime.wHour = 0;
	newTime.wMinute = 59;
	newTime.wSecond = 59;

	wsprintf(str, L"Next run at: %02i.%02i.%02i %02i:%02i:%02i", 
										newTime.wDay, newTime.wMonth , newTime.wYear, 
										newTime.wHour , newTime.wMinute , newTime.wSecond );
	nclog(L"RunAppAtTime(): %s\n", str);

	return CeRunAppAtTime(FileName, &t);

}

///start app in one day at same time
int RunAppAtTime(TCHAR FileName[MAX_PATH+1], SYSTEMTIME nextTime)
{
	//get actual time
	//SYSTEMTIME t;
	//memset(&t, 0, sizeof(SYSTEMTIME));
	//GetLocalTime(&t);

	SYSTEMTIME newTime;
	//add one day
	newTime = DT_AddDiff(nano100SecInDay, 1, &nextTime);

	//newTime.wHour = 0;
	//newTime.wMinute = 59;
	//newTime.wSecond = 59;

	wsprintf(str, L"Next run at: %02i.%02i.%02i %02i:%02i:%02i", 
										newTime.wDay, newTime.wMonth , newTime.wYear, 
										newTime.wHour , newTime.wMinute , newTime.wSecond );
	nclog(L"RunAppAtTime2(): %s\n", str);

	return CeRunAppAtTime(FileName, &newTime);

}

int runExe(TCHAR* szExe, TCHAR* szArg)
{
	PROCESS_INFORMATION pi;
	memset(&pi,0,sizeof(pi));
	BOOL bRes = CreateProcess(szExe, szArg, NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi);
	if(bRes){
		nclog(L"runExe OK for '%s'\n", szExe);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return 0;
	}
	else{
		int iErr=GetLastError();
		nclog(L"runExe failed for '%s', GetLastError=%i\n", szExe, iErr);
		return iErr;
	}
}

//##############################################
//				Kill process stuff
//##############################################

HANDLE GetProcessHandle(WCHAR *wszProcessName)
{
    HANDLE hTH= CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPNOHEAPS, 0 );
	if(hTH==INVALID_HANDLE_VALUE){
		DEBUGMSG(1, (L"CreateToolhelp32Snapshot: %08x\n", GetLastError()));
		return INVALID_HANDLE_VALUE;
	}

    PROCESSENTRY32 pe;
    pe.dwSize= sizeof(PROCESSENTRY32);
    HANDLE hProc= INVALID_HANDLE_VALUE;
    if (Process32First(hTH, &pe))
    {
        do {
            if (wcsicmp(wszProcessName, pe.szExeFile)==0)
            {
                hProc= OpenProcess(0, 0, pe.th32ProcessID);
                if (hProc != INVALID_HANDLE_VALUE && hProc!=NULL)
                    break;
            }
        } while (Process32Next(hTH, &pe));
    }

    CloseToolhelp32Snapshot(hTH);
    return hProc;
}

//pkfuncs.h
extern "C" BOOL SetKMode(BOOL bFlag);
extern "C" DWORD SetProcPermissions(DWORD dwPerms);

class KernelMode {
private:
    BOOL bMode;
    DWORD dwPerm;
public:
    KernelMode()
    {
        bMode = SetKMode(TRUE);
        dwPerm = SetProcPermissions(0xFFFFFFFF);
    }
    ~KernelMode()
    {
        SetProcPermissions(dwPerm);
        SetKMode(bMode);
    }
};

DWORD killExe(TCHAR* szExe){
	//cut path from szExe
	TCHAR* strNew = wcsrchr(szExe, L'\\');
	strNew++;

	KernelMode _km;

    HANDLE hProc;
    hProc= GetProcessHandle(strNew);

    if (hProc==NULL || hProc==INVALID_HANDLE_VALUE) {
        return ERROR_NOT_FOUND;
    }

	DWORD res= 0;
	if (!TerminateProcess(hProc, 0)){
		res= GetLastError();
	}
    CloseHandle(hProc);

	return res;
}

void listNotifications(){
	HRESULT hr = S_OK;
	LONG lRes=0; //count deleted entries

	// hold a notification
	PBYTE pBuff = (PBYTE)LocalAlloc(LPTR, 8192);

	if (!pBuff) {
		return;// E_OUTOFMEMORY;
	}

	// at most 256 notification handles
	HANDLE hNotifHandlers[256];
	DWORD nNumHandlers, nNumClearedHandlers = 0;
	DWORD i = 0;
	int rc = CeGetUserNotificationHandles(hNotifHandlers, 255, &nNumHandlers);
	if (!rc) {
		ULONG uErr = GetLastError();
		hr = E_FAIL;
		nclog(L"listNotifications(): no more handles? in CeGetUserNotificationHandles()? GetLastError()=%u\n", uErr);
		goto FuncExit;
	}
  
	// iterate all notifications
	// Notice: We do not care about the status of the notification.
	// Just clear it even if it is not filed??
	nclog(L"========= List of Notification entries: ===============\n");
	for (i=0; i<nNumHandlers; i++) {
		// query info for this specific handler
		BOOL bClearThis = FALSE;
		DWORD dwSize = 0;
		//get size for buffer first?
		rc = CeGetUserNotification(hNotifHandlers[i], 8192, &dwSize, pBuff);
		if (!rc) 
			continue;

		PCE_NOTIFICATION_INFO_HEADER pnih = (PCE_NOTIFICATION_INFO_HEADER)pBuff;

		PCE_USER_NOTIFICATION pnun = (CE_USER_NOTIFICATION*)pnih->pceun;
		
		PCE_NOTIFICATION_TRIGGER pNotifTrigger = pnih->pcent;

		//what type
		TCHAR* strType[5]={ L"zero", L"Event", L"Time", L"Period", L"ClassicTime" } ; // decode pNotifTrigger->dwType

		//if event, what event
		TCHAR strAction[MAX_PATH]; wsprintf(strAction, L"");
		if(pnun){
			switch(pnun->ActionFlags){
				case PUN_LED:
					wcscat(strAction, L"LED ");
					break;
				case PUN_VIBRATE:
					wcscat(strAction, L"Vibrate ");
					break;
				case PUN_DIALOG:
					wcscat(strAction, L"Dialog ");
					wcscat(strAction, L"'");
					wcscat(strAction, pnun->pwszDialogText);
					wcscat(strAction, L"'");
					break;
				case PUN_SOUND:
					wcscat(strAction, L"Sound ");
					break;
				case PUN_REPEAT:
					wcscat(strAction, L"Repeat ");
					break;
				case PUN_PRIVATE:
					wcscat(strAction, L"Private ");
					break;
				default:
					wcscat(strAction, L"nf");
			}
		}
		else
			wcscat(strAction, L"-none-");
		wcscat(strAction, L"\0");

		//get time entries
		TCHAR strTimes[MAX_PATH]; wsprintf(strTimes, L"");

		TCHAR strTemp[MAX_PATH]; wsprintf(strTemp, L"");
		if(pNotifTrigger->dwType==CNT_TIME || pNotifTrigger->dwType==CNT_CLASSICTIME || pNotifTrigger->dwType==CNT_PERIOD){
			SYSTEMTIME st = pNotifTrigger->stStartTime;
			wsprintf(strTemp, L"Start: %02i.%02i.%04i/%02i:%02i, ",
				st.wDay, st.wMonth, st.wYear,
				st.wHour,st.wMinute);
			wcscat(strTimes, strTemp);
			st = pNotifTrigger->stEndTime;
			wsprintf(strTemp, L"End: %02i.%02i.%04i/%02i:%02i",
				st.wDay, st.wMonth, st.wYear,
				st.wHour,st.wMinute);
			wcscat(strTimes, strTemp);
		}

		TCHAR strApp[MAX_PATH];wsprintf(strApp, L"");
		if(pNotifTrigger->lpszApplication!=NULL)
			wsprintf(strApp, pNotifTrigger->lpszApplication);
		else
			wsprintf(strApp, L"-no app-");
		
		TCHAR strArgs[MAX_PATH];wsprintf(strArgs, L"");
		if(pNotifTrigger->lpszArguments!=NULL)
			wsprintf(strArgs, pNotifTrigger->lpszArguments);
		else
			wsprintf(strArgs, L"-no args-");

		nclog(L"\tType:   %s\n", strType[pNotifTrigger->dwType] );
		nclog(L"\tAction: %s\n", strAction);
		nclog(L"\tTimes:  %s\n", strTimes);
		nclog(L"\tApp:    %s\n", strApp);
		nclog(L"\tArgs:   %s\n", strArgs);
		nclog(L"\t--------------------------\n");
		delete(pnun);
	}
  
	FuncExit:
	if (pBuff) {
		LocalFree(pBuff);
	}

	nclog(L"========== End of Notification entries: ===============\n");
	return;// lRes; //hr;
}
