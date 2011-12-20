//regRW.h

#pragma once

#ifndef REGRW_H
	#define REGRW_H

	#include "stdafx.h"
#include <time.h>

	const int iMaxTasks = 10;
	extern int iTaskCount;

	extern TCHAR* szTaskerEXE;
	extern TCHAR* _szRegKey;
	extern TCHAR _szRegSubKeys[iMaxTasks][MAX_PATH];

	typedef struct TASK{
		TCHAR		szTaskKey[MAX_PATH];	//"Task1", "TaskX"
		TCHAR		szExeName[MAX_PATH];	//reg "exe"="\Windows\fexplore.exe"
		TCHAR		szArgs[MAX_PATH];		//reg "args"="\My Documents"
		struct tm	stStartTime;			//reg "start"="1423" = 14h 23m
		struct tm	stStopTime;				//reg "stop"="1523" = 15h 23m
		struct tm	stDiffTime;				//reg "interval"="2400" = 24h 00m
		//SYSTEMTIME	stStartTime;			//reg "start"="1423" = 14h 23m
		//SYSTEMTIME	stStopTime;				//reg "stop"="1523" = 15h 23m
		//SYSTEMTIME	stDiffTime;				//reg "interval"="2400" = 24h 00m
		int			iActive;				//reg "active":DWORD= 0=false, 1=true, other values indicate failure
		BOOL		bStartOnAConly;			//defaults to FALSE
	} taskStruct;

	extern DWORD _dwVersion;
	DWORD regReadDbgLevel();
	extern int _dbgLevel;

	int regReadKeys();
	int getTaskNumber(TCHAR* _sTask);
	int getSTfromString(SYSTEMTIME* sysTime /*in,out*/, TCHAR* sStr /*in*/);

	int getTMfromString(struct tm* tmTime /*in,out*/, TCHAR* sStr /*in*/);	
	int getStrFromTM(struct tm sysTime, TCHAR sStr[4+1]);

	//new with v2.1
	int getSTfromLongString(SYSTEMTIME * sysTime /*in,out*/, TCHAR* sStr /*in*/);
	int getLongStrFromSysTime(SYSTEMTIME sysTime, TCHAR* sStr);
	
	int getLongStrFromTM(struct tm tmTime, TCHAR* sStr);

	TCHAR* getLongStrFromSysTime2(SYSTEMTIME sysTime);
	TCHAR* getLongStrFromTM(struct tm tmTime);

	DWORD getVersion();
	int writeVersion(DWORD newVersion);
	int getMaxDelay();

	int regDisableTask(int iTask);
	int regEnableTask(int iTask);
	int regSetStartTime(int iTask, SYSTEMTIME pStartTime);
	int regSetStartTime(int iTask, struct tm pStartTime);
	int regSetStopTime(int iTask, SYSTEMTIME pStartTime);
	int regSetStopTime(int iTask, struct tm pStopTime);
//	void setUpdateAll();
//	void unsetUpdateAll();
//	BOOL getUpdateAll();
#endif