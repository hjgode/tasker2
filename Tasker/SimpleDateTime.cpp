// SimpleDate.cpp: implementation of the CSimpleDateTime class.
//
//	Most of the date alogrithms used in this class can be found at
//	http://www.capecod.net/~pbaum/date/date0.htm
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SimpleDateTime.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma message( "Compiling " __FILE__ ) 
#pragma message( "Last modified on " __TIMESTAMP__ ) 


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static int  monthtotals[]=
    { 0, 0,  31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 } ;

static char* daysofweek[] =
{
   { "Sun" },
   { "Mon" },
   { "Tue" },
   { "Wed" },
   { "Thu" },
   { "Fri" },
   { "Sat" },
} ;

static char* days_of_week[] =
{
   { "Sunday" },
   { "Monday" },
   { "Tuesday" },
   { "Wednesday" },
   { "Thursday" },
   { "Friday" },
   { "Saturday" },
} ;

static char* monthsofyear[] =
{
   { "" },
   { "Jan" },
   { "Feb" },
   { "Mar" },
   { "Apr" },
   { "May" },
   { "Jun" },
   { "Jul" },
   { "Aug" },
   { "Sep" },
   { "Oct" },
   { "Nov" },
   { "Dec" },
} ;

static char* months_of_year[] =
{
   { "" },
   { "January" },
   { "February" },
   { "March" },
   { "April" },
   { "May" },
   { "June" },
   { "July" },
   { "August" },
   { "September" },
   { "October" },
   { "November" },
   { "December" },
} ;

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::CSimpleDateTime
// Description	    : Simple Constructor object is set to todays date
// Return type		: 
//--------------------------------------------------------------------
CSimpleDateTime::CSimpleDateTime()
{
	DEBUGMSG(1, (L"CSimpleDateTime init without arg\n"));
	SetToday();
//	SetTime();
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::CSimpleDateTime
// Description	    : paramaterized constructor object is set to date in systemTime.
// Return type		: 
// Argument         : SYSTEMTIME systemTime
//--------------------------------------------------------------------
CSimpleDateTime::CSimpleDateTime(SYSTEMTIME systemTime)
{
	DEBUGMSG(1, (L"CSimpleDateTime init with systemTime\n"));
	FILETIME ft;
	if(!SystemTimeToFileTime(&systemTime, &ft)){
		systemTime.wDayOfWeek=0;
		systemTime.wSecond=0;
		systemTime.wMilliseconds=0;
	}
	m_systemTime = systemTime;
	CTime
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::CSimpleDateTime
// Description	    : paramaterized constructor object is set to date in string. Parsing will be done based on formattype
// Return type		: 
// Argument         : LPCWSTR DateString
//--------------------------------------------------------------------

CSimpleDateTime::CSimpleDateTime(LPCWSTR DateString)
{
	DEBUGMSG(1, (L"CSimpleDateTime init with '%s'\n"));
//	if(wcslen(DateString)!=12)
		
	ParseDateTimeString(DateString);
	FILETIME ft;
	if(!SystemTimeToFileTime(&m_systemTime, &ft)){
		m_systemTime.wDayOfWeek=0;
		m_systemTime.wSecond=0;
		m_systemTime.wMilliseconds=0;
	}
//	m_JulianDate=ConvertToJulian();	
	//SetTime();
}


//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::~CSimpleDateTime
// Description	    : 
// Return type		: 
//--------------------------------------------------------------------
CSimpleDateTime::~CSimpleDateTime()
{
}


//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::SetToday
// Description	    : 
// Return type		: BOOL 
//--------------------------------------------------------------------
BOOL CSimpleDateTime::SetToday()
{
	GetLocalTime(&m_systemTime);
	return TRUE;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::SetToday
// Description	    : 
// Return type		: BOOL 
//--------------------------------------------------------------------
BOOL CSimpleDateTime::SetNow()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	m_systemTime=st;
	return TRUE;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::IsValid
// Description	    : Hope that this is right and Y2K OK
// Return type		: BOOL 
//--------------------------------------------------------------------
BOOL CSimpleDateTime::IsValid()
{
	   int is_leap, month_days ;

	   is_leap =  ( m_systemTime.wYear%4 == 0 && m_systemTime.wYear%100 != 0 || m_systemTime.wYear%400 == 0 ) ?  1 : 0 ;
		if(m_systemTime.wMonth > 12 || m_systemTime.wDay > 31)
			return FALSE;
	   month_days = monthtotals[ m_systemTime.wMonth+1 ] -  monthtotals[ m_systemTime.wMonth] ;
	   if ( m_systemTime.wMonth == 2 )  month_days += is_leap ;

	   if ( m_systemTime.wYear  < 0  ||
			m_systemTime.wMonth < 1  ||  m_systemTime.wMonth > 12  ||
			m_systemTime.wDay  < 1  ||  m_systemTime.wDay   > month_days )
			return FALSE ;        
	   return TRUE;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::ParseDateString
// Description	    : 
// Return type		: BOOL 
// Argument         : LPCWSTR TheDate as YYYYMMDDhhmm string
//--------------------------------------------------------------------
BOOL CSimpleDateTime::ParseDateTimeString(LPCWSTR TheDateTime)
{
	ParseDateString(TheDateTime, m_systemTime.wMonth, m_systemTime.wDay, m_systemTime.wYear, m_systemTime.wHour, m_systemTime.wMinute);
	return TRUE;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::ParseDateString
// Description	    : 
// Return type		: BOOL 
// Argument         : LPCWSTR TheDate
//--------------------------------------------------------------------
BOOL CSimpleDateTime::ParseDateString(LPCWSTR date,WORD& m, WORD& d, WORD& y, WORD& hour, WORD& min)
{
	BOOL bRet=TRUE; 
	int iErrCnt=0;
	TCHAR szDate[MAX_PATH]; //ie 201103041534
	wsprintf(szDate, L"%s", date);
	TCHAR* pszDateTime=szDate;
	TCHAR szTemp[MAX_PATH];

	m=0; d=0; y=0;
	if(wcslen(date)==4){
		memset(&m_systemTime,0,sizeof(SYSTEMTIME));
		m_systemTime.wHour=_wtol(date) / 100;
		m_systemTime.wMinute=_wtol(date) % 100;
		return TRUE;
	}
	//YYYY
	memset(szTemp, 0, sizeof(TCHAR)*MAX_PATH);
	wcsncpy(szTemp, pszDateTime, 4);
	int iYear;
	iYear=_wtoi(szTemp);
	if(iYear==0)
		iErrCnt += 1;
	pszDateTime+=4;

	//MM
	memset(szTemp, 0, sizeof(TCHAR)*MAX_PATH);
	wcsncpy(szTemp, pszDateTime, 2);
	int iMonth;
	iMonth=_wtoi(szTemp);
	if(iMonth==0)
		iErrCnt += 1;
	pszDateTime+=2;

	//DD
	memset(szTemp, 0, sizeof(TCHAR)*MAX_PATH);
	wcsncpy(szTemp, pszDateTime, 2);
	int iDay;
	iDay=_wtoi(szTemp);
	if(iDay==0)
		iErrCnt += 1;
	pszDateTime+=2;

	//hh
	memset(szTemp, 0, sizeof(TCHAR)*MAX_PATH);
	wcsncpy(szTemp, pszDateTime, 2);
	int iHour;
	iHour=_wtoi(szTemp);
	pszDateTime+=2;

	//mm
	memset(szTemp, 0, sizeof(TCHAR)*MAX_PATH);
	wcsncpy(szTemp, pszDateTime, 2);
	int iMin;
	iMin=_wtoi(szTemp);

	if(iYear)
		y=iYear;
	if(iMonth)
		m=iMonth;
	if(iDay)
		d=iDay;
	
	hour = iHour;
	min = iMin;
	
	if(iErrCnt>0)
		bRet=FALSE;
	return bRet;
}

BOOL CSimpleDateTime::ParseDateString(LPCWSTR date, WORD& m, WORD& d, WORD& y)
{
	BOOL bRet=TRUE;
	TCHAR szDate[MAX_PATH]; //ie 201103041534
	wsprintf(szDate, L"%s", date);
	TCHAR* pszDateTime=szDate;
	TCHAR szTemp[MAX_PATH];

	m=0; d=0; y=0;
	//YYYY
	memset(szTemp, 0, sizeof(TCHAR)*MAX_PATH);
	wcsncpy(szTemp, pszDateTime, 4);
	int iYear;
	iYear=_wtoi(szTemp);
	if(iYear==0)
		bRet = FALSE;
	pszDateTime+=4;

	//MM
	memset(szTemp, 0, sizeof(TCHAR)*MAX_PATH);
	wcsncpy(szTemp, pszDateTime, 2);
	int iMonth;
	iMonth=_wtoi(szTemp);
	if(iMonth==0)
		bRet = FALSE;
	pszDateTime+=2;

	//DD
	memset(szTemp, 0, sizeof(TCHAR)*MAX_PATH);
	wcsncpy(szTemp, pszDateTime, 2);
	int iDay;
	iDay=_wtoi(szTemp);
	if(iDay==0)
		bRet = FALSE;
	pszDateTime+=2;

	if(iYear)
		y=iYear;
	else
		y=0;
	if(iMonth)
		m=iMonth;
	else
		m=0;
	if(iDay)
		d=iDay;
	else
		d=0;
	
	return bRet;
}

//--------------------------------------------------------------------
//	JULIAN DATE CONVERSION ABOVE
//--------------------------------------------------------------------
//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::GetDayOfWeek
// Description	    : goto http://www.capecod.net/~pbaum/date/date0.htm for details
// Return type		: int 
//--------------------------------------------------------------------
int CSimpleDateTime::GetDayOfWeek()
{
	if(!IsValid())
		return 0;
	int res= ((int) (m_systemTime.wDayOfWeek));
	return res;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::GetFullDateString
// Description	    : return string in form 'DD MM YYYY hh:mm'
// Return type		: LPCWSTR 
//--------------------------------------------------------------------
LPCWSTR CSimpleDateTime::GetFullDateString()
{
	//if(!IsValid())
	//	return NULL;
	TCHAR* str = L"%02i %02i %04i %02i:%02i";
	m_DateString.Format(str, m_systemTime.wDay , m_systemTime.wMonth, m_systemTime.wYear, m_systemTime.wHour, m_systemTime.wMinute);
	//m_DateString.Format("%s %s %02d %04d", daysofweek[GetDayOfWeek()],monthsofyear[m_systemTime.wMonth],m_systemTime.wDay,m_systemTime.wYear);
	return m_DateString;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::GetFullDateString
// Description	    : return string in form 'YYYYMMDDhhmm'
// Return type		: LPCWSTR 
//--------------------------------------------------------------------
LPCWSTR CSimpleDateTime::GetDateTimeString()
{
	//if(!IsValid())
	//	return NULL;
	TCHAR str[MAX_PATH];
	wsprintf(str/*m_DateString*/, L"%04i%02i%02i%02i%02i", m_systemTime.wYear, m_systemTime.wMonth, m_systemTime.wDay, m_systemTime.wHour, m_systemTime.wMinute);
	//m_DateString.Format("%s %s %02d %04d", daysofweek[GetDayOfWeek()],monthsofyear[m_systemTime.wMonth],m_systemTime.wDay,m_systemTime.wYear);
	m_DateString = str;
	return m_DateString;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::GetFullDateStringLong
// Description	    : 
// Return type		: LPCWSTR 
//--------------------------------------------------------------------
LPCWSTR CSimpleDateTime::GetFullDateStringLong()
{
	//if(!IsValid())
	//	return NULL;
	TCHAR* str = L"%02i %02i %04i %02i:%02i";
	m_DateString.Format(str, m_systemTime.wDay , m_systemTime.wMonth, m_systemTime.wYear, m_systemTime.wHour, m_systemTime.wMinute);
	//m_DateString.Format("%s %s %02d %04d", days_of_week[GetDayOfWeek()],months_of_year[m_systemTime.wMonth],m_systemTime.wDay,m_systemTime.wYear);
	return m_DateString;
}



//--------------------------------------------------------------------
//	overloaded operators and copy constructors here
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// OPERATORS
//--------------------------------------------------------------------
// Function name	: DOUBLE
// Description	    : return the date as DOUBLE 201112241200 for 12:00 24.12.2011
// Return type		: CSimpleDateTime::operator return long
//--------------------------------------------------------------------
CSimpleDateTime::operator DOUBLE()
{
	if(!IsValid())
		return 0;
	return m_systemTime.wYear*10^10 + m_systemTime.wMonth*10^6 + m_systemTime.wDay*10^4 + m_systemTime.wHour*10^2 + m_systemTime.wMinute*1;
}

//--------------------------------------------------------------------
// Function name	: =
// Description	    : 
// Return type		: CSimpleDateTime::operator 
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::operator =(const CSimpleDateTime& Date)
{
	if(*this==Date)
		return *this;	//	self assignment
	m_systemTime.wYear=Date.m_systemTime.wYear;
	m_systemTime.wMonth=Date.m_systemTime.wMonth;
	m_systemTime.wDay=Date.m_systemTime.wDay;
	return *this;
}

////////////
//equivalent of DATEADD function from SQLServer
//Returns a new datetime value based on adding an interval
// to the specified date.
////////////*/
SYSTEMTIME /*new datetime*/
CSimpleDateTime::DT_AddDiff
			(	const __int64 datepart, /*datepart with we want to manipulate, 
			{nano100SecInDay ...}*/
			const __int64 num, /*value used to increment/decrement datepart*/
			const SYSTEMTIME* pst /*valid datetime which we want change*/
			)
{
	FILETIME ft;
	SYSTEMTIME st;
	__int64* pi; 

	SystemTimeToFileTime (pst,&ft); 
	pi = (__int64*)&ft; 
	(*pi) += (__int64)num*datepart; 

	/*convert FILETIME to SYSTEMTIME*/
	FileTimeToSystemTime (&ft,&st); 

	/*now, st contain new valid datetime, so return it*/
	return st;
}

SYSTEMTIME CSimpleDateTime::DT_AddDays(const SYSTEMTIME st, int days){
	SYSTEMTIME stStart;
	memcpy(&stStart, &st, sizeof(SYSTEMTIME));
	//GetLocalTime(&stNow);
	//stStart.wHour=st.wHour;
	//stStart.wMinute=st.wMinute;
	//stStart.wSecond=st.wSecond;

	SYSTEMTIME stNew = DT_AddDiff(nano100SecInDay, days, &stStart);

	return stNew;
}

// save inTime, get LocalTime and then set DAY, hour and minute of inTime
// added support for negaitive days, hours and minutes in v2.34
SYSTEMTIME CSimpleDateTime::DT_Add(const SYSTEMTIME& Date, short Years, short Months, short Days, short Hours, short Minutes, short Seconds, short Milliseconds) 
{
	FILETIME ft; SYSTEMTIME st; ULARGE_INTEGER ul1;
	/*### DO NOT CHANGE OR NORMALIZE INPUT ###*/
	//create a new systime and copy the single values to it	
	//SYSTEMTIME inTime;
	//memset(&inTime, 0, sizeof(SYSTEMTIME));
	//inTime.wDay = Date.wDay;
	//inTime.wHour = Date.wHour;
	//inTime.wMinute = Date.wMinute;

	//memcpy((void*)&Date, &inTime, sizeof(SYSTEMTIME));

	//convert INPUT to filetime
	SYSTEMTIME stStart;
	//memset(&stStart, 0, sizeof(SYSTEMTIME));
	memcpy(&stStart, &Date, sizeof(SYSTEMTIME));
	if (!SystemTimeToFileTime(&Date, &ft))
	{
		DEBUGMSG(1, (L"DT_Add: error in SystemTimeToFileTime: %i\n", GetLastError()));
		return Date;
	}
	ul1.HighPart = ft.dwHighDateTime;
	ul1.LowPart = ft.dwLowDateTime;
	 
	if (Milliseconds) 
		ul1.QuadPart += (Milliseconds * 10000); 

	if (Seconds)
		ul1.QuadPart += (Seconds * (__int64)10000000); 

	if (Minutes>0)
		ul1.QuadPart += (Minutes * (__int64)10000000 * 60); 
	else if (Minutes<0)
		ul1.QuadPart += (Minutes * (__int64)10000000 * 60); 

	if (Hours>0) 
		ul1.QuadPart += (Hours * (__int64)10000000 * 60 * 60);
	else if (Hours<0)
		ul1.QuadPart += (Hours * (__int64)10000000 * 60 * 60);

	if (Days>0)
		ul1.QuadPart += (Days * (__int64)10000000 * 60 * 60 * 24); 
	else if (Days<0)
		ul1.QuadPart += (Days * (__int64)10000000 * 60 * 60 * 24); 
	 
	ft.dwHighDateTime = ul1.HighPart;
	ft.dwLowDateTime = ul1.LowPart;
	
	//try to convert filetime back to a systemtime
	if (!FileTimeToSystemTime(&ft,&st)) {
		return Date;
	}
	 
	if (Months>0) {
		if ((Months += st.wMonth) <= 0) {
			Months *= (-1);
			st.wYear -= ((Months / 12) + 1);
			st.wMonth = 12 - (Months % 12);
		} else {
			st.wMonth = Months % 12;
			st.wYear += Months / 12;
		}
		while (!SystemTimeToFileTime(&st, &ft)) {
			st.wDay -= 1;
		}
	}
	return st;
}

//--------------------------------------------------------------------
// Function name	: +
// Description	    : 
// Return type		: CSimpleDateTime::operator 
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::operator +(LPCWSTR DateTime)
{
	ParseDateTimeString(DateTime);
	m_systemTime = DT_Add(this->m_systemTime, m_systemTime.wYear, m_systemTime.wMonth, m_systemTime.wDay, m_systemTime.wHour, m_systemTime.wMinute, m_systemTime.wSecond, 0);
	return *this;
}

//--------------------------------------------------------------------
// Function name	: +
// Description	    : 
// Return type		: CSimpleDateTime::operator 
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::operator +(SYSTEMTIME DateTime)
{
	m_systemTime = DT_Add(this->m_systemTime, DateTime.wYear, DateTime.wMonth, DateTime.wDay, DateTime.wHour, DateTime.wMinute, DateTime.wSecond, 0);
	//CSimpleDateTime newTime = CSimpleDateTime(sysTime);
	return  *this;
}

//--------------------------------------------------------------------
// Function name	: +
// Description	    : 
// Return type		: CSimpleDateTime::operator 
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::operator +(CSimpleDateTime& DateTime)
{
	SYSTEMTIME stStart = this->m_systemTime;
	SYSTEMTIME stNew = DT_Add(stStart, DateTime.m_systemTime.wYear, DateTime.m_systemTime.wMonth, DateTime.m_systemTime.wDay, 
		DateTime.m_systemTime.wHour, DateTime.m_systemTime.wMinute, DateTime.m_systemTime.wSecond, 0);
	//CSimpleDateTime newTime = CSimpleDateTime(sysTime);
	this->m_systemTime=stNew;
	return *this;//CSimpleDateTime(stNew);// *this;
}

//--------------------------------------------------------------------
// Function name	: =
// Description	    : 
// Return type		: CSimpleDateTime::operator 
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::operator =(LPCWSTR DateTime)
{
	ParseDateTimeString(DateTime);
	return *this;
}

//--------------------------------------------------------------------
//LONG CSimpleDateTime::getDouble(SYSTEMTIME sysTime){
//	LONG lDateTime =  m_systemTime.wYear	*1000000000 
//					+ m_systemTime.wMonth	*10000000 
//					+ m_systemTime.wDay		*100000
//					+ m_systemTime.wHour	*1000
//					+ m_systemTime.wMinute	*10
//					+ m_systemTime.wSecond;
//	return lDateTime;
//}

//--------------------------------------------------------------------
DOUBLE CSimpleDateTime::getDouble(SYSTEMTIME sysTime){
	DOUBLE lDateTime =  sysTime.wYear	*1000000000 
					+ sysTime.wMonth	*10000000 
					+ sysTime.wDay		*100000
					+ sysTime.wHour	*1000
					+ sysTime.wMinute	*10
					+ sysTime.wSecond;
	return lDateTime;
}

//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator > (const CSimpleDateTime& Date)	
{                        
	if(!IsValid())
		return FALSE;
	return getDouble(m_systemTime) > getDouble(Date.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator >= (const CSimpleDateTime& Date)	
{                        
	if(!IsValid())
		return FALSE;
	return getDouble(m_systemTime) >= getDouble(Date.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator < (const CSimpleDateTime& Date)	
{                        
	if(!IsValid())
		return FALSE;
	return getDouble(m_systemTime) < getDouble(Date.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator <= (const CSimpleDateTime& Date)	
{                        
	if(!IsValid())
		return FALSE;
	return getDouble(m_systemTime) <= getDouble(Date.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator == (const CSimpleDateTime& Date)	
{                        
	if(!IsValid())
		return FALSE;
	return getDouble(m_systemTime) == getDouble(Date.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator != (const CSimpleDateTime& Date)	
{                        
	if(!IsValid())
		return FALSE;
	DOUBLE thisDateTime = getDouble(m_systemTime);
	DOUBLE otherDateTime = getDouble(Date.m_systemTime);
	return (thisDateTime != otherDateTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator > (LPCWSTR Date)	
{                        
	if(!IsValid())
		return FALSE;
	CSimpleDateTime TheDate(Date);
	if(!TheDate.IsValid())
		return FALSE;
	return getDouble(m_systemTime) > getDouble(TheDate.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator < (LPCWSTR Date)	
{                        
	if(!IsValid())
		return FALSE;
	CSimpleDateTime TheDate(Date);
	if(!TheDate.IsValid())
		return FALSE;
	return getDouble(m_systemTime) < getDouble(TheDate.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator >= (LPCWSTR Date)	
{                        
	if(!IsValid())
		return FALSE;
	CSimpleDateTime TheDate(Date);
	if(!TheDate.IsValid())
		return FALSE;
	return getDouble(m_systemTime) >= getDouble(TheDate.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator <= (LPCWSTR Date)	
{                        
	if(!IsValid())
		return FALSE;
	CSimpleDateTime TheDate(Date);
	if(!TheDate.IsValid())
		return FALSE;
	return getDouble(m_systemTime) <= getDouble(TheDate.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator == (LPCWSTR Date)	
{                        
	if(!IsValid())
		return FALSE;
	CSimpleDateTime TheDate(Date);
	if(!TheDate.IsValid())
		return FALSE;
	return getDouble(m_systemTime) == getDouble(TheDate.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator != (LPCWSTR Date)	
{                        
	if(!IsValid())
		return FALSE;
	CSimpleDateTime TheDate(Date);
	if(!TheDate.IsValid())
		return FALSE;
	return getDouble(m_systemTime) != getDouble(TheDate.m_systemTime);
}			


//--------------------------------------------------------------------
// Date Math routines
//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::YearsOld
// Description	    : 
// Return type		: int 
//--------------------------------------------------------------------
int CSimpleDateTime::YearsOld()
{
	if(!IsValid())
		return 0;
	CSimpleDateTime ToDay;
	if(ToDay <= *this)
		return 0;
	long t=ToDay.GetYear();
	long b=GetYear();
	return (int)((t-b)/365.2425);

}


//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::AddMonths
// Description	    : 
// Return type		: const CSimpleDateTime& 
// Argument         : int Mon
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::AddMonths(int Mon)
{
	if(!IsValid())
		return *this;
	m_systemTime.wMonth+=Mon;
	if(m_systemTime.wMonth > 12)
	{
		while(m_systemTime.wMonth > 12)
		{
			m_systemTime.wMonth-=12;
			++m_systemTime.wYear;
		}
	}
	AdjustDays();
	return *this;
}


//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::SubtractMonths
// Description	    : 
// Return type		: const CSimpleDateTime& 
// Argument         : int Mon
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::SubtractMonths(int Mon)
{
	if(!IsValid())
		return *this;
	if(Mon > 12)
	{
		int t= Mon/12;
		m_systemTime.wYear-=t;
		Mon=Mon-(t*12);
	}
	m_systemTime.wMonth-=Mon;
	if(m_systemTime.wMonth < 1)
	{
		--m_systemTime.wYear;
		m_systemTime.wMonth+=12;
	}
	AdjustDays();
	return *this;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::AddYears
// Description	    : 
// Return type		: const CSimpleDateTime& 
// Argument         : int Yrs
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::AddYears(int Yrs)
{
	if(!IsValid())
		return *this;
	m_systemTime.wYear+=Yrs;
	AdjustDays();
	return *this;

}


//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::SubtractYears
// Description	    : 
// Return type		: const CSimpleDateTime& 
// Argument         : int Yrs
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::SubtractYears(int Yrs)
{
	if(!IsValid())
		return *this;
	m_systemTime.wYear-=Yrs;
	AdjustDays();
	return *this;

}
//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::AddDays
// Description	    : 
// Return type		: const CSimpleDateTime& 
// Argument         : int Days
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::setHHMM(LPCWSTR strHHMM)
{
	SYSTEMTIME st;
	memset(&st,0,sizeof(SYSTEMTIME));
	LONG wTime = _wtol(strHHMM);
	st.wHour=(WORD)(wTime / 100);
	st.wMinute=(WORD)(wTime % 100);
	m_systemTime=st;
	return *this;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::AddDays
// Description	    : 
// Return type		: const CSimpleDateTime& 
// Argument         : int Days
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::AddDays(int Days)
{
	if(!IsValid())
		return *this;
	m_systemTime = this->DT_AddDays(this->m_systemTime, Days);
	return *this;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::GetNextSchedule
// Description	    : 
// Return type		: const CSimpleDateTime& 
// Argument         : CSimpleDateTime start
// Argument         : int time, ie 0130 for 1:30 clock
// Argument         : int interval, ie 2430 for an interval of 24 hours and 30 minutes
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::GetNextSchedule(SYSTEMTIME start, UINT time, UINT interval){
	if(!IsValid())
		return *this;
	short shStartHour = time/100;		//0130 ->	 1 hour
	short shStartMinute = time % 100;	//0130 ->	30 minutes

	short shHours = interval/100;		//2430 ->	24 hours
	shHours=shHours % 24;				//2430 ->	 0 hours
	short shDays = interval/100 / 24;		//2430 ->	 1 day
	short shMinutes = interval % 100;	//2430 ->	30 minutes

	m_systemTime = this->DT_Add(start, 0, 0, shDays, shHours, shMinutes, 0, 0);
	return *this;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::AddDay
// Description	    : 
// Return type		: const CSimpleDateTime& 
// Argument         : int Days
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::AddDay()
{
	if(!IsValid())
		return *this;
	m_systemTime = this->DT_AddDays(this->m_systemTime, 1);
	return *this;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::SubtractDays
// Description	    : 
// Return type		: const CSimpleDateTime& 
// Argument         : int Days
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::SubtractDays(int Days)
{
	if(!IsValid())
		return *this;
	m_systemTime = DT_AddDays(m_systemTime, -Days);
	return *this;

}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::AdjustDays
// Description	    : 
// Return type		: void 
//--------------------------------------------------------------------
void CSimpleDateTime::AdjustDays()
{
   int is_leap, month_days ;
   is_leap =  ( m_systemTime.wYear%4 == 0 && m_systemTime.wYear%100 != 0 || m_systemTime.wYear%400 == 0 ) ?  1 : 0 ;
   month_days = monthtotals[ m_systemTime.wMonth+1 ] -  monthtotals[ m_systemTime.wMonth] ;
   if ( m_systemTime.wMonth == 2 )  month_days += is_leap ;
	while(m_systemTime.wDay   > month_days)
		--m_systemTime.wDay;

}

//--------------------------------------------------------------------
//	Time functions
//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::SetTime
// Description	    : The only time function: Sets the variables for later use
// Return type		: void 
//--------------------------------------------------------------------
void CSimpleDateTime::SetTime()
{
	GetLocalTime(&m_systemTime);
}


//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::GetTimeString
// Description	    : 
// Return type		: void 
// Argument         : CString& s
// Argument         : BOOL AmPm
//--------------------------------------------------------------------
//void CSimpleDateTime::GetTimeString(CString& s,BOOL AmPm)
//{
//	if(!IsValid())
//	{
//		s.Empty();
//		return;
//	}
//	SetTime();
//	int Flag=0;
//	if(AmPm==TRUE)
//	{
//		if(m_bPM)
//			Flag=1;
//		else
//			Flag=2;
//	}
//	s.Format("%02d:%02d:%02d   %s",m_systemTime.wHour==0 ? 12:m_systemTime.wHour,m_systemTime.wMinute,m_Second,Flag==0 ? "":Flag==1 ? "PM":"AM");
//}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::GetTimeString
// Description	    : 
// Return type		: void 
// Argument         : LPSTR s
// Argument         : int nLen
// Argument         : BOOL AmPm
//--------------------------------------------------------------------
//void CSimpleDateTime::GetTimeString(LPSTR s,int nLen,BOOL AmPm)
//{
//	CString cs;
//	GetTimeString(cs,AmPm);
//	strncpy(s,cs,nLen);
//}


//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::GetTimeStringShort
// Description	    : 
// Return type		: void 
// Argument         : CString& s
// Argument         : BOOL AmPm
//--------------------------------------------------------------------
//void CSimpleDateTime::GetTimeStringShort(CString& s,BOOL AmPm)
//{
//	if(!IsValid())
//	{
//		s.Empty();
//		return;
//	}
//	SetTime();
//	int Flag=0;
//	if(AmPm==TRUE)
//	{
//		if(m_bPM)
//			Flag=1;
//		else
//			Flag=2;
//	}
//	s.Format("%02d:%02d %s",m_systemTime.wHour==0 ? 12:m_systemTime.wHour,m_systemTime.wMinute,Flag==0 ? "":Flag==1 ? "PM":"AM");
//}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::GetTimeStringShort
// Description	    : 
// Return type		: void 
// Argument         : LPSTR s
// Argument         : int nLen
// Argument         : BOOL AmPm
//--------------------------------------------------------------------
//void CSimpleDateTime::GetTimeStringShort(LPSTR s,int nLen,BOOL AmPm)
//{
//	CString cs;
//	GetTimeStringShort(cs,AmPm);
//	strncpy(s,cs,nLen);
//}


//--------------------------------------------------------------------
//	Static member functions
//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::FixDateFormat
// Description	    : 
// Return type		: BOOL 
// Argument         : LPSTR str
//--------------------------------------------------------------------
//BOOL CSimpleDateTime::FixDateFormat(LPSTR str)
//{
//	CString	s;
//	s=str;
//	if(FixDateFormat(s))
//	{
//		strcpy(str,s);
//		return TRUE;
//	}
//	return FALSE;
//}
//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::FixDateFormat
// Description	    : 
// Return type		: BOOL 
// Argument         : CString& str
//--------------------------------------------------------------------
//BOOL CSimpleDateTime::FixDateFormat(CString& str)
//{
//	char	Buff[20];
//
//	if(str.IsEmpty())
//		return FALSE;
//	if(str.GetLength() < 6 || str.GetLength() > 11)
//		return FALSE;		
//	CParseIt ParseIt(str,"/.-");      
//	ParseIt.Parse();
//	int N=ParseIt.GetNumFields();
//	if(N > 1)
//		return FALSE;
//	strcpy(Buff,str.Left(2));
//	strcat(Buff,"/");
//	strcat(Buff,str.Mid(2,2));
//	strcat(Buff,"/");		
//	if(str.GetLength()==6)
//		strcat(Buff,str.Right(2));
//	else
//		strcat(Buff,str.Right(str.GetLength()-4));		
//	str=Buff;		
//	return TRUE;
//}
//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::VerifyDateFormat
// Description	    : 
// Return type		: BOOL 
// Argument         : LPCWSTR date
//--------------------------------------------------------------------
//BOOL CSimpleDateTime::VerifyDateFormat(LPCWSTR date)
//{
//	BOOL	HasSeperators=FALSE;
//	CString TheDate;
//	//	allow blank dates
//	if(strlen(date)==0)
//		return TRUE;
//	//	shortest possible date
//	if(strlen(date) < 6)
//		return FALSE;
//	//	look for unacceptable characters
//	for(int x=0;x < (signed)strlen(date);++x)           
//	{
//		if( date[x] !='/' && date[x] !='-' && date[x] !='.' && !isdigit(date[x]))
//		   	return FALSE;
//	}		   	
//	for(x=0;x < (signed)strlen(date);++x)           
//	{
//		if( date[x] =='/' || date[x] =='-' || date[x] =='.')
//		   	HasSeperators=TRUE;
//	}		   	
//	TheDate=date;
//	if(!HasSeperators)
//	{
//		if(!FixDateFormat(TheDate))
//			return FALSE;
//	}
//	CSimpleDateTime s(TheDate);
//	return s.IsValid();		
//}

