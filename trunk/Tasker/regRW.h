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
		TCHAR		szExeName[MAX_PATH];	//reg_sz "exe"="\Windows\fexplore.exe"
		TCHAR		szArgs[MAX_PATH];		//reg_sz "args"="\My Documents"
		struct tm	stStartTime;			//reg_sz "start"="1423" = 14h 23m
		struct tm	stStopTime;				//reg_sz "stop"="1523" = 15h 23m, or "none" to disable stopTask
		struct tm	stDiffTime;				//reg_sz "interval"="2400" = 24h 00m
		int			iActive;				//reg_dword "active":DWORD= 0=false, 1=true, other values indicate failure
		BOOL		bStartOnAConly;			//reg_dword defaults to FALSE
		BOOL		bStopActive;			//will be set, if reg_sz "stop" reads "none"
	} taskStruct;

	extern DWORD _dwVersion;
	DWORD regReadDbgLevel();
	extern int _dbgLevel;

	int regReadKeys();
	int getTaskNumber(TCHAR* _sTask);

	int getTMfromString(struct tm* tmTime /*in,out*/, TCHAR* sStr /*in*/);	
	int getStrFromTM(struct tm sysTime, TCHAR sStr[4+1]);

	int getLongStrFromTM(struct tm tmTime, TCHAR* sStr);

	TCHAR* getLongStrFromTM(struct tm tmTime);

	DWORD getVersion();
	int writeVersion(DWORD newVersion);
	int getMaxDelay();

	int regDisableTask(int iTask);
	int regEnableTask(int iTask);
	int regSetStartTime(int iTask, struct tm pStartTime);
	int regSetStopTime(int iTask, struct tm pStopTime);

#endif //REGRW_H