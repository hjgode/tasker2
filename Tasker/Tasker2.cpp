// Tasker2.cpp : Defines the entry point for the console application.
//
// use test mode to disable the creation of new schedules and control 
// everything with just calling Tasker2 with args manually

//#define TESTMODE

#pragma warning (once : 4244)

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

//--------------------------------------------------------------------
// Function name  : ClearAllSchedules
// Description    : remove all schedules to tasker2 from schedule db
// Return type    : int, number of schedules found and removed
//--------------------------------------------------------------------
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

//--------------------------------------------------------------------
// Function name  : scheduleAllTasks
// Description    : remove and re-add all schedules 
// Return type    : int, number of schedules added
//--------------------------------------------------------------------
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
			short shHourIntvl	=	_Tasks[iTask].stDiffTime.tm_hour ;
			short shMinIntvl		=	_Tasks[iTask].stDiffTime.tm_min;
			short shDaysIntvl	=   0;

			if(shHourIntvl>=24){	//hour interval value is one day or more
				shDaysIntvl = (short) (shHourIntvl / 24);
				shHourIntvl = (short) (shHourIntvl % 24);
			}

			struct tm tmNewTime;
			//read next start time (hour/minute)
			tmNewTime=_Tasks[iTask].stStartTime;

			//get the current time
			tmNewTime=g_tmCurrentStartTime;
			//set hour/minute to start/stop time
			tmNewTime.tm_hour = _Tasks[iTask].stStartTime.tm_hour ;// shHour;
			tmNewTime.tm_min  = _Tasks[iTask].stStartTime.tm_min;// shMin;

			tmNewTime = createNextSchedule(tmNewTime, shDaysIntvl, shHourIntvl, shMinIntvl);
#ifndef TESTMODE
			if(ScheduleRunApp(szTaskerEXE, strTaskCmdLine, tmNewTime)==0)
				iRet++;
#endif
			////save new start time
			regSetStartTime(iTask, tmNewTime);

			//################ create a new KILL schedule for taskX
			wsprintf(strTaskCmdLine, L"-k task%i", iTask+1); //the cmdLine for tasker.exe for this task	
			nclog(L"Creating new Kill Task schedule for '%s' in Task%i\n", _Tasks[iTask].szExeName, iTask+1);
			shHourIntvl	=	_Tasks[iTask].stDiffTime.tm_hour;
			shMinIntvl	=	_Tasks[iTask].stDiffTime.tm_min;
			shDaysIntvl	=	0;

			if(shHourIntvl>=24){	//hour interval value is one day or more
				shDaysIntvl = (short) (shHourIntvl / 24);
				shHourIntvl = (short) (shHourIntvl % 24);
			}				
			tmNewTime=g_tmCurrentStartTime;
			tmNewTime.tm_hour = _Tasks[iTask].stStopTime.tm_hour;
			tmNewTime.tm_min  = _Tasks[iTask].stStopTime.tm_min;

			tmNewTime = createNextSchedule(tmNewTime, shDaysIntvl, shHourIntvl, shMinIntvl);
#ifndef TESTMODE
			if(ScheduleRunApp(szTaskerEXE, strTaskCmdLine, tmNewTime)==0)
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
	////##################### use only one current start time #############################
	////v2.28: use only one current time, the time the exe has been started
	g_tmCurrentStartTime = getLocalTime(&g_tmCurrentStartTime);

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

	nclog(L"Date/Time after 11 2011. OK\n");

	writeVersion(_dwVersion);

	if(argc==1){ //equal to no arguments
		//just re-schedule all tasks
		//schedule all active tasks , //first clear all tasker.exe schedules and then re-add all tasks to schedule according to registry
		int iCount = scheduleAllTasks();
		nclog(L"Scheduled %i Tasks\n", iCount);		
	}

	//The system passes the APP_RUN_AT_TIME string or another single arg to the application as the command line.
	if(argc==2){
		DEBUGMSG(1, (L"one arg: '%s'\n", argv[1]));
		if(wcsicmp(argv[1], L"-c")==0){	
			//clear all task schedules
			int iCount = ClearAllSchedules(); 
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
				//now again check if we have a valid date
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

			nclog(L"\t Nothing currently implemented!\n");

			nclog(L"... test mode ended\n");
		}
	}
	
	if(argc==3){	// we got called by the scheduler with either "-s taskX" or "-k taskX", "-r taskX" and "-a taskX" can be used manually
		DEBUGMSG(1, (L"two args: '%s', '%s'\n", argv[1], argv[2]));

		if ( (wcsicmp(argv[1], L"-s")==0) || (wcsicmp(argv[1], L"-k")==0) ){	//start taskX app
			processStartStopCmd(argv);
		}
		else if(wcsicmp(argv[1], L"-r")==0){	//remove schedules for taskX
			int iTask = getTaskNumber(argv[2]);
			//create cmd line for tasker
			nclog(L"Clearing all Tasker schedules for Task%i\n", iTask+1);
			TCHAR strTaskCmdLine[MAX_PATH];

			wsprintf(strTaskCmdLine, L"-k task%i", iTask+1); //the KILL cmdLine for tasker.exe for this task	

			//clear all start tasker schedules for taskX
			nclog(L"Clearing all Start schedules for Task%i\n", iTask+1);
			notiClearRunApp(szTaskerEXE, strTaskCmdLine);

			wsprintf(strTaskCmdLine, L"-s task%i", iTask+1); //the Start cmdLine for tasker.exe for this task	

			//clear all stop tasker schedules for taskX
			nclog(L"Clearing all Kill schedules for Task%i\n", iTask+1);
			notiClearRunApp(szTaskerEXE, strTaskCmdLine);

			//set task to disabled
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
			shHour	=	_Tasks[iTask].stDiffTime.tm_hour;
			short shMin;
			shMin	=	_Tasks[iTask].stDiffTime.tm_min;
			short shDays	=	0;
			if(shHour>=24){	//hour interval value is one day or more
				shDays = (short) (shHour / 24);
				shHour = (short) (shHour % 24);
			}				

			struct tm tmNewTime;			
			tmNewTime = _Tasks[iTask].stStartTime;
			tmNewTime = createNextSchedule(tmNewTime, shDays, shHour, shMin);
			//stNewTime = DT_Add(_Tasks[iTask].stStartTime, 0, 0, 0, shHour, shMin, 0, 0);// DT_AddDay(_Tasks[iTask].stStartTime);
			
#ifndef TESTMODE
			ScheduleRunApp(szTaskerEXE, strTaskCmdLine, tmNewTime);
#endif			
			//the new kill task
			wsprintf(strTaskCmdLine, L"-k task%i", iTask+1); //the KILL cmdLine for tasker.exe for this task	

			nclog(L"Clearing all Kill schedules for Task%i\n", iTask+1);
			notiClearRunApp(szTaskerEXE, strTaskCmdLine);

			//create a new kill schedule with new time
			nclog(L"Creating new Kill schedule for '%s' in Task%i\n", _Tasks[iTask].szExeName, iTask+1);
			shHour	=	_Tasks[iTask].stDiffTime.tm_hour;
			shMin	=	_Tasks[iTask].stDiffTime.tm_min;
			shDays	=	0;
			if(shHour>=24){	//hour interval value is one day or more
				shDays = (short) (shHour / 24);
				shHour = (short) (shHour % 24);
			}			
			tmNewTime=_Tasks[iTask].stStopTime;
			tmNewTime=createNextSchedule(tmNewTime, shDays, shHour, shMin);
#ifndef TESTMODE
			ScheduleRunApp(szTaskerEXE, strTaskCmdLine, tmNewTime);
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
//--------------------------------------------------------------------
// Function name  : processStartStopCmd
// Description    : calc new start/stop time, add schedules and start/kill application
// Argument       : TCHAR* argv[], the cmdLine array
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
int processStartStopCmd(TCHAR* argv[]){
	int iReturn = 0;

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

		struct tm tmNewTime = {0};
		short shHour	=	_Tasks[iTask].stDiffTime.tm_hour;
		short shMin		=	_Tasks[iTask].stDiffTime.tm_min;
		short shDays	=	0;
		if(shHour>=24){	//hour interval value is one day or more
			shDays = (short) (shHour / 24);
			shHour = (short) (shHour % 24);
		}				

		//is this a delayed schedule?
		nclog(L"\tchecking for delayed schedule...\n");
		int iDeltaMinutes;
		__time64_t ttStart;
		__time64_t ttStop;
		__time64_t ttCurr;
		ttStop = _mktime64(&(_Tasks[iTask].stStopTime));
		ttStart = _mktime64(&(_Tasks[iTask].stStartTime));
		ttCurr = _mktime64(&g_tmCurrentStartTime);

		if(thisTaskType==stopTask){
			//difftime(timer1, timer0) returns the elapsed time in seconds, from timer0 to timer1
			iDeltaMinutes = difftime(ttCurr, ttStop)/60;// stDeltaMinutes(_Tasks[iTask].stStopTime, g_CurrentStartTime);
			nclog(L"stStopTime = '%s', ", getLongStrFromTM(_Tasks[iTask].stStopTime));
		}
		else{
			iDeltaMinutes = difftime(ttStart, ttCurr)/60;// stDeltaMinutes(_Tasks[iTask].stStartTime, g_CurrentStartTime);
			nclog(L"stStartTime = '%s', ", getLongStrFromTM(_Tasks[iTask].stStartTime));
		}

		nclog(L"current time = '%s'\n", getLongStrFromTM(g_tmCurrentStartTime));
		nclog(L"interval is: %id%02ih%02im\n", shDays, shHour, shMin);
		nclog(L"\tdelta is %i minutes\n", iDeltaMinutes);

		//started BEFORE scheduled time, iDelta is negative
		if(iDeltaMinutes<0){
			if(thisTaskType==stopTask){
				//calculate new schedules or leave them as is?
				//tmNewTime=_Tasks[iTask].stStopTime;	//leave them as is
				tmNewTime=createNextSchedule(_Tasks[iTask].stStopTime, shDays,shHour,shMin); //calc new schedule
			}
			else{
				//tmNewTime=_Tasks[iTask].stStartTime;	//leave them as is
				tmNewTime=createNextSchedule(_Tasks[iTask].stStartTime, shDays,shHour,shMin); //calc new schedule
			}
#ifndef TESTMODE
			ScheduleRunApp(szTaskerEXE, strTaskCmdLine, tmNewTime);
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
			//we calc the next schedule on base of the current time by using the saved start/stop time 
			//is just greater than the current time
			if(thisTaskType==stopTask)
				tmNewTime=_Tasks[iTask].stStopTime;
			else
				tmNewTime=_Tasks[iTask].stStartTime;

			tmNewTime = createNextSchedule(tmNewTime, shDays, shHour, shMin);
		}
		else{
			nclog(L"*** NO delayed schedule *** recognized\n");
			if(thisTaskType==stopTask){
				tmNewTime = createNextSchedule(_Tasks[iTask].stStopTime, shDays, shHour, shMin);
			}
			else{
				tmNewTime = createNextSchedule(_Tasks[iTask].stStartTime, shDays, shHour, shMin);
			}
			bIsDelayedSchedule=FALSE;
		}

		//create a new kill or start schedule with new time
		nclog(L"Creating new schedule for '%s' in Task%i\n", _Tasks[iTask].szExeName, iTask+1);
#ifndef TESTMODE
		ScheduleRunApp(szTaskerEXE, strTaskCmdLine, tmNewTime);
#endif
		//save new changed stop/start ime
		if(thisTaskType==stopTask)
			regSetStopTime(iTask, tmNewTime);
		else
			regSetStartTime(iTask, tmNewTime);

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
				nclog(L"Killing exe '%s'...\n", _Tasks[iTask].szExeName);
				//get only the process name without path
				TCHAR* strNew = wcsrchr(_Tasks[iTask].szExeName, L'\\');
				strNew++;
				nclog(L"\tTerminate process: '%s'\n", strNew);
				DWORD iKillRes = killExe( strNew ) ;//_Tasks[iTask].szExeName);
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

