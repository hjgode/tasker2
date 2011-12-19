//regRW.cpp

#include "stdafx.h"
#include "task.h"
#include "regRW.h"
#include "./common/nclog.h"

//CTask _ctasks[10];

TASK _Tasks[iMaxTasks];
int iTaskCount=0;

DWORD _dwVersion = 234L;
int _dbgLevel = 0;

static TCHAR* _szRegKey = L"Software\\tasker";
TCHAR _szRegSubKeys[10][MAX_PATH];

int getTaskNumber(TCHAR* _sTask){
	TCHAR* sTemp = new TCHAR[wcslen(_sTask)+1];
	int iPos = wcslen(_sTask) - wcslen(L"Task");
	sTemp=&_sTask[wcslen(L"Task")];
	wcscat(sTemp, L"\0");
	int iRet = _wtoi(sTemp);
	return iRet-1;
}


///convert a HourMinute string (ie "1423) to a systemtime
int getSTfromString(SYSTEMTIME* sysTime /*in,out*/, TCHAR* sStr /*in*/){

	if(_dbgLevel>4) 
		nclog(L"getSTfromString: ...\n");
	int iRet=-1;
	if(wcslen(sStr)!=4){
		if(_dbgLevel>4) nclog(L"getSTfromString: failure, string len not equal to 4\n");
		return -1;	//string to short
	}
	extern SYSTEMTIME g_CurrentStartTime;
	memcpy(sysTime, &g_CurrentStartTime, sizeof(SYSTEMTIME));
	//GetLocalTime(sysTime); //v2.28
	int iTime = _wtoi(sStr);
	//v2.30: removed testing if 0
	//if(iTime == 0)
	//	return -2;	//string not a number
	int iHour = iTime/100;
	int iMinute = iTime % 100;
	sysTime->wHour=iHour;
	sysTime->wMinute=iMinute;
	if(_dbgLevel>4) nclog(L"getSTfromString: return with '%s'\n", sStr);
	return 0;
}

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

///convert a systemtime to a HourMinute string (ie "1423")
int getStrFromSysTime(SYSTEMTIME sysTime, TCHAR sStr[4+1]){
	if(_dbgLevel>4) 
		nclog(L"getSTfromString2: ...\n");
	int iRet=-1;
	if(wcslen(sStr)!=4){
		if(_dbgLevel>4) nclog(L"getSTfromString2: failure, string len not equal to 4\n");
		return -1;	//string to short
	}
	TCHAR sTemp[4+1];
	wsprintf(sTemp, L"%02i%02i", sysTime.wHour, sysTime.wMinute);
	if(wcsncpy(sStr, sTemp, 4)==NULL){
		if(_dbgLevel>4) nclog(L"getSTfromString2: returning with error for '%s'\n", sTemp);
		return -1;
	}
	else{
		if(_dbgLevel>4) nclog(L"getSTfromString2: returning with '%s'\n", sStr);
		return 0; //no Error
	}
}

///convert a YearMonthDayHourMinute string (ie "201110201423) to a systemtime
int getSTfromLongString(SYSTEMTIME * sysTime /*in,out*/, TCHAR* sStr /*in*/){
	int iRet=-1;
	if(wcslen(sStr) != 12){
		return -1;	//string to short
	}
	extern SYSTEMTIME g_CurrentStartTime;
	memcpy(sysTime, &g_CurrentStartTime, sizeof(SYSTEMTIME));
	//GetLocalTime(sysTime); //v2.28
	//
	TCHAR* pStr = sStr;
	TCHAR sYear[MAX_PATH];
	int iYear = -1;
	TCHAR sMonth[MAX_PATH];
	int iMonth = -1;
	TCHAR sDay[MAX_PATH];
	int iDay = -1;
	TCHAR sHour[MAX_PATH];
	int iHour = -1;
	TCHAR sMinute[MAX_PATH];
	int iMinute = -1;

	wcsncpy(sYear, pStr, 4);	pStr+=4;
	wcsncpy(sMonth, pStr, 2);		pStr+=2;
	wcsncpy(sDay, pStr, 2);	pStr+=2;
	wcsncpy(sHour, pStr, 2);	pStr+=2;
	wcsncpy(sMinute, pStr, 2);	pStr+=4;

	if((iYear=_wtoi(sYear))<0)
		return -11;
	if((iMonth=_wtoi(sMonth))<0)
		return -12;
	if((iDay=_wtoi(sDay))<0)
		return -13;
	if((iHour=_wtoi(sHour))<0)
		return -14;
	if((iMinute=_wtoi(sMinute))<0)
		return -15;
	sysTime->wYear=iYear;
	sysTime->wMonth=iMonth;
	sysTime->wDay=iDay;
	sysTime->wHour=iHour;
	sysTime->wMinute=iMinute;
	sysTime->wSecond=0;
	sysTime->wMilliseconds=0;

	return 0;
}

///convert a systemtime to a YearMonthDayHourMinute string (ie "201112241423")
int getLongStrFromSysTime(SYSTEMTIME sysTime, TCHAR* sStr){
	int iRet=-1;
	wsprintf(sStr, L"000000000000");
	int iSize = wcslen(sStr);
	if(iSize != 12){
		if(_dbgLevel>4) nclog(L"getLongStrFromSysTime: str len not equal 12\n");
		return -1;	//string to short
	}
	TCHAR sTemp[12+1];
	wsprintf(sTemp, L"%04i%02i%02i%02i%02i", 
		sysTime.wYear,
		sysTime.wMonth,
		sysTime.wDay,
		sysTime.wHour, 
		sysTime.wMinute);
	wsprintf(sStr, L"%s", sTemp);
	if(_dbgLevel>4) nclog(L"getLongStrFromSysTime: returning with '%s'\n", sStr);
	return 0; //no Error

}

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

TCHAR* getLongStrFromSysTime2(SYSTEMTIME sysTime){
		TCHAR* sTemp = new TCHAR[12+1];
		wsprintf(sTemp, L"%04i%02i%02i%02i%02i", 
			sysTime.wYear,
			sysTime.wMonth,
			sysTime.wDay,
			sysTime.wHour, 
			sysTime.wMinute);
		wsprintf(sTemp, L"%s", sTemp);
		return sTemp; //no Error
}

//normalize a SYSTEMTIME, ie, if hours is > 24
SYSTEMTIME fixSystemTime(SYSTEMTIME st){
	TCHAR szTemp[13]; wsprintf(szTemp, L"000000000000");
	getLongStrFromSysTime(st, szTemp);
	if(_dbgLevel>4) 
		nclog(L"fixSystemTime: started with '%s'\n", szTemp);

	short shMin=st.wMinute;
	//minutes above 60? add to hours
	if(shMin>=60){
		st.wHour+=shMin/60;
		shMin=shMin%60;
	}
	short shHour = st.wHour;
	//hours above 24? add to days
	if(shHour>24)
	{
		st.wDay+=shHour/24;
		shHour=shHour%24;
	}

	SYSTEMTIME stReturn;
	memcpy(&stReturn, &st, sizeof(SYSTEMTIME));

	stReturn.wMinute=shMin;
	stReturn.wHour=shHour;

	getLongStrFromSysTime(stReturn, szTemp);
	if(_dbgLevel>4) nclog(L"fixSystemTime: returning with '%s'\n", szTemp);
	return stReturn;
}

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

int regReadKeys(){
	int iRet = 0;
	DWORD dwDbgLevel=regReadDbgLevel();
	
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

		SYSTEMTIME st;
		//change for v2.2: always only read start/stop values and use NextStart and NextStop only as info
		dwSize=sizeof(TCHAR)*MAX_PATH;
		dwType=REG_SZ;
		rc=RegQueryValueEx(hKey, L"start", 0, &dwType, (LPBYTE) &szVal, &dwSize);
		if(rc==0){
			if(_dbgLevel>4) 
				nclog(L"\tregReadKeys: 'start' entry is '%s'\n", szVal);
			iRes=getSTfromString(&st, szVal);
			if(iRes==0){
				_Tasks[i].stStartTime=fixSystemTime(st);
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
			iRes=getSTfromString(&st, szVal);
			if(iRes==0){
				if(_dbgLevel>4) 
					nclog(L"\tregReadKeys: task.stStopTime entry set\n");
				_Tasks[i].stStopTime=fixSystemTime(st);
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
			iRes=getSTfromString(&st, szVal);
			if(iRes==0){
				if(_dbgLevel>4) 
					nclog(L"\tregReadKeys: 'interval' using %02i:%02i\n", st.wHour, st.wMinute);
				if(st.wHour==0 && st.wMinute==0 && st.wDay==0){ //interval 000000 not supported
					_Tasks[i].iActive = 0;
					nclog(L"interval = 0 is NOT supported\n");
					iRet=-99; //can not read exe entry
					goto exit_readallkeys;
				}
				_Tasks[i].stDiffTime=st;
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
		DEBUGMSG(1, (L"regSetStartTime: OK. Debug Level is %s\n", szVal));
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

int regSetStartTime(int iTask, SYSTEMTIME pStartTime){
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

	if((iRet=getStrFromSysTime(pStartTime, szHM))!=0)
		goto exit_regSetStartTime;

	//prepare subkey to read
	wsprintf(subkey, L"%s\\%s", _szRegKey, _szRegSubKeys[iTask]);
	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, &hKey);
	if (rc != ERROR_SUCCESS){
		nclog(L"regSetStartTime: FATAL cannot open key '%s': %u\n", subkey, rc);
		iRet=-1;
		goto exit_regSetStartTime;
	}
	//change for v2.2: always only read start/start values and use NextStart and NextStart only as info
	wsprintf(szVal, L"%s", szHM);
	dwSize=wcslen(szHM)*sizeof(TCHAR);
	dwType=REG_SZ;

	rc = RegSetValueEx(hKey, L"start", 0, dwType, (LPBYTE)szVal, dwSize);
	if(rc == 0)
		DEBUGMSG(1, (L"regSetStartTime: OK. Debug Level is %s\n", szVal));
	else{
		DEBUGMSG(1, (L"regSetStartTime: FAILED %i\n", rc));
	}

	TCHAR sNextStart[13]; 
	wsprintf(sNextStart, L"000000000000");
	if(getLongStrFromSysTime(pStartTime, sNextStart)==0){
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

exit_regSetStartTime:
	if(hKey!=NULL)
		RegFlushKey(hKey);
	RegCloseKey(hKey);
	delete(szHM);
	return iRet;
}

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
		DEBUGMSG(1, (L"regSetStopTime: OK. Debug Level is %s\n", szVal));
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

int regSetStopTime(int iTask, SYSTEMTIME pStopTime){
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

	if((iRet=getStrFromSysTime(pStopTime, szHM))!=0)
		goto exit_regSetStopTime;

	//prepare subkey to read
	wsprintf(subkey, L"%s\\%s", _szRegKey, _szRegSubKeys[iTask]);
	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, &hKey);
	if (rc != ERROR_SUCCESS){
		nclog(L"regSetStopTime: FATAL cannot open key '%s': %u\n", subkey, rc);
		iRet=-1;
		goto exit_regSetStopTime;
	}
	//change for v2.2: always only read start/stop values and use NextStart and NextStop only as info
	wsprintf(szVal, L"%s", szHM);
	dwSize=wcslen(szHM)*sizeof(TCHAR);
	dwType=REG_SZ;

	rc = RegSetValueEx(hKey, L"stop", 0, dwType, (LPBYTE)szVal, dwSize);
	if(rc == 0)
		DEBUGMSG(1, (L"regSetStopTime: OK. Debug Level is %s\n", szVal));
	else{
		DEBUGMSG(1, (L"regSetStopTime: FAILED %i\n", rc));
	}

	TCHAR sNextStop[13]; 
	wsprintf(sNextStop, L"000000000000");
	if(getLongStrFromSysTime(pStopTime, sNextStop)==0){
		wsprintf(szVal, L"%s", sNextStop);
		dwSize=sizeof(TCHAR)*wcslen(szVal);
		rc = RegSetValueEx(hKey, L"NextStop", 0, dwType, (LPBYTE)szVal, dwSize);
		if(rc == 0){
			DEBUGMSG(1,(L"regSetStopTime: changed NextStop reg for task%i to '%s'\n", iTask+1, sNextStop));
			iRet=10;
		}
		else
			DEBUGMSG(1, (L"regSetStopTime: changing stop reg for task%i to '%s' Failed: %i\n", iTask+1, szHM, rc));
	}
	else{
		DEBUGMSG(1, (L"regSetStopTime: -getLongStrFromSysTime() failed\n"));
	}

exit_regSetStopTime:
	if(hKey!=NULL)
		RegFlushKey(hKey);
	RegCloseKey(hKey);
	delete(szHM);
	return iRet;
}

//BOOL getUpdateAll()
//{
//	int iRes=0;
//	DWORD dwVal=0, dwSize=sizeof(DWORD);
//	DWORD dwType = REG_DWORD;
//	DWORD dwReturn=FALSE;
//	TCHAR subkey[MAX_PATH];
//	//prepare subkey to read
//	HKEY hKey=NULL;
//	wsprintf(subkey, L"%s", _szRegKey);
//	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_QUERY_VALUE, &hKey);
//	if (rc != ERROR_SUCCESS){
//		nclog(L"getUpdateAll: FATAL cannot open key '%s': %u. Using default 0\n", subkey, rc);
//		dwReturn=FALSE;
//		goto exit_getUpdateAll;
//	}
//	//read value
//	rc = RegQueryValueEx(hKey, L"UpdateAll", NULL, &dwType, (LPBYTE) &dwVal, &dwSize);
//
//	if(rc == 0){
//		DEBUGMSG(1, (L"getUpdateAll: OK. Version is %u\n", dwVal));
//		if(dwVal)
//			dwReturn=TRUE;
//		else
//			dwReturn=FALSE;
//	}
//	else{
//		dwReturn=FALSE;
//		DEBUGMSG(1, (L"getUpdateAll: FAILED %u\n", rc));
//	}
//exit_getUpdateAll:
//	RegCloseKey(hKey);
//
//	setUpdateAll();
//
//	return dwReturn;
//}

void setUpdateAll()
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
		nclog(L"setUpdateAll: Could not write %s\n", subkey);
		iRes=-1;
		goto exit_setUpdateAll;
	}

	dwVal=1;
	rc = RegSetValueEx(	hKey, L"UpdateAll", NULL, dwType, (LPBYTE) &dwVal, dwSize); 

	if(rc == 0)
		DEBUGMSG(1, (L"setUpdateAll: OK. Debug Level is %i\n", dwVal));
	else
		DEBUGMSG(1, (L"setUpdateAll: FAILED %i\n", rc));

exit_setUpdateAll:
	if(hKey!=NULL)
		RegFlushKey(hKey);
	RegCloseKey(hKey);
}

void unsetUpdateAll()
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
		nclog(L"unsetUpdateAll: Could not write %s\n", subkey);
		iRes=-1;
		goto exit_unsetUpdateAll;
	}

	dwVal=0;
	rc = RegSetValueEx(	hKey, L"UpdateAll", NULL, dwType, (LPBYTE) &dwVal, dwSize); 

	if(rc == 0)
		DEBUGMSG(1, (L"unsetUpdateAll: OK. Debug Level is %i\n", dwVal));
	else
		DEBUGMSG(1, (L"unsetUpdateAll: FAILED %i\n", rc));

exit_unsetUpdateAll:
	if(hKey!=NULL)
		RegFlushKey(hKey);
	RegCloseKey(hKey);
}

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
		DEBUGMSG(1, (L"writeMaxDelay: OK. Debug Level is %i\n", dwVal));
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

