#pragma once

#include "stdafx.h"
#include "myNotify.h"
#include "task.h"
#include "notify.h"
#include "tlhelp32.h"

#pragma comment(lib, "toolhelp.lib")

TCHAR str[MAX_PATH];

/*
   struct tm  when;
   __time64_t now, result;
   int        days;
   TCHAR       buff[80];

   _time64( &now );	//get the system time
   _localtime64_s( &when, &now );	//convert a system time to a local tm time and correct for local time zone
   
   wprintf( L"Current time is %s\n", wasctime_s(buff, &now, 80));// L"_wasctime not implemented" );
   days = 20;
   int hours = 24;
   when.tm_mday = when.tm_mday + days;
   if( (result = _mktime64( &when )) != (time_t)-1 ) { //convert local tm time to calendar time
      wprintf( L"In %d days the time will be %s\n", days, wasctime_s(buff, &result, 80) );
   } else
      DEBUGMSG(1, ( L"_mktime64 failed" ) );
   when.tm_hour = when.tm_hour + hours;
   if( (result = _mktime64( &when )) != (time_t)-1 ) { //convert local tm time to calendar time
      wprintf( L"In %d hours the time will be %s\n", hours, wasctime_s(buff, &result, 80) );
   } else
      DEBUGMSG(1, ( L"_mktime64 failed" ) );
*/

//--------------------------------------------------------------------
// Function name  : getLocalTime
// Description    : return the current local time
// Return type    : struct tm
//--------------------------------------------------------------------
struct tm getLocalTime(struct tm* pLocalTime){
	nclog(L"~~~ using actual localtime:\n");
	// START ----------- store the start time in a global var
	__time64_t now;
	_tzset();
    // Get UNIX-style time
	_time64( &now );	//get the system time
	_localtime64_s( pLocalTime, &now );	// convert to local time
//	g_tmCurrentStartTime = *pLocalTime;
	nclog(L"\t %02i.%02i.%04i, %02i:%02i\n",
		pLocalTime->tm_mday, pLocalTime->tm_mon+1, pLocalTime->tm_year +1900,
		pLocalTime->tm_hour, pLocalTime->tm_min);
	
	return *pLocalTime;
}

//--------------------------------------------------------------------
// Function name  : convertTM2SYSTEMTIME
// Description    : convert a struct tm to SYSTEMTIME
// Argument       : SYSTEMTIME *systemTime
// Argument       : struct tm *tmTime
// Return type    : SYSTEMTIME
//--------------------------------------------------------------------
SYSTEMTIME convertTM2SYSTEMTIME(SYSTEMTIME *systemTime, struct tm *tmTime){
	systemTime->wDay	= tmTime->tm_mday;
	systemTime->wDayOfWeek	= (tmTime->tm_wday);
	systemTime->wMonth	= (tmTime->tm_mon)+1;
	systemTime->wYear	= (tmTime->tm_year)+1900;
	systemTime->wHour	= tmTime->tm_hour;
	systemTime->wMinute	= tmTime->tm_min;
	systemTime->wSecond	= tmTime->tm_sec;
	systemTime->wMilliseconds	= 0;
	return *systemTime;
}

//--------------------------------------------------------------------
// Function name  : convertSystemTime2TM
// Description    : convert a SYSTEMTIME to a struct tm
// Argument       : struct tm *tmTime
// Argument       : SYSTEMTIME *systemTime
// Return type    : struct tm
//--------------------------------------------------------------------
struct tm convertSystemTime2TM( struct tm *tmTime, SYSTEMTIME *systemTime){
	tmTime->tm_mday		= systemTime->wDay;
	tmTime->tm_wday		= systemTime->wDayOfWeek;
	tmTime->tm_mon		= systemTime->wMonth-1; 
	tmTime->tm_year		= (systemTime->wYear)-1900; 
	tmTime->tm_hour		= systemTime->wHour;
	tmTime->tm_min		= systemTime->wMinute;
	tmTime->tm_sec		= systemTime->wSecond;
	
	return *tmTime;
}

//--------------------------------------------------------------------
// Function name  : wasctime_s
// Description    : return a string representating the time
// Argument       : TCHAR* buf
// Argument       : const __time64_t* time64
// Argument       : int dwSize
// Return type    : TCHAR*
//--------------------------------------------------------------------
TCHAR* wasctime_s(TCHAR* buf, const __time64_t* time64, int dwSize){
	struct tm tmTime;
	error_status_t eStat;
	if((eStat =_localtime64_s(&tmTime, time64))==0)
		wsprintf(buf, L"%04i%02i%02i%02i%02i", tmTime.tm_year+1900, tmTime.tm_mon+1, tmTime.tm_mday, tmTime.tm_hour, tmTime.tm_min);
	else{
		wsprintf(buf, L"");
		nclog(L"\terror %i in wasctime_s\n");
	}
	return buf;
}

//--------------------------------------------------------------------
// Function name  : isACpowered
// Description    : return if device is powered externally
// Return type    : BOOL
//--------------------------------------------------------------------
BOOL isACpowered(){
	nclog(L"\tchecking AC line status\n");
#ifndef INTERMEC
	SYSTEM_POWER_STATUS_EX2 sysPowr;
	GetSystemPowerStatusEx2(&sysPowr, sizeof(SYSTEM_POWER_STATUS_EX2), TRUE);
	if(sysPowr.ACLineStatus==AC_LINE_ONLINE)
		return TRUE;
	else
		return FALSE;
#else
	DWORD dwLineStatus=0;
	DWORD dwBatteryStatus=0;
	DWORD dwBackupBatteryStatus=0;
	UINT uFuelGauge=0;
	if (ITCPowerStatus(&dwLineStatus, &dwBatteryStatus, &dwBackupBatteryStatus, &uFuelGauge)==ITC_SUCCESS){
		nclog(L"\tITCPowerStatus() OK\n");
		if(dwLineStatus==ITC_ACLINE_CONNECTED){
			nclog(L"\tAC line connected\n");
			return TRUE;
		}
		else
			nclog(L"\tAC line not connected or undefined\n");
	}
	else
		nclog(L"\tITCPowerStatus() failed\n");
	return FALSE;
#endif
}

//--------------------------------------------------------------------
// Function name  : ScheduleRunApp
// Description    : add a schedule for exe with args at the time
// Argument       : LPCTSTR szExeName
// Argument       : LPCTSTR szArgs
// Argument       : struct tm tmTime
// Return type    : HRESULT, 0 for no error
//--------------------------------------------------------------------
HRESULT ScheduleRunApp(LPCTSTR szExeName, LPCTSTR szArgs, struct tm tmTime)
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

	st = convertTM2SYSTEMTIME(&st, &tmTime); //use provided new datetime

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


//--------------------------------------------------------------------
// Function name  : notiClearRunApp
// Description    : remove a schedule for exe with args 
// Argument       : LPCTSTR szExeName
// Argument       : LPCTSTR szArgs
// Return type    : HRESULT, 0 for no error
//--------------------------------------------------------------------
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

//--------------------------------------------------------------------
// Function name  : notiClearRunApp
// Description    : remove a schedule for exe 
// Argument       : LPCTSTR szExeName
// Return type    : HRESULT, 0 for no error
//--------------------------------------------------------------------
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

//--------------------------------------------------------------------
// Function name  : ClearRunAppAtTimeChangeEvents
// Description    : remove all EVENT schedules for the filename 
// Argument       : TCHAR FileName[MAX_PATH+1]
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
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

//--------------------------------------------------------------------
// Function name  : RunAppAtTimeChangeEvents
// Description    : delete and create new Time_Change and TZ_Change notifications 
// Argument       : TCHAR FileName[MAX_PATH+1]
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
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

//--------------------------------------------------------------------
// Function name  : runExe
// Description    : start the exe using CreateProcess 
// Argument       : TCHAR* szExe
// Argument       : TCHAR* szArg
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
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

//--------------------------------------------------------------------
// Function name  : GetProcessHandle
// Description    : find the handle for a process 
// Argument       : WCHAR *wszProcessName
// Return type    : HANDLE
//--------------------------------------------------------------------
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

//--------------------------------------------------------------------
// Function name  : killExe
// Description    : terminate a process 
// Argument       : TCHAR *szExe
// Return type    : DWORD, 0 for success
//--------------------------------------------------------------------
DWORD killExe(TCHAR* szExe){
	//cut path from szExe
//	TCHAR* strNew = wcsrchr(szExe, L'\\');
//	strNew++;

//	KernelMode _km;

    HANDLE hProc;
    hProc= GetProcessHandle(szExe);// strNew);

    if (hProc==NULL || hProc==INVALID_HANDLE_VALUE) {
        return ERROR_NOT_FOUND;
    }

	DWORD res= 0;
	if (!TerminateProcess(hProc, 0)){
		res= GetLastError();
	}
    CloseHandle(hProc);
//	_km.~KernelMode();
	return res;
}

//--------------------------------------------------------------------
// Function name  : listNotifications
// Description    : dump actual known schedules with nclog 
// Return type    : VOID
//--------------------------------------------------------------------
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


//--------------------------------------------------------------------
// Function name  : dumpTT64
// Description    : dump a __time64_t to DEBUG out 
// Argument       : TCHAR *szNote, string to prepend to output
// Argument       : __time64_t ttTime, the time_t to dump
// Return type    : VOID
//--------------------------------------------------------------------
void dumpTT64(TCHAR* szNote, __time64_t ttTime){
#if !DEBUG
	return;
#endif
	struct tm tmTime;
	_localtime64_s(&tmTime, &ttTime);

	TCHAR szStr[MAX_PATH];
	wsprintf(szStr, L"%04i%02i%02i %02i:%02i:%02i",
		(tmTime.tm_year)+1900, (tmTime.tm_mon)+1, tmTime.tm_mday,
		tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec);
	DEBUGMSG(1, (L"%s: %s\n", szNote, szStr));
}

//--------------------------------------------------------------------
// Function name  : dumpTM
// Description    : dump a 'struct tm' to DEBUG out 
// Argument       : TCHAR *szNote, string to prepend to output
// Argument       : struct tm tmTime, the time to dump
// Return type    : VOID
//--------------------------------------------------------------------
void dumpTM(TCHAR* szNote, struct tm tmTime){
#if !DEBUG
	return;
#endif
	TCHAR szStr[MAX_PATH];
	wsprintf(szStr, L"%04i%02i%02i %02i:%02i:%02i",
		(tmTime.tm_year)+1900, (tmTime.tm_mon)+1, tmTime.tm_mday,
		tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec);
	DEBUGMSG(1, (L"%s: %s\n", szNote, szStr));
}

//--------------------------------------------------------------------
// Function name  : dumpST
// Description    : dump a SYSTEMTIME to DEBUG out 
// Argument       : TCHAR *szNote, string to prepend to output
// Argument       : SYSTEMTIME st, the time to dump
// Return type    : VOID
//--------------------------------------------------------------------
void dumpST(TCHAR* szNote, SYSTEMTIME st){
#if !DEBUG
	return;
#endif
	TCHAR szStr[MAX_PATH];
	wsprintf(szStr, L"%04i%02i%02i %02i:%02i:%02i",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond);
	DEBUGMSG(1, (L"%s: %s\n", szNote, szStr));
}

//--------------------------------------------------------------------
// Function name  : dumpST
// Description    : dump a SYSTEMTIME to DEBUG out 
// Argument       : SYSTEMTIME st, the time to dump
// Return type    : VOID
//--------------------------------------------------------------------
void dumpST(SYSTEMTIME st){
#if !DEBUG
	return;
#endif
	TCHAR szStr[MAX_PATH];
	wsprintf(szStr, L"%04i%02i%02i %02i:%02i:%02i",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond);
	DEBUGMSG(1, (L"%s\n", szStr));
}

////////////////////////////////////////////////////////////////////
//	createNextSchedule
//	calculate the next schedule based on time and interval values
////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------
// Function name  : createNextSchedule
// Description    : calculate the next time starting at tmNext using days, hours and minutes 
// Argument       : struct tm *tmNext
// Argument       : short shDays
// Argument       : short shHour
// Argument       : short shMin
// Return type    : struct tm*, the calculated next time
//--------------------------------------------------------------------
//struct tm *createNextSchedule(struct tm *tmNext, short shDays, short shHour, short shMin){
//	TCHAR szTime[24] = {0};
//
//	if(shMin>=60){
//		shHour += (short)(shMin / 60);
//		shMin = (short)(shMin % 60);
//	}
//	if(shHour>=24){	//hour interval value is one day or more
//		shDays = (short) (shHour / 24);
//		shHour = (short) (shHour % 24);
//	}
//	nclog(L"\tcalculating new schedule for '%s'...\n", getLongStrFromTM(*tmNext));
//	nclog(L"\tinterval is: %id%02ih%02im\n", shDays, shHour, shMin);
//#if DEBUG
//	dumpTM(L"stNext", *tmNext);
//	dumpTM(L"stCurrentTime", g_tmCurrentStartTime);
//#endif
//	__time64_t ttNext = _mktime64(tmNext);	// time as seconds elapsed since midnight, January 1, 1970, or -1 in the case of an error.
//	__time64_t ttCurr = _mktime64(&g_tmCurrentStartTime);
//
//	dumpTT64(L"current time: ", ttCurr);
//	dumpTT64(L"next time:    ", ttNext);
//	//double dTime = difftime(mktime(&tmNext), mktime(&g_tmCurrentStartTime)); //difftime(endingTime, startTime)
//	//is the next schedule in future?
//	if(ttNext <= ttCurr){
//		do{
//			//add interval to stNewTime
//			tmNext->tm_mday+=shDays;
//			tmNext->tm_hour+=shHour;
//			tmNext->tm_min+=shMin;
//			ttNext=_mktime64(tmNext);
//			//ttNext = DT_Add(tmNext->tm_mday+shDays , 0, 0, shDays, shHour, shMin, 0, 0);// DT_AddDay(_Tasks[iTask].stStartTime);
//		}while (ttNext <= ttCurr);
//	}
//	ttNext=_mktime64(tmNext);
//	dumpTT64(L"\tschedule adjusted to ", ttNext);
//	//else
//	//	nclog(L"\tno schedule adjustement needed.\n");
//
//	nclog(L"\tschedule adjusted to '%s'\n", getLongStrFromTM(*tmNext));
//
//	return tmNext;
//}

//--------------------------------------------------------------------
// Function name  : createNextSchedule
// Description    : calculate the next time starting at tmNext using days, hours and minutes 
// Argument       : struct tm tmNext
// Argument       : short shDays
// Argument       : short shHour
// Argument       : short shMin
// Return type    : struct tm, the calculated next time
//--------------------------------------------------------------------
struct tm createNextSchedule(struct tm tmNext, short shDays, short shHour, short shMin){
	TCHAR szTime[24] = {0};
	struct tm tmReturn;
	memcpy(&tmReturn, &tmNext, sizeof(struct tm));

	if(shMin>=60){
		shHour += (short)(shMin / 60);
		shMin = (short)(shMin % 60);
	}
	if(shHour>=24){	//hour interval value is one day or more
		shDays = (short) (shHour / 24);
		shHour = (short) (shHour % 24);
	}
	nclog(L"\tcalculating new schedule for '%s'...\n", getLongStrFromTM(tmNext));
	nclog(L"\tinterval is: %id%02ih%02im\n", shDays, shHour, shMin);
#if DEBUG
	dumpTM(L"stNext", tmNext);
	dumpTM(L"stCurrentTime", g_tmCurrentStartTime);
#endif
	__time64_t ttNext = _mktime64(&tmNext);	// __time64_t: time as seconds elapsed since midnight, January 1, 1970, or -1 in the case of an error.
	__time64_t ttCurr = _mktime64(&g_tmCurrentStartTime);

	dumpTT64(L"current time: ", ttCurr);
	dumpTT64(L"next time:    ", ttNext);
	//double dTime = difftime(mktime(&tmNext), mktime(&g_tmCurrentStartTime)); //difftime(endingTime, startTime)
	
	if(ttNext <= ttCurr){//is the next schedule in past?
		if(shDays>0){
			//if we have a day interval we start with current day and add shDays
			int iHH=tmNext.tm_hour; 
			int iMM=tmNext.tm_min;
			tmNext=g_tmCurrentStartTime;
			//apply hour and min and then add hh and mm interval
			tmNext.tm_hour	=iHH + shHour;
			tmNext.tm_min	=iMM + shMin;
			tmNext.tm_mday	+=shDays;
			ttNext=_mktime64(&tmNext);
			while(ttNext<=ttCurr){
				tmNext.tm_hour+=shHour;
				tmNext.tm_min+=shMin;
				tmNext.tm_yday+=shDays;
				ttNext=_mktime64(&tmNext);
			}
		}
		else{
			do{
				//add interval to stNewTime
				//tmNext.tm_mday+=shDays;
				tmNext.tm_hour+=shHour;
				tmNext.tm_min+=shMin;
				ttNext=_mktime64(&tmNext);
				//ttNext = DT_Add(tmNext->tm_mday+shDays , 0, 0, shDays, shHour, shMin, 0, 0);// DT_AddDay(_Tasks[iTask].stStartTime);
			}while (ttNext <= ttCurr);
		}
	}//is the next schedule in past?
	else{//next schedule is in future!

		//check if scheduled time - timeInterval is in future
		//if yes, subtract timeInterval of scheduled time
		//repeat until scheduled time is in past of actual time
		if(shDays>0){
			//if we have a day interval we start with current day and add shDays
			int iHH=tmNext.tm_hour; 
			int iMM=tmNext.tm_min;
			tmNext=g_tmCurrentStartTime;
			//apply hour and min and then add hh and mm interval
			tmNext.tm_hour	=iHH + shHour;
			tmNext.tm_min	=iMM + shMin;
			tmNext.tm_mday	+=shDays;
			while(ttNext>ttCurr){
				tmNext.tm_hour-=shHour;
				tmNext.tm_min-=shMin;
				tmNext.tm_year-=shDays;
				ttNext=_mktime64(&tmNext);
			}
			//we are before current time, add one interval
			tmNext.tm_hour+=shHour;
			tmNext.tm_min+=shMin;
			tmNext.tm_year+=shDays;
			ttNext=_mktime64(&tmNext);

		}
		else{
			do{
				int iHH=tmNext.tm_hour; 
				int iMM=tmNext.tm_min;

				tmNext.tm_hour-=shHour;
				tmNext.tm_min-=shMin;
				ttNext=_mktime64(&tmNext);
			}while(ttNext > ttCurr);
			//now add one interval on top of the next schedule
			tmNext.tm_hour+=shHour;
			tmNext.tm_min+=shMin;
		}
	}
	ttNext=_mktime64(&tmNext);
	dumpTT64(L"\tschedule adjusted to ", ttNext);
	//else
	//	nclog(L"\tno schedule adjustement needed.\n");

	nclog(L"\tschedule adjusted to '%s'\n", getLongStrFromTM(tmNext));
	tmReturn=tmNext;
	return tmReturn;
}

