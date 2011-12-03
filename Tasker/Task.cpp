#include "StdAfx.h"
#include "Task.h"

CTask::CTask(void)
{

}

//get number at end of string, ie "Task1"
int CTask::getTaskNumber(TCHAR* _sTask){
	TCHAR* sTemp = new TCHAR[wcslen(_sTask)+1];
	int iPos = wcslen(_sTask) - wcslen(L"Task");
	sTemp=&_sTask[wcslen(L"Task")];
	wcscat(sTemp, L"\0");
	int iRet = _wtoi(sTemp);
	return iRet-1;
}

CTask::~CTask(void)
{
}
