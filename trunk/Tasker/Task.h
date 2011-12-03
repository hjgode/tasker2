
#pragma once

#ifndef CTASK_LOADED
#define CTASK_LOADED

class CTask
{
	typedef struct TASK{
		TCHAR		szTaskKey[MAX_PATH];	//"Task1", "TaskX"
		TCHAR		szExeName[MAX_PATH];	//reg "exe"="\Windows\fexplore.exe"
		TCHAR		szArgs[MAX_PATH];		//reg "args"="\My Documents"
		SYSTEMTIME	stStartTime;			//reg "start"="1423" = 14h 23m
		SYSTEMTIME	stStopTime;				//reg "stop"="1523" = 15h 23m
		SYSTEMTIME	stDiffTime;				//reg "period"="2400" = 24h 00m
		int			iActive;				//reg "active":DWORD= 0=false, 1=true, other values indicate failure
	} taskStruct;
public:
	TASK _task;
	CTask(void);
	~CTask(void);
	int getTaskNumber(TCHAR* _sTask);
};


#endif