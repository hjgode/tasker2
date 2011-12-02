// SetDateTime.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

void dumpST(SYSTEMTIME st){
#if !DEBUG
	return;
#endif
	TCHAR szStr[MAX_PATH];
	wsprintf(szStr, L"%04i %02i %02i %02i:%02i:%02i",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond);
	DEBUGMSG(1, (L"%s\n", szStr));
}


int _tmain(int argc, _TCHAR* argv[])
{
	int iRet=0;
	if(argc==1)
	{
		MessageBeep(MB_ICONERROR);
		iRet = -10;
	}
	if(argc==2){
		TCHAR szDateTime[MAX_PATH];
		TCHAR szTemp[MAX_PATH];
		//should be yyyyMMddhhmm, ie 201112011650
		wsprintf(szDateTime, L"%s", argv[1]);
		SYSTEMTIME st;
		//create a pointer for szDateTime
		TCHAR* pszDateTime = szDateTime;

		memset(szTemp, 0, sizeof(TCHAR)*MAX_PATH);
		wcsncpy(szTemp, pszDateTime, 4);
		int iYear;
		iYear=_wtoi(szTemp);
		if(iYear==0)
			iRet = -1;
		pszDateTime+=4;
		
		memset(szTemp, 0, sizeof(TCHAR)*MAX_PATH);
		wcsncpy(szTemp, pszDateTime, 2);
		int iMonth;
		iMonth=_wtoi(szTemp);
		if(iMonth==0)
			iRet = -2;
		pszDateTime+=2;

		memset(szTemp, 0, sizeof(TCHAR)*MAX_PATH);
		wcsncpy(szTemp, pszDateTime, 2);
		int iDay;
		iDay=_wtoi(szTemp);
		if(iDay==0)
			iRet = -3;
		pszDateTime+=2;

		memset(szTemp, 0, sizeof(TCHAR)*MAX_PATH);
		wcsncpy(szTemp, pszDateTime, 2);
		int iHour;
		iHour=_wtoi(szTemp);
		pszDateTime+=2;

		memset(szTemp, 0, sizeof(TCHAR)*MAX_PATH);
		wcsncpy(szTemp, pszDateTime, 2);
		int iMin;
		iMin=_wtoi(szTemp);

		memset(&st, 0, sizeof(SYSTEMTIME));
		st.wDay=iDay;
		st.wMonth=iMonth;
		st.wYear=iYear;
		st.wHour=iHour;
		st.wMinute=iMin;

		dumpST(st);

		if(iRet==0)
			if(!SetLocalTime(&st))
				iRet = GetLastError();
	}
	if(iRet!=0)
		MessageBeep(MB_ICONERROR);
	else
		MessageBeep(MB_ICONINFORMATION);
	return iRet;
}

