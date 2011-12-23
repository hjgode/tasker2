//regRW.cpp

#include "stdafx.h"
#include "task.h"
#include "regRW.h"
#include "./common/nclog.h"

//CTask _ctasks[10];

extern struct tm g_tmCurrentStartTime;

TASK _Tasks[iMaxTasks];
int iTaskCount=0;

DWORD _dwVersion = 300L;
int _dbgLevel = 0;

static TCHAR* _szRegKey = L"Software\\tasker";
TCHAR _szRegSubKeys[10][MAX_PATH];

//--------------------------------------------------------------------
// Function name  : getTaskNumber
// Description    : extract the task number from reg based taskX string 
// Argument       : TCHAR* _sTask
// Return type    : the task number
//--------------------------------------------------------------------
int getTaskNumber(TCHAR* _sTask){
	TCHAR* sTemp = new TCHAR[wcslen(_sTask)+1];
	int iPos = wcslen(_sTask) - wcslen(L"Task");
	sTemp=&_sTask[wcslen(L"Task")];
	wcscat(sTemp, L"\0");
	int iRet = _wtoi(sTemp);
	return iRet-1;
}

///convert a HourMinute string (ie "1423) to a systemtime
//--------------------------------------------------------------------
// Function name  : getTMfromString
// Description    : convert a 'hhmm' string to a struct tm value
// Argument       : struct tm* tmTime /*in,out*/, the var to store the value
// Argument       : TCHAR* sStr, the string to convert 
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
int getTMfromString(struct tm* tmTime /*in,out*/, TCHAR* sStr /*in*/){

	if(_dbgLevel>4) 
		nclog(L"getTMfromString: ...\n");
	int iRet=-1;
	if(wcslen(sStr)!=4){
		if(_dbgLevel>4) nclog(L"getTMfromString: failure, string len not equal to 4\n");
		return -1;	//string to short
	}
	//extern SYSTEMTIME g_CurrentStartTime;
	memcpy(tmTime, &g_tmCurrentStartTime, sizeof(struct tm));
	//GetLocalTime(tmTime); //v2.28
	int iTime = _wtoi(sStr);
	//v2.30: removed testing if 0
	//if(iTime == 0)
	//	return -2;	//string not a number
	int iHour = iTime/100;
	int iMinute = iTime % 100;
	tmTime->tm_hour=iHour;
	tmTime->tm_min=iMinute;
	if(_dbgLevel>4) nclog(L"getTMfromString: return with '%s'\n", sStr);
	return 0;
}

//--------------------------------------------------------------------
// Function name  : getStrFromTM
// Description    : convert a struct tm value to a 'hhmm' string 
// Argument       : struct tm tmTime /*in*/, the var to convert
// Argument       : TCHAR* sStr, the string to return 
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
int getStrFromTM(struct tm sysTime, TCHAR sStr[4+1]){
	if(_dbgLevel>4) 
		nclog(L"getStrFromTM: ...\n");
	int iRet=-1;
	if(wcslen(sStr)!=4){
		if(_dbgLevel>4) nclog(L"getStrFromTM: failure, string len not equal to 4\n");
		return -1;	//string to short
	}
	TCHAR sTemp[4+1];
	wsprintf(sTemp, L"%02i%02i", sysTime.tm_hour, sysTime.tm_min);
	if(wcsncpy(sStr, sTemp, 4)==NULL){
		if(_dbgLevel>4) nclog(L"getStrFromTM: returning with error for '%s'\n", sTemp);
		return -1;
	}
	else{
		if(_dbgLevel>4) nclog(L"getStrFromTM: returning with '%s'\n", sStr);
		return 0; //no Error
	}
}

//--------------------------------------------------------------------
// Function name  : getLongStrFromTM
// Description    : convert a struct tm value to a 'YYYMMMDDhhmm' string 
// Argument       : struct tm tmTime /*in*/, the var to convert
// Argument       : TCHAR* sStr, the string to return 
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
int getLongStrFromTM(struct tm tmTime, TCHAR* sStr){
	int iRet=-1;
	wsprintf(sStr, L"000000000000");
	int iSize = wcslen(sStr);
	if(iSize != 12){
		if(_dbgLevel>4) nclog(L"getLongStrFromTM: str len not equal 12\n");
		return -1;	//string to short
	}
	TCHAR sTemp[12+1];
	wsprintf(sTemp, L"%04i%02i%02i%02i%02i", 
		(tmTime.tm_year)+1900 ,
		(tmTime.tm_mon)+1,
		tmTime.tm_mday,
		tmTime.tm_hour, 
		tmTime.tm_min);
	wsprintf(sStr, L"%s", sTemp);
	if(_dbgLevel>4) nclog(L"getLongStrFromTM: returning with '%s'\n", sStr);
	return 0; //no Error
}

//--------------------------------------------------------------------
// Function name  : getLongStrFromTM
// Description    : convert a struct tm value to a 'YYYMMMDDhhmm' string 
// Argument       : struct tm tmTime /*in*/, the var to convert
// Return type    : TCHAR*
//--------------------------------------------------------------------
TCHAR* getLongStrFromTM(struct tm tmTime){
		TCHAR* sTemp = new TCHAR[12+1];
		wsprintf(sTemp, L"%04i%02i%02i%02i%02i", 
			(tmTime.tm_year)+1900 ,
			(tmTime.tm_mon)+1,
			tmTime.tm_mday,
			tmTime.tm_hour, 
			tmTime.tm_min);
		wsprintf(sTemp, L"%s", sTemp);
		return sTemp; //no Error
}

//normalize a tm, ie, if hours is > 24
//--------------------------------------------------------------------
// Function name  : fixTM
// Description    : normalize a struct tm value, overflow mins and days 
// Argument       : struct tm tmIN, the time to fix
// Return type    : struct tm
//--------------------------------------------------------------------
struct tm fixTM(struct tm tmIn){
	TCHAR szTemp[13]; wsprintf(szTemp, L"000000000000");
	getLongStrFromTM(tmIn, szTemp);
	if(_dbgLevel>4) 
		nclog(L"fixTM: started with '%s'\n", szTemp);

	short shMin=tmIn.tm_min;
	//minutes above 60? add to hours
	if(shMin>=60){
		tmIn.tm_hour+=shMin/60;
		shMin=shMin%60;
	}
	short shHour = tmIn.tm_hour;
	//hours above 24? add to days
	if(shHour>24)
	{
		tmIn.tm_mday+=shHour/24;
		shHour=shHour%24;
	}

	struct tm tmReturn;
	memcpy(&tmReturn, &tmIn, sizeof(struct tm));

	tmReturn.tm_min=shMin;
	tmReturn.tm_hour=shHour;

	getLongStrFromTM(tmReturn, szTemp);
	if(_dbgLevel>4) nclog(L"fixTM: returning with '%s'\n", szTemp);
	return tmReturn;
}

//--------------------------------------------------------------------
// Function name  : regWriteDbgLevel
// Description    : write actual debug level to registry
// Argument       : DWORD dwDbgLevel
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
int regWriteDbgLevel(DWORD dwDbgLevel)
{
	int iRes=0;
	TCHAR subkey[MAX_PATH];
	HKEY hKey=NULL;
	DWORD dwVal=0;
	DWORD dwSize=sizeof(DWORD);
	DWORD dwType = REG_DWORD;

	//prepare subkey to write
	wsprintf(subkey, L"%s", _szRegKey);
	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, &hKey);
	if(rc != 0){
		nclog(L"Could not write %s\n", subkey);
		iRes=-1;
		goto exit_regWriteDbgLevel;
	}

	dwVal=dwDbgLevel;
	rc = RegSetValueEx(	hKey, L"dbglevel", NULL, dwType, (LPBYTE) &dwVal, dwSize); 

	if(rc == 0)
		DEBUGMSG(1, (L"regWriteDbgLevel: OK. Debug Level is %i\n", dwVal));
	else
		DEBUGMSG(1, (L"regWriteDbgLevel: FAILED %i\n", rc));

exit_regWriteDbgLevel:
	if(hKey!=NULL)
		RegFlushKey(hKey);
	RegCloseKey(hKey);
	return iRes;
}

//--------------------------------------------------------------------
// Function name  : regReadDbgLevel
// Description    : read desired debug level from registry
// Return type    : DWORD dwDbgLevel
//--------------------------------------------------------------------
DWORD regReadDbgLevel(){
	int iRes=0;
	DWORD dwVal=0, dwSize=sizeof(DWORD);
	DWORD dwType = REG_DWORD;

	TCHAR subkey[MAX_PATH];
	//prepare subkey to read
	HKEY hKey=NULL;
	wsprintf(subkey, L"%s", _szRegKey);
	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_QUERY_VALUE, &hKey);
	if (rc != ERROR_SUCCESS){
		nclog(L"regReadDbgLevel: FATAL cannot open key '%s': %u. Using default 0\n", subkey, rc);
		_dbgLevel=0;
		goto exit_regReadDbgLevel;
	}
	//read value
	rc = RegQueryValueEx(hKey, L"dbglevel", NULL, &dwType, (LPBYTE) &dwVal, &dwSize);

	if(rc == 0){
		DEBUGMSG(1, (L"regReadDbgLevel: OK. dbglevel is %u\n", dwVal));
		_dbgLevel=dwVal;
	}
	else{
		dwVal=1;
		DEBUGMSG(1, (L"regReadDbgLevel: FAILED %u\n", rc));
	}
exit_regReadDbgLevel:
	RegCloseKey(hKey);
	regWriteDbgLevel(_dbgLevel);
	return _dbgLevel;
}

//--------------------------------------------------------------------
// Function name  : regReadKeys
// Description    : read all registry entries into task var array
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
int regReadKeys(){
	int iRet = 0;
	DWORD dwDbgLevel=regReadDbgLevel();
#if DEBUG
	dwDbgLevel=5;
#endif
	TCHAR subkey[MAX_PATH];
	memset(&subkey, 0, sizeof(TCHAR)*MAX_PATH);
	wsprintf(subkey, L"%s", _szRegKey);

	HKEY hKey=NULL;
	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKey);
	if (rc != ERROR_SUCCESS){
		nclog(L"regReadKeys: FATAL cannot open key '%s': %u\n", subkey, rc);
		iRet=-1;
		goto exit_readallkeys;
	}

	int iCount = 0;// = regCountSubKeys();
	//+++ count number of subkeys
		TCHAR* lpName = new TCHAR(MAX_PATH);
		memset(lpName, 0, sizeof(TCHAR)*MAX_PATH);
		DWORD dwCount = MAX_PATH*sizeof(TCHAR);
		DWORD dwIdx=0;
		TCHAR szStr[MAX_PATH];
		while(RegEnumKeyEx(hKey, dwIdx, lpName, &dwCount, NULL, NULL, 0, NULL)==ERROR_SUCCESS){
			if(_dbgLevel>5){
				wcscpy(szStr, lpName);
				nclog(L"\tregReadKeys: found subkey '%s'\n", szStr);
			}
			memset(lpName, 0, sizeof(TCHAR)*MAX_PATH);
			dwCount=MAX_PATH; //reset string length var
			dwIdx++;
		}
	iCount=dwIdx;
	//--- count number of subkeys

	if(iCount==0)
	{
		if(_dbgLevel>4) 
			nclog(L"regReadKeys: No sub keys found\n");
		iRet=-2; //enum error
		goto exit_readallkeys;
	}
	if(_dbgLevel>4) 
		nclog(L"regReadKeys: Found %i tasker sub keys\n", iCount);

	//not more than 10 tasks allowed
	if(iCount>iMaxTasks)
		iCount=iMaxTasks;

	if(_dbgLevel>4) 
		nclog(L"regReadKeys: Allocating memory for %i tasks\n", iCount);
	for(int i=0; i<iMaxTasks; i++){
		memset(_szRegSubKeys[i],0,sizeof(TCHAR)*MAX_PATH);
	}
	iTaskCount=iCount;

	for(int i=0; i<iCount; i++){
		int iRes=0;
		wsprintf(_szRegSubKeys[i], L"Task%i", i+1);
		wsprintf(_Tasks[i].szTaskKey, L"Task%i", i+1);	//save TaskX entry to structure

		//prepare subkey to read
		wsprintf(subkey, L"%s\\%s", _szRegKey, _szRegSubKeys[i]);
		rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKey);
		if(rc!=0){
			nclog(L"regReadKeys: RegOpenKey failed for task%i at '%s' with %i...\n", i, subkey, rc);
			iRet=-3; //can not read subkey
			goto exit_readallkeys;
		}

		if(_dbgLevel>4) 
			nclog(L"regReadKeys: Processing task%i at '%s' ...\n", i, subkey);

		//now we can read the values in each task key
		DWORD dwVal=0;
		TCHAR szVal[MAX_PATH];
		DWORD dwSize=sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		rc=RegQueryValueEx(hKey, L"active", 0, &dwType, (LPBYTE) &dwVal, &dwSize);
		if(rc==0){
			if(_dbgLevel>4) 
				nclog(L"\tregReadKeys: 'active' entry is %i\n", dwVal);
			_Tasks[i].iActive=dwVal;
		}
		else
		{
			if(_dbgLevel>4) 
				nclog(L"\tregReadKeys: error in read 'active' entry, using 0 (inactive)\n");
			_Tasks[i].iActive=0;
		}

		dwSize=sizeof(TCHAR)*MAX_PATH;
		dwType=REG_SZ;
		rc=RegQueryValueEx(hKey, L"exe", 0, &dwType, (LPBYTE) &szVal, &dwSize);
		if(rc==0){
			if(_dbgLevel>4) 
				nclog(L"\tregReadKeys: 'exe' entry is '%s'\n", szVal);
			wsprintf(_Tasks[i].szExeName, szVal);
		}
		else{
			_Tasks[i].iActive = 0;
			if(_dbgLevel>4) 
				nclog(L"\tregReadKeys: error in read 'exe' entry for '%s'!\n", subkey);
			iRet=-4; //can not read exe entry
			goto exit_readallkeys;
		}

		dwSize=sizeof(TCHAR)*MAX_PATH;
		dwType=REG_SZ;
		rc=RegQueryValueEx(hKey, L"args", 0, &dwType, (LPBYTE) &szVal, &dwSize);
		if(rc==0){
			if(_dbgLevel>4) 
				nclog(L"\tregReadKeys: 'args' entry is '%s'\n", szVal);
			wsprintf(_Tasks[i].szArgs, szVal);
		}
		else{
			if(_dbgLevel>4) nclog(L"\tregReadKeys: error in read 'args' entry!\n");
			wsprintf(_Tasks[i].szArgs, L"");
		}
//####### read times
		struct tm tmTemp;
		//change for v2.2: always only read start/stop values and use NextStart and NextStop only as info
		dwSize=sizeof(TCHAR)*MAX_PATH;
		dwType=REG_SZ;
		rc=RegQueryValueEx(hKey, L"start", 0, &dwType, (LPBYTE) &szVal, &dwSize);
		if(rc==0){
			if(_dbgLevel>4) 
				nclog(L"\tregReadKeys: 'start' entry is '%s'\n", szVal);
			iRes=getTMfromString(&tmTemp, szVal);
			if(iRes==0){
				_Tasks[i].stStartTime=fixTM(tmTemp);
				if(_dbgLevel>4) 
					nclog(L"\tregReadKeys: task.stStartTime entry set\n");
			}
			else{
				if(_dbgLevel>4) 
					nclog(L"\tregReadKeys: task.stStartTime could not be set, iRes=%i\n", iRes);
				_Tasks[i].iActive = 0;
				iRet=-6; //can not read exe entry
				goto exit_readallkeys;
			}
		}
		else
		{
			if(_dbgLevel>4) 
				nclog(L"\tregReadKeys: error in read 'start' entry\n");
			iRet=-5; //can not read exe entry
			goto exit_readallkeys;
		}

		//change for v2.2: always only read start/stop values and use NextStart and NextStop only as info
		dwSize=sizeof(TCHAR)*MAX_PATH;
		dwType=REG_SZ;
		rc=RegQueryValueEx(hKey, L"stop", 0, &dwType, (LPBYTE) &szVal, &dwSize);
		if(rc==0){
			if(_dbgLevel>4) nclog(L"\tregReadKeys: 'stop' entry is '%s'\n", szVal);
			iRes=getTMfromString(&tmTemp, szVal);
			if(iRes==0){
				if(_dbgLevel>4) 
					nclog(L"\tregReadKeys: task.stStopTime entry set\n");
				_Tasks[i].stStopTime=fixTM(tmTemp);
			}
			else{
				_Tasks[i].iActive = 0;
				if(_dbgLevel>4) 
					nclog(L"\tregReadKeys: task.stStopTime could not be set, iRes=%i\n", iRes);
				iRet=-8; //can not read exe entry
				goto exit_readallkeys;
			}
		}
		else
		{
			if(_dbgLevel>4) 
				nclog(L"\tregReadKeys: error in read 'stop' entry\n");
			iRet=-7; //can not read exe entry
			goto exit_readallkeys;
		}

		dwSize=sizeof(TCHAR)*MAX_PATH;
		dwType=REG_SZ;
		rc=RegQueryValueEx(hKey, L"interval", 0, &dwType, (LPBYTE) &szVal, &dwSize);
		if(rc==0){
			if(_dbgLevel>4) 
				nclog(L"\tregReadKeys: 'interval' entry is '%s'\n", szVal);
			iRes=getTMfromString(&tmTemp, szVal);
			if(iRes==0){
				if(_dbgLevel>4) 
					nclog(L"\tregReadKeys: 'interval' using %02i:%02i\n", tmTemp.tm_hour, tmTemp.tm_min);
				if(tmTemp.tm_hour==0 && tmTemp.tm_min==0 && tmTemp.tm_mday==0){ //interval 000000 not supported
					_Tasks[i].iActive = 0;
					nclog(L"interval = 0 is NOT supported\n");
					iRet=-99; //can not read exe entry
					goto exit_readallkeys;
				}
				_Tasks[i].stDiffTime=tmTemp;
			}
			else{
				if(_dbgLevel>4) nclog(L"\tregReadKeys: error in read 'interval' entry. Using Active=FALSE\n");
				_Tasks[i].iActive = 0;
				iRet=-10; //can not read exe entry
				goto exit_readallkeys;
			}
		}
		else
		{
			if(_dbgLevel>4) 
				nclog(L"\tregReadKeys: error in read 'interval' entry\n");
			iRet=-9; //can not read interval entry
			goto exit_readallkeys;
		}

		//read startOnAConly
		dwSize = sizeof(DWORD);
		dwType = REG_DWORD;
		rc=RegQueryValueEx(hKey, L"startOnAConly", 0, &dwType, (LPBYTE) &dwVal, &dwSize);
		if(rc==0){
			if(_dbgLevel>4) 
				nclog(L"\tregReadKeys: 'startOnAConly' entry is %i\n", dwVal);
			if(dwVal==1)
				_Tasks[i].bStartOnAConly=TRUE;
			else
				_Tasks[i].bStartOnAConly=FALSE;
		}
		else
		{
			if(_dbgLevel>4) 
				nclog(L"\tregReadKeys: error in read 'startOnAConly' entry. Using FALSE\n");
			_Tasks[i].bStartOnAConly=FALSE;
		}
	}//for ... next

exit_readallkeys:
	RegCloseKey(hKey);
	return iRet;
}

//--------------------------------------------------------------------
// Function name  : regDisableTask
// Description    : disable a task by writing 0 to reg for 'active' 
// Argument       : int iTask, the task to disable via the registry
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
int regDisableTask(int iTask){
	int iRes=0;
	TCHAR subkey[MAX_PATH];
	HKEY hKey=NULL;
	DWORD dwVal=0;
	DWORD dwSize=sizeof(DWORD);
	DWORD dwType = REG_DWORD;
	//set inactive
	_Tasks[iTask].iActive=0;
	//prepare subkey to read
	wsprintf(subkey, L"%s\\%s", _szRegKey, _szRegSubKeys[iTask]);

	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, &hKey);
	if(rc != 0){
		nclog(L"Could not open %s\n", subkey);
		iRes=-1;
		goto exit_regDisableTask;
	}

	rc = RegSetValueEx(	hKey, L"active", NULL, dwType, (LPBYTE) &dwVal, dwSize); 

	if(rc == 0)
		DEBUGMSG(1,(L"changed reg for task%i to inactive\n", iTask+1));
	else{
		DEBUGMSG(1, (L"changing reg for task%i to inactive Failed: %i\n", iTask+1, rc));
		iRes=-2;
	}

exit_regDisableTask:
	if(hKey!=NULL)
		RegFlushKey(hKey);
	RegCloseKey(hKey);
	return iRes;
}

//--------------------------------------------------------------------
// Function name  : regEnableTask
// Description    : enable a task by writing 1 to reg for 'active' 
// Argument       : int iTask, the task to enable via the registry
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
int regEnableTask(int iTask){
	int iRes=0;
	TCHAR subkey[MAX_PATH];
	HKEY hKey=NULL;
	DWORD dwVal=1;
	DWORD dwSize=sizeof(DWORD);
	DWORD dwType = REG_DWORD;
	//set inactive
	_Tasks[iTask].iActive=0;
	//prepare subkey to read
	wsprintf(subkey, L"%s\\%s", _szRegKey, _szRegSubKeys[iTask]);

	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, &hKey);
	if(rc != 0){
		nclog(L"Could not open %s\n", subkey);
		iRes=-1;
		goto exit_regEnableTask;
	}

	rc = RegSetValueEx(	hKey, L"active", NULL, dwType, (LPBYTE) &dwVal, dwSize); 

	if(rc == 0)
		DEBUGMSG(1,(L"changed reg for task%i to active\n", iTask+1));
	else{
		DEBUGMSG(1, (L"changing reg for task%i to active Failed: %i\n", iTask+1, rc));
		iRes=-2;
	}

exit_regEnableTask:
	if(hKey!=NULL)
		RegFlushKey(hKey);
	RegCloseKey(hKey);
	return iRes;
}

//--------------------------------------------------------------------
// Function name  : regSetStartTime
// Description    : write next start time of a task to registry
// Argument       : int iTask, the task to set
// Argument       : struct tm pStartTime, the next time to start
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
int regSetStartTime(int iTask, struct tm pStartTime){
	TCHAR subkey[MAX_PATH];
	TCHAR szVal[MAX_PATH];
	DWORD dwSize=0;
	DWORD dwType=REG_SZ;
	HKEY hKey=NULL;

	//convert systemtime to HHmm
	TCHAR* szHM = new TCHAR[4+1];
	szHM = (TCHAR*)memset(szHM, 0, 4+1);
	wsprintf(szHM, L"0000");
	int iRet=0;

	if((iRet=getStrFromTM(pStartTime, szHM))!=0)
		goto exit_regSetStartTime2;

	//prepare subkey to read
	wsprintf(subkey, L"%s\\%s", _szRegKey, _szRegSubKeys[iTask]);
	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, &hKey);
	if (rc != ERROR_SUCCESS){
		nclog(L"regSetStartTime: FATAL cannot open key '%s': %u\n", subkey, rc);
		iRet=-1;
		goto exit_regSetStartTime2;
	}
	//change for v2.2: always only read start/start values and use NextStart and NextStart only as info
	wsprintf(szVal, L"%s", szHM);
	dwSize=wcslen(szHM)*sizeof(TCHAR);
	dwType=REG_SZ;

	rc = RegSetValueEx(hKey, L"start", 0, dwType, (LPBYTE)szVal, dwSize);
	if(rc == 0)
		DEBUGMSG(1, (L"regSetStartTime: OK. START is %s\n", szVal));
	else{
		DEBUGMSG(1, (L"regSetStartTime: FAILED %i\n", rc));
	}

	TCHAR sNextStart[13]; 
	wsprintf(sNextStart, L"000000000000");
	wsprintf(sNextStart, L"%s", getLongStrFromTM(pStartTime));
	if(wcslen(sNextStart) > 0){
		wsprintf(szVal, L"%s", sNextStart);
		dwSize=sizeof(TCHAR)*wcslen(szVal);
		rc = RegSetValueEx(hKey, L"NextStart", 0, dwType, (LPBYTE)szVal, dwSize);
		if(rc == 0){
			DEBUGMSG(1,(L"regSetStartTime: changed NextStart reg for task%i to '%s'\n", iTask+1, sNextStart));
			iRet=10;
		}
		else
			DEBUGMSG(1, (L"regSetStartTime: changing start reg for task%i to '%s' Failed: %i\n", iTask+1, szHM, rc));
	}
	else{
		DEBUGMSG(1, (L"regSetStartTime: -getLongStrFromSysTime() failed\n"));
	}

exit_regSetStartTime2:
	if(hKey!=NULL)
		RegFlushKey(hKey);
	RegCloseKey(hKey);
	delete(szHM);
	return iRet;
}

//--------------------------------------------------------------------
// Function name  : regSetStopTime
// Description    : write next stop time of a task to registry
// Argument       : int iTask, the task to set
// Argument       : struct tm pStartTime, the next time to start
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
int regSetStopTime(int iTask, struct tm pStopTime){
	TCHAR subkey[MAX_PATH];
	TCHAR szVal[MAX_PATH];
	DWORD dwSize=0;
	DWORD dwType=REG_SZ;
	HKEY hKey=NULL;

	//convert systemtime to HHmm
	TCHAR* szHM = new TCHAR[4+1];
	szHM = (TCHAR*)memset(szHM, 0, 4+1);
	wsprintf(szHM, L"0000");
	int iRet=0;

	if((iRet=getStrFromTM(pStopTime, szHM))!=0)
		goto exit_regSetStopTime2;

	//prepare subkey to read
	wsprintf(subkey, L"%s\\%s", _szRegKey, _szRegSubKeys[iTask]);
	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, &hKey);
	if (rc != ERROR_SUCCESS){
		nclog(L"regSetStopTime: FATAL cannot open key '%s': %u\n", subkey, rc);
		iRet=-1;
		goto exit_regSetStopTime2;
	}
	//change for v2.2: always only read start/start values and use NextStop and NextStop only as info
	wsprintf(szVal, L"%s", szHM);
	dwSize=wcslen(szHM)*sizeof(TCHAR);
	dwType=REG_SZ;

	rc = RegSetValueEx(hKey, L"start", 0, dwType, (LPBYTE)szVal, dwSize);
	if(rc == 0)
		DEBUGMSG(1, (L"regSetStopTime: OK. START is %s\n", szVal));
	else{
		DEBUGMSG(1, (L"regSetStopTime: FAILED %i\n", rc));
	}

	TCHAR sNextStop[13]; 
	wsprintf(sNextStop, L"000000000000");
	wsprintf(sNextStop, L"%s", getLongStrFromTM(pStopTime));
	if(wcslen(sNextStop) > 0){
		wsprintf(szVal, L"%s", sNextStop);
		dwSize=sizeof(TCHAR)*wcslen(szVal);
		rc = RegSetValueEx(hKey, L"NextStop", 0, dwType, (LPBYTE)szVal, dwSize);
		if(rc == 0){
			DEBUGMSG(1,(L"regSetStopTime: changed NextStop reg for task%i to '%s'\n", iTask+1, sNextStop));
			iRet=10;
		}
		else
			DEBUGMSG(1, (L"regSetStopTime: changing start reg for task%i to '%s' Failed: %i\n", iTask+1, szHM, rc));
	}
	else{
		DEBUGMSG(1, (L"regSetStopTime: -getLongStrFromSysTime() failed\n"));
	}

exit_regSetStopTime2:
	if(hKey!=NULL)
		RegFlushKey(hKey);
	RegCloseKey(hKey);
	delete(szHM);
	return iRet;

}

//--------------------------------------------------------------------
// OBSOLETE
// Function name  : getVersion
// Description    : read version from registry
// Return type    : DWORD with version number, default 200
//--------------------------------------------------------------------
DWORD getVersion()
{
	int iRes=0;
	DWORD dwVal=0, dwSize=sizeof(DWORD);
	DWORD dwType = REG_DWORD;
	DWORD dbgLevel=0;
	TCHAR subkey[MAX_PATH];
	//prepare subkey to read
	HKEY hKey=NULL;
	wsprintf(subkey, L"%s", _szRegKey);
	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_QUERY_VALUE, &hKey);
	if (rc != ERROR_SUCCESS){
		nclog(L"getVersion: FATAL cannot open key '%s': %u. Using default 0\n", subkey, rc);
		dbgLevel=0;
		goto exit_getVersion;
	}
	//read value
	rc = RegQueryValueEx(hKey, L"Version", NULL, &dwType, (LPBYTE) &dwVal, &dwSize);

	if(rc == 0){
		DEBUGMSG(1, (L"getVersion: OK. Version is %u\n", dwVal));
		dbgLevel=dwVal;
	}
	else{
		dwVal=200L;
		DEBUGMSG(1, (L"getVersion: FAILED %u\n", rc));
	}
exit_getVersion:
	RegCloseKey(hKey);
	return dwVal;
}

//--------------------------------------------------------------------
// FOR TEST USE ONLY
// Function name  : writeMaxDelay
// Description    : write actual maxDelay to registry
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
int writeMaxDelay(UINT uDelay)
{
	int iRes=0;
	TCHAR subkey[MAX_PATH];
	HKEY hKey=NULL;
	DWORD dwVal=0;
	DWORD dwSize=sizeof(DWORD);
	DWORD dwType = REG_DWORD;

	//prepare subkey to write
	wsprintf(subkey, L"%s", _szRegKey);
	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, &hKey);
	if(rc != 0){
		nclog(L"Could not write %s\n", subkey);
		iRes=-1;
		goto exit_writeMaxDelay;
	}

	dwVal=uDelay;
	rc = RegSetValueEx(	hKey, L"maxDelay", NULL, dwType, (LPBYTE) &dwVal, dwSize); 

	if(rc == 0)
		DEBUGMSG(1, (L"writeMaxDelay: OK. maxDelay is %i\n", dwVal));
	else{
		DEBUGMSG(1, (L"writeMaxDelay: FAILED %i\n", rc));
		iRes=-2;
	}

exit_writeMaxDelay:
	if(hKey!=NULL)
		RegFlushKey(hKey);
	RegCloseKey(hKey);
	return iRes;
}

/*
	read max allowed DeltaTime for ecognizing a delayed schedule
*/
//--------------------------------------------------------------------
// Function name  : getMaxDelay
// Description    : get maxDelay value from registry
// Return type    : int, 1 default
//--------------------------------------------------------------------
int getMaxDelay(){
	int iRes=0;
	DWORD dwVal=0, dwSize=sizeof(DWORD);
	DWORD dwType = REG_DWORD;
	DWORD dwReturn=0;
	TCHAR subkey[MAX_PATH];
	//prepare subkey to read
	HKEY hKey=NULL;
	wsprintf(subkey, L"%s", _szRegKey);
	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_QUERY_VALUE, &hKey);
	if (rc != ERROR_SUCCESS){
		nclog(L"getMaxDelay: FATAL cannot open key '%s': %u. Using default 1\n", subkey, rc);
		dwReturn=1;
		goto exit_regReadDbgLevel;
	}
	//read value
	rc = RegQueryValueEx(hKey, L"maxDelay", NULL, &dwType, (LPBYTE) &dwVal, &dwSize);

	if(rc == 0){
		DEBUGMSG(1, (L"getMaxDelay: OK. dbglevel is %u\n", dwVal));
		dwReturn=dwVal;
	}
	else{
		dwReturn=1;
		DEBUGMSG(1, (L"getMaxDelay: FAILED %u. Using default 1\n", rc));
	}
exit_regReadDbgLevel:
	RegCloseKey(hKey);
	return dwReturn;
}

//--------------------------------------------------------------------
// Function name  : writeVersion
// Description    : write version number to registry
// Argument       : DWORD newVersion, the version number to store
// Return type    : int, 0 for no error
//--------------------------------------------------------------------
int writeVersion(DWORD newVersion)
{
	int iRes=0;
	TCHAR subkey[MAX_PATH];
	HKEY hKey=NULL;
	DWORD dwVal=0;
	DWORD dwSize=sizeof(DWORD);
	DWORD dwType = REG_DWORD;

	//prepare subkey to write
	wsprintf(subkey, L"%s", _szRegKey);
	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, &hKey);
	if(rc != 0){
		nclog(L"writeVersion: Could not write %s\n", subkey);
		iRes=-1;
		goto exit_writeVersion;
	}

	dwVal=newVersion;
	rc = RegSetValueEx(	hKey, L"Version", NULL, dwType, (LPBYTE) &dwVal, dwSize); 

	if(rc == 0)
		DEBUGMSG(1, (L"writeVersion: OK. Version is %i\n", dwVal));
	else
		DEBUGMSG(1, (L"writeVersion: FAILED %i\n", rc));

exit_writeVersion:
	if(hKey!=NULL)
		RegFlushKey(hKey);
	RegCloseKey(hKey);
	return iRes;
}

