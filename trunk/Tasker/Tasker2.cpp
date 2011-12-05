// Tasker2.cpp : Defines the entry point for the console application.
//
// use test mode to disable the creation of new schedules and control 
// everything with just calling Tasker2 with args manually
#define TESTMODE

//if for emulator test, then undef INTERMEC
//#define INTERMEC

#include "stdafx.h"
#include "task.h"
#include "regRW.h"
#include <notify.h>
#include "myNotify.h"
#include "./common/nclog.h"

#ifdef INTERMEC
//#include "C:/Program Files (x86)/Intermec/Developer Library/Include/itc50.h"
//#pragma comment(lib, "C:/Program Files (x86)/Intermec/Developer Library/Lib/WCE600/WM6/Armv4i/itc50.lib")
#include "C:/Programme/Intermec/Developer Library/Include/itc50.h"
#pragma comment(lib, "C:/Programme/Intermec/Developer Library/Lib/WCE600/WM6/Armv4i/itc50.lib")
#else
#include <pm.h>
#endif

extern TASK _Tasks[iMaxTasks];
SYSTEMTIME g_CurrentStartTime;

TCHAR* szTaskerEXE = L"\\Windows\\Tasker2.exe";

#define MY_MUTEX L"TASKER_RUNNING"
HANDLE hMutex=NULL;

//forward declaration
int processStartStopCmd(TCHAR* argv[]);
SYSTEMTIME createNextSchedule(SYSTEMTIME stNext, short shDays, short shHour, short shMin);
SYSTEMTIME createDelayedNextSchedule(SYSTEMTIME stNext, short shDays, short shHour, short shMin);

void dumpST(TCHAR* szNote, SYSTEMTIME st);
SYSTEMTIME getNextTime(SYSTEMTIME stStart, SYSTEMTIME stBegin, int iIntervalDays, int iIntervalHours, int iIntervalMinutes);

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

int ClearAllSchedules(){
	//clear all event schedules
	int iRes=0;
#ifndef TESTMODE
	ClearRunAppAtTimeChangeEvents(szTaskerEXE);
	//clear all task schedules
	iRes = notiClearRunApp(szTaskerEXE);
#endif
	nclog(L"Cleared %i Tasker schedules\n", iRes);
	return iRes;
}

int scheduleAllTasks(){
	int iRet=0;
	//clear all task schedules
	iRet = ClearAllSchedules();
	nclog(L"scheduleAllTasks: ClearAllSchedules for %i tasks\n", iRet);
	
	//clear and renew all event notifications
#ifndef TESTMODE
	RunAppAtTimeChangeEvents(szTaskerEXE);
#endif
	iRet=0;

	//create new KILL and START schedules
	for(int i=0; i<iMaxTasks; i++)
	{
		int iTask = i;
		if(_Tasks[iTask].iActive==1){
			//################ create a new START schedule cmd line for tasker
			TCHAR strTaskCmdLine[MAX_PATH];

			//create a new START schedule with new time
			wsprintf(strTaskCmdLine, L"-s task%i", iTask+1); //the cmdLine for tasker.exe for this task	
			nclog(L"Creating new Start Task schedule for '%s' in Task%i\n", _Tasks[iTask].szExeName, iTask+1);
			short shHour	=	_Tasks[iTask].stDiffTime.wHour;
			short shMin		=	_Tasks[iTask].stDiffTime.wMinute;
			short shDays	=   0;

			SYSTEMTIME stNewTime;
			//SYSTEMTIME stCurrentTime;
			//GetLocalTime(&stCurrentTime); //v2.28

			stNewTime=_Tasks[iTask].stStartTime;
			if(shHour>=24){	//hour interval value is one day or more
				shDays = (short) (shHour / 24);
				shHour = (short) (shHour % 24);
			}				

			stNewTime = createNextSchedule(stNewTime, shDays, shHour, shMin);
#ifndef TESTMODE
			if(ScheduleRunApp(szTaskerEXE, strTaskCmdLine, stNewTime)==0)
				iRet++;
#endif
			////save new start time
			regSetStartTime(iTask, stNewTime);

			//################ create a new KILL schedule for taskX
			wsprintf(strTaskCmdLine, L"-k task%i", iTask+1); //the cmdLine for tasker.exe for this task	
			nclog(L"Creating new Kill Task schedule for '%s' in Task%i\n", _Tasks[iTask].szExeName, iTask+1);
			shHour	=	_Tasks[iTask].stDiffTime.wHour;
			shMin	=	_Tasks[iTask].stDiffTime.wMinute;
			shDays	=	0;

			stNewTime=_Tasks[iTask].stStopTime;
			if(shHour>=24){	//hour interval value is one day or more
				shDays = (short) (shHour / 24);
				shHour = (short) (shHour % 24);
			}				

			stNewTime = createNextSchedule(stNewTime,shDays, shHour, shMin);
#ifndef TESTMODE
			if(ScheduleRunApp(szTaskerEXE, strTaskCmdLine, stNewTime)==0)
				iRet++;
#endif				
			////save new changed stoptime
			regSetStopTime(iTask, stNewTime);

		}
	}
	nclog(L"scheduleAllTasks: scheduled %i new tasks\n", iRet);
	return iRet;
}

/*
update all task entries to the new NextStart and NextStop reg entries
*/
int updateAllTasksV210(){
	int iRet=0;
	////clear all task schedules
	//iRet = ClearAllSchedules();
	nclog(L"+++ Updating to v2.1 - for %i tasks...\n", iTaskCount);
	
	iRet=0;
	int iCount=0;

	//create new KILL and START schedules
	for(int i=0; i < iTaskCount; i++)
	{
		int iTask = i;
		iCount++;
		if(_Tasks[iTask].iActive==1){
			SYSTEMTIME stNewTime;
			stNewTime = _Tasks[iTask].stStartTime;	
			TCHAR szTime[MAX_PATH];
			if(getLongStrFromSysTime(stNewTime, szTime)==0)
				nclog(L"\tstStartTime=%s\n", szTime);
			////save new start time
			if(regSetStartTime(iTask, stNewTime)==10)
				nclog(L"NextStart updated for task %i\n");
			else{
				iCount--;
				nclog(L"NextStart Update failed for task %i\n", i+1);
			}

			stNewTime = _Tasks[iTask].stStopTime;	
			if(getLongStrFromSysTime(stNewTime, szTime)==0)
				nclog(L"\tstStartTime=%s\n", szTime);
			////save new changed stoptime
			if(regSetStopTime(iTask, stNewTime)==10)
				nclog(L"NextStop updated for task %i\n");
			else{
				iCount--;
				nclog(L"NextStop Update failed for task %i\n", i+1);
			}
		}
	}
	nclog(L"--- Updating to v2.1 - DONE\n", iRet);
	return iTaskCount - iCount; //if any error, this will be negative, else it will be zero
}


int _tmain(int argc, _TCHAR* argv[])
{
	DEBUGMSG(1, (L"here we are: '%s'\n", argv[0]));

	nclog(L"++++++++++++++++ Tasker v%i started +++++++++++++++++++\n", _dwVersion);
#ifdef DEBUG
	nclog(L"             DEBUG BUILD !\n");
#endif
#ifdef TESTMODE
	nclog(L"             TESTMODE !\n");
#endif
	//##################### dont run if already running #############################
	nclog(L"Checking for Mutex (single instance allowed only)...\n");

	hMutex=CreateMutex(NULL, TRUE, MY_MUTEX);
	if(hMutex==NULL){
		//this should never happen
		nclog(L"Error in CreateMutex! GetLastError()=%i\n", GetLastError());
		nclog(L"-------- END -------\n");
		return -99;
	}
	DWORD dwLast = GetLastError();
	if(dwLast== ERROR_ALREADY_EXISTS){//mutex already exists, wait for mutex release
		nclog(L"\tAttached to existing mutex\n");
		nclog(L"................ Waiting for mutex release......\n");
		WaitForSingleObject( hMutex, INFINITE );
		nclog(L"++++++++++++++++ Mutex released. +++++++++++++++\n");
	}
	else{
		nclog(L"\tCreated new mutex\n");
	}

	//##################### dont run if already running #############################
	
	BOOL bIsDelayedSchedule = TRUE; //used to save a delayed schedule situation

	SYSTEMTIME stNow;
	GetLocalTime(&stNow);
	//clean up
	stNow.wSecond=0;
	stNow.wMilliseconds=0;

	nclog(L"Using launch time %02i.%02i.%04i, %02i:%02i\n",
		stNow.wDay, stNow.wMonth, stNow.wYear,
		stNow.wHour, stNow.wMinute);

	nclog(L"CmdLine = \n");
	for(int x=1; x<argc; x++){
		nclog(L"\targv[%i]: '%s'\n", x, argv[x]);
	}

	//##################### use only one current start time #############################
	//v2.28: use only one current time, the time the exe has been started
	memcpy(&g_CurrentStartTime, &stNow, sizeof(SYSTEMTIME));

		//change tasker2.exe path
	// Get name of executable
	TCHAR lpFileName[MAX_PATH+1];
	GetModuleFileName(NULL, lpFileName, MAX_PATH); //lpFileName will contain the exe name of this running app!
	//apply current exe name to szExe for reference
	szTaskerEXE = new TCHAR[MAX_PATH+1];
	wsprintf(szTaskerEXE, lpFileName);

	//check if date/time is valid moved down in v 2.27

	//read all task entries from REG and version number
	int iRegRet = regReadKeys();
	if(iRegRet != 0)
	{
		nclog(L"######## ERRORS in REGISTRY (0x%0x) ############\n*********** END ************\n", iRegRet);
		return iRegRet;
	}

	//check if date/time is valid
	nclog(L"Checking for valid date/time...\n");
	if( (stNow.wYear*100 + stNow.wMonth) < 201111){
		nclog(L"scheduling event notifications\n");
		//clear and renew all event notifications
#ifndef TESTMODE
		RunAppAtTimeChangeEvents(szTaskerEXE);
#endif
		nclog(L"Date/Time not valid!\n*********** END ************\n");
		return 0xBADDA1E;
	}
	nclog(L"Date/Time after 11 2011. OK\n");

	//nclog(L"running version: %i\n", _dwVersion);
	//if(_dwVersion!=210L){
	//	nclog(L"Need to update to v2.10\n");
	//	if(updateAllTasksV210()==0){
	//		_dwVersion=220L;
	//	}
	//	writeVersion(_dwVersion);
	//}
	writeVersion(_dwVersion);

	//v2.28: we now use a global current time called g_CurrentStartTime
	//it is initialized with program start
	//SYSTEMTIME stCurrentTime;
	//GetLocalTime(&stCurrentTime); //store the current local time

	if(argc==1){ //equal to no arguments
		//just re-schedule all tasks
		//schedule all active tasks , //first clear all tasker.exe schedules and then re-add all tasks to schedule according to registry
		int iCount = scheduleAllTasks();
		nclog(L"Scheduled %i Tasks\n", iCount);		
	}

	//The system passes the APP_RUN_AT_TIME string to the application as the command line.
	if(argc==2){
		DEBUGMSG(1, (L"one arg: '%s'\n", argv[1]));
		if(wcsicmp(argv[1], L"-c")==0){	
			//clear all task schedules
			int iCount = ClearAllSchedules(); //ClearRunApp(szTaskerEXE);
			nclog(L"Cleared %i Tasker schedules\n", iCount);
		}
		else if(wcsicmp(argv[1], APP_RUN_AT_TIME)==0){
			//schedule all active tasks
			int iCount = scheduleAllTasks();
			nclog(L"Scheduled %i Tasks\n", iCount);
		}
		else if(wcsicmp(argv[1], APP_RUN_AFTER_TIME_CHANGE)==0 || wcsicmp(argv[1], APP_RUN_AFTER_TZ_CHANGE)==0){
			nclog(L"got '%s' signaled\n", argv[1]);

				//better to reread the current time we work with, possibly we have been blocked
				GetLocalTime(&stNow);
				//clean up
				stNow.wSecond=0;
				stNow.wMilliseconds=0;
				memcpy(&g_CurrentStartTime, &stNow, sizeof(SYSTEMTIME));

			//now again check if we have a valid date

				//check if date/time is valid
				nclog(L"Checking for valid date/time...\n");
				if( (stNow.wYear*100 + stNow.wMonth) < 201111){
					nclog(L"scheduling event notifications\n");
					//clear and renew all event notifications
#ifndef TESTMODE
					RunAppAtTimeChangeEvents(szTaskerEXE);
#endif
					nclog(L"Date/Time not valid!\n*********** END ************\n");
					return 0xBADCAB1E;
				}
				nclog(L"Date/Time after 11 2011. OK\n");

			//schedule all active tasks
			int iCount = scheduleAllTasks();
			nclog(L"Scheduled %i Tasks\n", iCount);
		}
		else if(wcsicmp(argv[1], L"-d")==0){	//dump list
			//dump all infos
			listNotifications();
		}
		else if(wcsicmp(argv[1], L"-t")==0){	//test mode
			//just sleep 15 seconds for testing mutex
			nclog(L"test mode...\n");
			SYSTEMTIME stSched, stActual, stNew;
			GetLocalTime(&stActual);
			memcpy(&stSched, &stActual, sizeof(SYSTEMTIME));

			DEBUGMSG(1, (L"schedule is one day in future"));
			stSched = DT_Add(stActual,0,0,1,0,0,0,0);	//add one day
			stNew = getNextTime(stSched, stActual, 0, 1, 0); //get next schedule for 1 hour interval
			dumpST(L"old sched   = ", stSched);
			dumpST(L"actual time = ", stActual);
			dumpST(L"new sched   = ", stNew);

			DEBUGMSG(1, (L"schedule is one day in past"));
			stSched = DT_Add(stActual,0,0,-2,0,0,0,0);	//minus two days
			stNew = getNextTime(stSched, stActual, 0, 1, 0);
			dumpST(L"old sched   = ", stSched);
			dumpST(L"actual time = ", stActual);
			dumpST(L"new sched   = ", stNew);
			
			nclog(L"... test mode ended\n");
		}
	}
	
	if(argc==3){
		DEBUGMSG(1, (L"two args: '%s', '%s'\n", argv[1], argv[2]));

		if ( (wcsicmp(argv[1], L"-s")==0) || (wcsicmp(argv[1], L"-k")==0) ){	//start taskX app
			processStartStopCmd(argv);
		}
		else if(wcsicmp(argv[1], L"-r")==0){	//remove schedules for taskX
			int iTask = getTaskNumber(argv[2]);
			//create cmd line for tasker
			//clear all tasker schedules for taskX
			nclog(L"Clearing all Tasker schedules for Task%i\n", iTask+1);
			TCHAR strTaskCmdLine[MAX_PATH];

			wsprintf(strTaskCmdLine, L"-k task%i", iTask+1); //the KILL cmdLine for tasker.exe for this task	

			nclog(L"Clearing all Start schedules for Task%i\n", iTask+1);
			notiClearRunApp(szTaskerEXE, strTaskCmdLine);

			wsprintf(strTaskCmdLine, L"-s task%i", iTask+1); //the Start cmdLine for tasker.exe for this task	

			nclog(L"Clearing all Kill schedules for Task%i\n", iTask+1);
			notiClearRunApp(szTaskerEXE, strTaskCmdLine);

			_Tasks[iTask].iActive=0;
			//write change to registry
			nclog(L"Setting Task%i to inactive\n", iTask+1);
			int iRes = regDisableTask(iTask);
			if(iRes==0)
				nclog(L"\tOK\n");
			else
				nclog(L"\tFAILED: %i\n", iRes);
		}
		else if(wcsicmp(argv[1], L"-a")==0){	//add schedules for taskX
			int iTask = getTaskNumber(argv[2]);
			//create cmd line for tasker
			//clear all tasker schedules for taskX
			nclog(L"Adding Tasker schedules for Task%i\n", iTask+1);
			TCHAR strTaskCmdLine[MAX_PATH];

			wsprintf(strTaskCmdLine, L"-s task%i", iTask+1); //the KILL cmdLine for tasker.exe for this task	

			nclog(L"Clearing all Start schedules for Task%i\n", iTask+1);
			notiClearRunApp(szTaskerEXE, strTaskCmdLine);

			//create a new schedule with new time
			nclog(L"Creating new Start schedule for '%s' in Task%i\n", _Tasks[iTask].szExeName, iTask+1);
			
			//the new start task
			short shHour;
			shHour	=	_Tasks[iTask].stDiffTime.wHour;
			short shMin;
			shMin	=	_Tasks[iTask].stDiffTime.wMinute;
			short shDays	=	0;
			if(shHour>=24){	//hour interval value is one day or more
				shDays = (short) (shHour / 24);
				shHour = (short) (shHour % 24);
			}				

			SYSTEMTIME stNewTime;
			
			stNewTime=_Tasks[iTask].stStartTime;
			stNewTime = createNextSchedule(stNewTime, shDays, shHour, shMin);
			//stNewTime = DT_Add(_Tasks[iTask].stStartTime, 0, 0, 0, shHour, shMin, 0, 0);// DT_AddDay(_Tasks[iTask].stStartTime);
			
#ifndef TESTMODE
			ScheduleRunApp(szTaskerEXE, strTaskCmdLine, stNewTime);
#endif			
			//the new kill task
			wsprintf(strTaskCmdLine, L"-k task%i", iTask+1); //the KILL cmdLine for tasker.exe for this task	

			nclog(L"Clearing all Kill schedules for Task%i\n", iTask+1);
			notiClearRunApp(szTaskerEXE, strTaskCmdLine);

				//create a new kill schedule with new time
				nclog(L"Creating new Kill schedule for '%s' in Task%i\n", _Tasks[iTask].szExeName, iTask+1);
				shHour	=	_Tasks[iTask].stDiffTime.wHour;
				shMin	=	_Tasks[iTask].stDiffTime.wMinute;
				shDays	=	0;
				if(shHour>=24){	//hour interval value is one day or more
					shDays = (short) (shHour / 24);
					shHour = (short) (shHour % 24);
				}			
				stNewTime=_Tasks[iTask].stStopTime;
				stNewTime=createNextSchedule(stNewTime, shDays, shHour, shMin);
				//stNewTime = DT_Add(_Tasks[iTask].stStopTime, 0, 0, 0, shHour, shMin, 0, 0);// DT_AddDay(_Tasks[iTask].stStartTime);
#ifndef TESTMODE
				ScheduleRunApp(szTaskerEXE, strTaskCmdLine, stNewTime);
#endif
			_Tasks[iTask].iActive=1;
			//write change to registry
			nclog(L"Setting Task%i to active\n", iTask+1);
			int iRes = regEnableTask(iTask);
			if(iRes==0)
				nclog(L"\tOK\n");
			else
				nclog(L"\tFAILED: %i\n", iRes);
		}
	}

	nclog(L"\tReleaseMutex...");
	ReleaseMutex(MY_MUTEX);
	if(CloseHandle(hMutex))
		nclog(L"OK\n");
	else{
		nclog(L"FALSE, lastError=%i\n", GetLastError());
	}
	nclog(L"++++++++++++++++++ Tasker2 ended +++++++++++++++++++\n");
	return 0;
}

/*	########################################################
		Process -s and -k comd line args
	########################################################
*/
int processStartStopCmd(TCHAR* argv[]){
	int iReturn = 0;

	//SYSTEMTIME stCurrentTime;
	//GetLocalTime(&stCurrentTime); //store the current local time
	BOOL bIsDelayedSchedule = TRUE; //used to save a delayed schedule situation

	enum taskType{
		startTask = 1,
		stopTask = 2
	};
	taskType thisTaskType;

	if(wcsicmp(argv[1], L"-k")==0)	//kill taskX app
		thisTaskType=stopTask;
	else
		thisTaskType=startTask;

	int iTask = getTaskNumber(argv[2]);
	if(_Tasks[iTask].iActive==1){
		//create a new schedule cmd line for tasker
		TCHAR strTaskCmdLine[MAX_PATH];

		if(thisTaskType==stopTask)
			wsprintf(strTaskCmdLine, L"-k task%i", iTask+1); //the cmdLine for tasker.exe for this task	
		else
			wsprintf(strTaskCmdLine, L"-s task%i", iTask+1); //the cmdLine for tasker.exe for this task	

		//clear all tasker schedules for taskX
		nclog(L"Clearing all schedules for Task%i with '%s'\n", iTask+1, strTaskCmdLine);
		notiClearRunApp(szTaskerEXE, strTaskCmdLine);

		SYSTEMTIME stNewTime = {0};
		short shHour	=	_Tasks[iTask].stDiffTime.wHour;
		short shMin		=	_Tasks[iTask].stDiffTime.wMinute;
		short shDays	=	0;
		if(shHour>=24){	//hour interval value is one day or more
			shDays = (short) (shHour / 24);
			shHour = (short) (shHour % 24);
		}				

		//is this a delayed schedule?
		nclog(L"\tchecking for delayed schedule...\n");
		int iDeltaMinutes;

		if(thisTaskType==stopTask){
			iDeltaMinutes = stDeltaMinutes(_Tasks[iTask].stStopTime, g_CurrentStartTime);
			nclog(L"stStopTime = '%s', ", getLongStrFromSysTime2(_Tasks[iTask].stStopTime));
		}
		else{
			iDeltaMinutes = stDeltaMinutes(_Tasks[iTask].stStartTime, g_CurrentStartTime);
			nclog(L"stStartTime = '%s', ", getLongStrFromSysTime2(_Tasks[iTask].stStartTime));
		}

		nclog(L"current time = '%s'\n", getLongStrFromSysTime2(g_CurrentStartTime));
		nclog(L"interval is: %id%02ih%02im\n", shDays, shHour, shMin);
		nclog(L"\tdelta is %i minutes\n", iDeltaMinutes);

		//started BEFORE scheduled time, iDelta is negative
		if(iDeltaMinutes<0){
			if(thisTaskType==stopTask)
				stNewTime=_Tasks[iTask].stStopTime;
			else
				stNewTime=_Tasks[iTask].stStartTime;
#ifndef TESTMODE
			ScheduleRunApp(szTaskerEXE, strTaskCmdLine, stNewTime);
#endif
			nclog(L"*** re-scheduled future task *** \n");
			return 0;
		}

		int iMaxDelay = getMaxDelay();
		nclog(L"\tmax allowed diff for delayed schedule recognition is plus %i\n", iMaxDelay);

		if( iDeltaMinutes > iMaxDelay ) //is the time diff greater than 1 minute
		{
			bIsDelayedSchedule = TRUE;
			nclog(L"*** delayed schedule *** recognized\n");
			//this is a delayed schedule
			DOUBLE dbTimeDiff = 0;

			//it may happen that the schedule is far in the future
			//we calc the next schedule on base of the current time by adding the interval to stStartTime/stStopTime until the hh:mm
			//is just greater than the current time
			if(thisTaskType==stopTask)
				stNewTime=_Tasks[iTask].stStopTime;
			else
				stNewTime=_Tasks[iTask].stStartTime;

			stNewTime = createDelayedNextSchedule(stNewTime, shDays, shHour, shMin);
			//if(!isNewer(stNewTime, stCurrentTime)){
			//	while (!isNewer(stNewTime, stCurrentTime)){
			//		//add interval to stNewTime
			//		stNewTime = DT_Add(stNewTime, 0, 0, shDays, shHour, shMin, 0, 0);// DT_AddDay(_Tasks[iTask].stStartTime);
			//	}
			//	if(getLongStrFromSysTime(stNewTime, szTime)==0)
			//		nclog(L"\tschedule adjusted to %s\n", szTime);
			//	else
			//		nclog(L"\t... schedule adjusted.\n");
			//}
		}
		else{
			nclog(L"*** NO delayed schedule *** recognized\n");
			if(thisTaskType==stopTask){
				stNewTime = createNextSchedule(_Tasks[iTask].stStopTime, shDays, shHour, shMin);
				//stNewTime = DT_Add(_Tasks[iTask].stStopTime, 0, 0, shDays, shHour, shMin, 0, 0);
			}
			else{
				stNewTime = createNextSchedule(_Tasks[iTask].stStartTime, shDays, shHour, shMin);
				//stNewTime = DT_Add(_Tasks[iTask].stStartTime, 0, 0, shDays, shHour, shMin, 0, 0);
			}
			bIsDelayedSchedule=FALSE;
		}

		//create a new kill or start schedule with new time
		nclog(L"Creating new schedule for '%s' in Task%i\n", _Tasks[iTask].szExeName, iTask+1);
#ifndef TESTMODE
		ScheduleRunApp(szTaskerEXE, strTaskCmdLine, stNewTime);
#endif
		//save new changed stop/start ime
		if(thisTaskType==stopTask)
			regSetStopTime(iTask, stNewTime);
		else
			regSetStartTime(iTask, stNewTime);

		if(!bIsDelayedSchedule){
			nclog(L"Not a delayed schedule\n");
			if(thisTaskType==startTask){
				if(_Tasks[iTask].bStartOnAConly){
					if(isACpowered()){
						nclog(L"Starting exe '%s' as on AC power\n", _Tasks[iTask].szExeName);
						runExe(_Tasks[iTask].szExeName, _Tasks[iTask].szArgs);
					}
					else{
						nclog(L"Skipping start of exe '%s' as not on AC power\n", _Tasks[iTask].szExeName);
					}
				}
				else{
					nclog(L"Starting exe '%s'\n", _Tasks[iTask].szExeName);
					runExe(_Tasks[iTask].szExeName, _Tasks[iTask].szArgs);
				}
			}
			else{ // a stop task			
				//now kill the task's exe				
				nclog(L"Killing exe '%s'\n", _Tasks[iTask].szExeName);
				DWORD iKillRes = killExe(_Tasks[iTask].szExeName);
				switch (iKillRes){
					case ERROR_NOT_FOUND:
						nclog(L"\texe not running\n");
						break;
					case 0:
						nclog(L"\texe killed\n");
						break;
					default:
						nclog(L"unable to kill exe. GetLastError=%08x\n", iKillRes);
						break;
				}
			}
		}
		else{
			nclog(L"Exec/Kill skipped as this is a delayed schedule\n");
		}
	}
	else
		nclog(L"\ttask %i is inactive (0x%02x)\n", iTask, _Tasks[iTask].iActive);

		return iReturn;
}

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
/*
	return time of next schedule in future that meets the given interval
*/
SYSTEMTIME createNextSchedule(SYSTEMTIME stNext, short shDays, short shHour, short shMin){
	
	//v2.30, always test for delayed schedule
	//v2.31 removed 	//return createDelayedNextSchedule(stNext, shDays, shHour, shMin);

	//SYSTEMTIME stCurrentTime;
	//GetLocalTime(&stCurrentTime);
	
	//cleanup, done now at startup
	//stCurrentTime.wSecond=0;
	//stCurrentTime.wMilliseconds=0;

	TCHAR szTime[24] = {0};

	if(shMin>=60){
		shHour += (short)(shMin / 60);
		shMin = (short)(shMin % 60);
	}
	if(shHour>=24){	//hour interval value is one day or more
		shDays = (short) (shHour / 24);
		shHour = (short) (shHour % 24);
	}

	nclog(L"\tcalculating new schedule for '%s'...\n", getLongStrFromSysTime2(stNext));
	nclog(L"\tinterval is: %id%02ih%02im\n", shDays, shHour, shMin);
#if DEBUG
	dumpST(L"stNext", stNext);
	dumpST(L"stCurrentTime", g_CurrentStartTime);
#endif
	if(!isNewer(stNext, g_CurrentStartTime)){
		do{
			//add interval to stNewTime
			stNext = DT_Add(stNext, 0, 0, shDays, shHour, shMin, 0, 0);// DT_AddDay(_Tasks[iTask].stStartTime);
		}while (!isNewer(stNext, g_CurrentStartTime));
	}
	nclog(L"\tschedule adjusted to '%s'\n", getLongStrFromSysTime2(stNext));
	//else
	//	nclog(L"\tno schedule adjustement needed.\n");

	return stNext;
}

/*
	return the next time (hhmm) that is past stBegin with a given interval
	subtract interval as long as stStart>stBegin and finally add one interval

	stStart is the current schedule time
	stBegin is the current time
*/
SYSTEMTIME getNextTime(SYSTEMTIME stStart, SYSTEMTIME stBegin, int iIntervalDays, int iIntervalHours, int iIntervalMinutes){
	SYSTEMTIME stStart1; 
	//memset(&stStart1, 0, sizeof(SYSTEMTIME));
	memcpy(&stStart1, &stStart, sizeof(SYSTEMTIME));
	//DEBUGMSG(1, (L"BEFORE: wDay=%02i, wHour=%02i, wMinute=%02i\n", stStart1.wDay, stStart1.wHour, stStart1.wMinute));
	dumpST(L"stStart1", stStart1);
	dumpST(L"stBegin", stBegin);

	int iCompareTimes = isNewer2(stStart1, stBegin);
	if(iCompareTimes==0) //both times are equal
	{
		//simply add one interval
		stStart1 = DT_Add(stStart1, 0, 0, iIntervalDays, iIntervalHours, iIntervalMinutes, 0, 0);
		////if stStart is in the future of stBegin?
		//do{
		//	//subtract interval from stStart until before stBegin
		//	stStart1 = DT_Add(stStart1, 0, 0, -iIntervalDays, -iIntervalHours, -iIntervalMinutes, 0, 0); 
		//	dumpST(L"stStart1", stStart1);
		//	dumpST(L"stBegin", stBegin);
		//}while (isNewer(stStart1, stBegin));// (uStart>uBegin);
		////add one interval
		//stStart1 = DT_Add(stStart1, 0, 0, iIntervalDays, iIntervalHours, iIntervalMinutes,0,0); 
	}
	else if(iCompareTimes==-1) //first time before second time, stBegin is current
	{
		//if stStart is before stBegin?
		// stStart = '20111122 1210', stBegin = '20111122 1411'
		do{
			//add interval from stStart until before stBegin
			stStart1 = DT_Add(stStart1, 0, 0, iIntervalDays, iIntervalHours, iIntervalMinutes,0,0); 
			dumpST(L"stStart1", stStart1);
			dumpST(L"stBegin", stBegin);
			iCompareTimes = isNewer2(stStart1, stBegin);
		}while (iCompareTimes==-1);// (uStart>uBegin);
	}
	else if(iCompareTimes==1) //first time is after second time, stBegin is current
	{
		//if stStart is after stBegin?
		// stStart = '20111122 1210', stBegin = '20111122 1411'
		do{
			//add interval from stStart until before stBegin
			stStart1 = DT_Add(stStart1, 0, 0, -iIntervalDays, -iIntervalHours, -iIntervalMinutes,0,0); 
			dumpST(L"stStart1", stStart1);
			dumpST(L"stBegin", stBegin);
			iCompareTimes = isNewer2(stStart1, stBegin);
		}while (iCompareTimes==1);// (uStart>uBegin);
	}
	dumpST(L"stStart1", stStart1);
	dumpST(L"stBegin", stBegin);

	//DEBUGMSG(1, (L"AFTER: wDay=%02i, wHour=%02i, wMinute=%02i\n", stStart1.wDay, stStart1.wHour, stStart1.wMinute));
	return stStart1;
}

/*
	return time of next schedule in future that meets the given interval
	stNext is the actual time of the schedule
	shDays, shHour and shMin define the interval for the schedule
*/
SYSTEMTIME createDelayedNextSchedule(SYSTEMTIME stNext, short shDays, short shHour, short shMin){
	nclog(L"+++ createDelayedNextSchedule: delayed schedule recalculation...\n");
	//SYSTEMTIME stCurrentTime;
	//GetLocalTime(&stCurrentTime);
	
	//cleanup, v2.28 now done in main
	//stCurrentTime.wSecond=0;
	//stCurrentTime.wMilliseconds=0;

	TCHAR szTime[24] = {0};

	if(shMin>=60){
		shHour += (short)(shMin / 60);
		shMin = (short)(shMin % 60);
	}
	if(shHour>=24){	//hour interval value is one day or more
		shDays = (short) (shHour / 24);
		shHour = (short) (shHour % 24);
	}

	nclog(L"\tcalculating new schedule for '%s'...\n", getLongStrFromSysTime2(stNext));
	nclog(L"\tinterval is: %id%02ih%02im\n", shDays, shHour, shMin);
#if DEBUG
	dumpST(L"stNext", stNext);
	dumpST(L"stCurrentTime", g_CurrentStartTime);
#endif
	//for delayed schedules 
	//example: stStopTime = '200902011205' and current time = '200902010000' and interval is: 0d00h10m
	//we need to get to stNext = '200902010005'
	SYSTEMTIME stNextNew = getNextTime(stNext, g_CurrentStartTime, shDays, shHour, shMin);
	stNext=stNextNew;
	nclog(L"\tschedule adjusted to '%s'\n", getLongStrFromSysTime2(stNext));
	//if(!isNewer(stNext, stCurrentTime)){
	//	do{
	//		//add interval to stNewTime
	//		stNext = DT_Add(stNext, 0, 0, shDays, shHour, shMin, 0, 0);// DT_AddDay(_Tasks[iTask].stStartTime);
	//	}while (!isNewer(stNext, stCurrentTime));
	//	nclog(L"\tschedule adjusted to '%s'\n", getLongStrFromSysTime2(stNext));
	//}
	//else
	//	nclog(L"\tno schedule adjustement needed.\n");

	nclog(L"--- createDelayedNextSchedule: delayed schedule recalculation finished.\n");
	return stNext;
}