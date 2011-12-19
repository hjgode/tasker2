// Tasker2.cpp : Defines the entry point for the console application.
//
// use test mode to disable the creation of new schedules and control 
// everything with just calling Tasker2 with args manually
#define TESTMODE

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
//__time64_t g_tmCurrentStartTime;
struct tm g_tmCurrentStartTime;

TCHAR* szTaskerEXE = L"\\Windows\\Tasker2.exe";

#define MY_MUTEX L"TASKER_RUNNING"
HANDLE hMutex=NULL;

//forward declaration
int processStartStopCmd(TCHAR* argv[]);

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
			//use the interval settings
			short shHourIntvl	=	_Tasks[iTask].stDiffTime.wHour;
			short shMinIntvl		=	_Tasks[iTask].stDiffTime.wMinute;
			short shDaysIntvl	=   0;

			SYSTEMTIME stNewTime;
			//SYSTEMTIME stCurrentTime;
			//GetLocalTime(&stCurrentTime); //v2.28

			if(shHourIntvl>=24){	//hour interval value is one day or more
				shDaysIntvl = (short) (shHourIntvl / 24);
				shHourIntvl = (short) (shHourIntvl % 24);
			}

			//read next start time (hour/minute)
			stNewTime=_Tasks[iTask].stStartTime;

			struct tm tmNewTime;
			//tmNewTime = convertSystemTime2TM(&tmNewTime, &stNewTime);
			////set next to current plus interval
			//__time64_t local_time;
			//_time64(&local_time); // get current time

			//get the current time
			//set hour/minute to start/stop time
			tmNewTime=g_tmCurrentStartTime;
			//tmNewTime.tm_mday+=shDays;
			tmNewTime.tm_hour = stNewTime.wHour;// shHour;
			tmNewTime.tm_min  = stNewTime.wMinute;// shMin;

			createNextSchedule(&tmNewTime, shDaysIntvl, shHourIntvl, shMinIntvl);
			//stNewTime = createNextSchedule(stNewTime, shDays, shHour, shMin);
//			stNewTime = convertTM2SYSTEMTIME(&stNewTime, &tmNewTime);
#ifndef TESTMODE
			if(ScheduleRunApp(szTaskerEXE, strTaskCmdLine, stNewTime)==0)
				iRet++;
#endif
			////save new start time
			regSetStartTime(iTask, stNewTime);

			//################ create a new KILL schedule for taskX
			wsprintf(strTaskCmdLine, L"-k task%i", iTask+1); //the cmdLine for tasker.exe for this task	
			nclog(L"Creating new Kill Task schedule for '%s' in Task%i\n", _Tasks[iTask].szExeName, iTask+1);
			shHourIntvl	=	_Tasks[iTask].stDiffTime.wHour;
			shMinIntvl	=	_Tasks[iTask].stDiffTime.wMinute;
			shDaysIntvl	=	0;

			if(shHourIntvl>=24){	//hour interval value is one day or more
				shDaysIntvl = (short) (shHourIntvl / 24);
				shHourIntvl = (short) (shHourIntvl % 24);
			}				
			stNewTime=_Tasks[iTask].stStopTime;
			tmNewTime=g_tmCurrentStartTime;
			//tmNewTime.tm_mday+=shDays;
			tmNewTime.tm_hour = stNewTime.wHour;
			tmNewTime.tm_min  = stNewTime.wMinute;

			createNextSchedule(&tmNewTime, shDaysIntvl, shHourIntvl, shMinIntvl);
			//stNewTime = createNextSchedule(stNewTime,shDays, shHour, shMin);
#ifndef TESTMODE
			if(ScheduleRunApp(szTaskerEXE, strTaskCmdLine, stNewTime)==0)
				iRet++;
#endif				
			////save new changed stoptime
			regSetStopTime(iTask, tmNewTime);
			
		}
	}
	nclog(L"scheduleAllTasks: scheduled %i new tasks\n", iRet);
	return iRet;
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

	// START ----------- store the start time in a global var
	//struct tm when;
	//__time64_t now;
	//_tzset();
 //   // Get UNIX-style time
	//_time64( &now );	//get the system time
	//_localtime64_s( &when, &now );	// convert to local time
	//g_tmCurrentStartTime = when;
	g_tmCurrentStartTime = getLocalTime(&g_tmCurrentStartTime);

	nclog(L"Using launch time %02i.%02i.%04i, %02i:%02i\n",
		g_tmCurrentStartTime.tm_mday, g_tmCurrentStartTime.tm_mon+1, g_tmCurrentStartTime.tm_year +1900,
		g_tmCurrentStartTime.tm_hour, g_tmCurrentStartTime.tm_min);
	
	//// using bad SYSTEMTIME
	//SYSTEMTIME stNow;
	//GetLocalTime(&stNow);
	////clean up
	//stNow.wSecond=0;
	//stNow.wMilliseconds=0;

	//nclog(L"Using launch time %02i.%02i.%04i, %02i:%02i\n",
	//	stNow.wDay, stNow.wMonth, stNow.wYear,
	//	stNow.wHour, stNow.wMinute);

	////##################### use only one current start time #############################
	////v2.28: use only one current time, the time the exe has been started
	//memcpy(&g_CurrentStartTime, &stNow, sizeof(SYSTEMTIME));
	// END ----------- store the start time in a global var

	nclog(L"CmdLine = \n");
	for(int x=1; x<argc; x++){
		nclog(L"\targv[%i]: '%s'\n", x, argv[x]);
	}

	// change global tasker2.exe path
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

	//check if date/time is valid (tm version)
	nclog(L"Checking for valid date/time...\n");
	if( ((g_tmCurrentStartTime.tm_year+1900)*100 + g_tmCurrentStartTime.tm_mon+1) < 201111){
		nclog(L"scheduling event notifications\n");
		//clear and renew all event notifications
#ifndef TESTMODE
		RunAppAtTimeChangeEvents(szTaskerEXE);
#endif
		nclog(L"Date/Time not valid!\n*********** END ************\n");
		return 0xBADDA1E;
	}

//	//check if date/time is valid (SYSTEMTIME version)
//	nclog(L"Checking for valid date/time...\n");
//	if( (stNow.wYear*100 + stNow.wMonth) < 201111){
//		nclog(L"scheduling event notifications\n");
//		//clear and renew all event notifications
//#ifndef TESTMODE
//		RunAppAtTimeChangeEvents(szTaskerEXE);
//#endif
//		nclog(L"Date/Time not valid!\n*********** END ************\n");
//		return 0xBADDA1E;
//	}
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
			g_tmCurrentStartTime=getLocalTime(&g_tmCurrentStartTime);
				//GetLocalTime(&stNow);
				////clean up
				//stNow.wSecond=0;
				//stNow.wMilliseconds=0;
				//memcpy(&g_CurrentStartTime, &stNow, sizeof(SYSTEMTIME));

			//now again check if we have a valid date

				//check if date/time is valid
				nclog(L"Checking for valid date/time...\n");
				if( ((g_tmCurrentStartTime.tm_year+1900)*100 + g_tmCurrentStartTime.tm_mon+1) < 201111){
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


			stNew = newSystemTime(stNew, L"201112131425");
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
			
			DEBUGMSG(1, (L"schedule is one day in past, interval is 1 day"));
			stSched = DT_Add(stActual,0,0,-2,0,0,0,0);	//minus two days
			stNew = getNextTime(stSched, stActual, 1, 0, 0);
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

