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
CSimpleDateTime::CSimpleDateTime(int FormatType)
{
	m_Format=FormatType;
	SetToday();
	m_JulianDate=ConvertToJulian();	
	SetTime();
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::CSimpleDateTime
// Description	    : paramaterized constructor object is set to date in systemTime.
// Return type		: 
// Argument         : SYSTEMTIME systemTime
//--------------------------------------------------------------------
CSimpleDateTime::CSimpleDateTime(SYSTEMTIME systemTime)
{
	m_systemTime = systemTime;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::CSimpleDateTime
// Description	    : paramaterized constructor object is set to date in string. Parsing will be done based on formattype
// Return type		: 
// Argument         : LPCWSTR DateString
//--------------------------------------------------------------------

CSimpleDateTime::CSimpleDateTime(LPCWSTR DateString,int FormatType)
{
	m_Format=FormatType;
	ParseDateTimeString(DateString);
	m_JulianDate=ConvertToJulian();	
	SetTime();
}



//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::CSimpleDateTime
// Description	    : 
// Return type		: 
// Argument         : long JD
// Argument         : int FormatType
//--------------------------------------------------------------------
CSimpleDateTime::CSimpleDateTime(long JD,int FormatType)
{
	m_Format=FormatType;
	m_JulianDate=JD;	
	ConvertFromJulian();  // note: m_JulianDate must be set prior to this call
	SetTime();
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
// Argument         : LPCWSTR TheDate
//--------------------------------------------------------------------
//BOOL CSimpleDateTime::ParseDateString(LPCWSTR TheDate)
//{
//	ParseDateString(TheDate, m_systemTime.wMonth, m_systemTime.wDay, m_systemTime.wYear);
//	return TRUE;
//}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::ParseDateString
// Description	    : 
// Return type		: BOOL 
// Argument         : LPCWSTR TheDate
//--------------------------------------------------------------------
BOOL CSimpleDateTime::ParseDateTimeString(LPCWSTR TheDateTime)
{
	ParseDateString(TheDateTime, m_systemTime.wMonth, m_systemTime.wDay, m_systemTime.wYear, m_systemTime.wHour, m_systemTime.wMinute);
	return TRUE;
}

BOOL CSimpleDateTime::ParseDateString(LPCWSTR date,WORD& m, WORD& d, WORD& y, WORD& hour, WORD& min)
{
	BOOL bRet=TRUE; 
	int iErrCnt=0;
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
// Function name	: CSimpleDateTime::ParseDateString
// Description	    : See CParseIt class in Parseit.cpp for details
// Return type		: BOOL 
// Argument         : LPCWSTR date
// Argument         : int& m
// Argument         : int& d
// Argument         : int& y
//--------------------------------------------------------------------
//BOOL CSimpleDateTime::ParseDateString(LPCWSTR date,int& m,int& d,int& y)
//{
//	CParseIt ParseIt(date,"/.-");      
//	ParseIt.Parse();
//	int	one,two,three;           
//	one=two=three=0;
//	int N=ParseIt.GetNumFields();
//	one=(int)ParseIt.GetField(1);	
//	if(N > 1)
//		two=(int)ParseIt.GetField(2);	
//	if(N > 2)
//		three=(int)ParseIt.GetField(3);	
//	switch(m_Format)
//	{
//	case MMDDYY:
//		m=one;d=two;y=three;
//		if(y < 100)     // ajust two digit year to the range of 1900-1999 
//			y+=1900;
//		break;
//	case DDMMYY:
//		d=one;m=two;y=three;
//		if(y < 100)
//			y+=1900;
//		break;
//	case YYMMDD:
//		y=one;m=two;d=three;
//		if(y < 100)
//			y+=1900;
//		break;
//	case MMDDYYYY:
//		m=one;d=two;y=three;
//		if(y < 100)
//			y+=1900;
//		break;
//	case DDMMYYYY:
//		d=one;m=two;y=three;
//		if(y < 100)
//			y+=1900;
//		break;
//	case YYYYMMDD:
//		y=one;m=two;d=three;
//		if(y < 100)
//			y+=1900;
//		break;
//	default:
//		m=y=d=0;
//	}
//
//	return TRUE;
//
//}
//
//--------------------------------------------------------------------
//	JULIAN DATE CONVERSION BELOW
//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::ConvertToJulian
// Description	    : 
// Return type		: long 
//--------------------------------------------------------------------
long CSimpleDateTime::ConvertToJulian()
{
	return ConvertToJulian(m_systemTime.wMonth,m_systemTime.wDay,m_systemTime.wYear);

}
//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::ConvertToJulian
// Description	    : go to http://www.capecod.net/~pbaum/date/date0.htm for details
// Return type		: long 
// Argument         :  int month
// Argument         : int day
// Argument         : int year
//--------------------------------------------------------------------
long CSimpleDateTime::ConvertToJulian( WORD m, WORD d, WORD y)
{
	if(m < 3)
	{
      m = m + 12;
      y=y-1 ;
	}
	long jd = d + (153 * m - 457) / 5 + 365 * y + (y / 4) - (y / 100) + (y / 400) + 1721119;
	return jd;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::ConvertFromJulian
// Description	    : 
// Return type		: void 
//--------------------------------------------------------------------
void CSimpleDateTime::ConvertFromJulian()
{
	ConvertFromJulian(m_systemTime.wMonth, m_systemTime.wDay, m_systemTime.wYear);
}
//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::ConvertFromJulian
// Description	    : goto http://www.capecod.net/~pbaum/date/date0.htm for details
// Return type		: void 
// Argument         : int Month
// Argument         : int DAy
// Argument         : int Year
//--------------------------------------------------------------------
void CSimpleDateTime::ConvertFromJulian(WORD& Month, WORD& Day, WORD& Year)
{

	long L = m_JulianDate + 68569;
    long N = (long) ((4*L)/146097) ;
    L = L - ((long)((146097 * N + 3)/4) );
    long I = (long) ((4000 *(L + 1)/1461001)) ;
    L = L - (long)((1461*I)/4) + 31; 
    long J = (long)((80*L)/2447); 
    Day = L - (long)((2447*J)/80);
    L=(long)(J/11) ;
    Month = J + 2 - 12*L;
    Year = 100*(N-49) + I + L ;
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
	int res= ((int) (m_JulianDate + 1.5)) % 7;
	return res;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::GetFullDateString
// Description	    : 
// Return type		: LPCWSTR 
//--------------------------------------------------------------------
LPCWSTR CSimpleDateTime::GetFullDateString()
{
	if(!IsValid())
		return NULL;
	wsprintf(m_DateString, L"%s %s %02d %04d", daysofweek[GetDayOfWeek()],monthsofyear[m_systemTime.wMonth],m_systemTime.wDay,m_systemTime.wYear);
	//m_DateString.Format("%s %s %02d %04d", daysofweek[GetDayOfWeek()],monthsofyear[m_systemTime.wMonth],m_systemTime.wDay,m_systemTime.wYear);
	return m_DateString;
}

//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::GetFullDateStringLong
// Description	    : 
// Return type		: LPCWSTR 
//--------------------------------------------------------------------
LPCWSTR CSimpleDateTime::GetFullDateStringLong()
{
	if(!IsValid())
		return NULL;
	wsprintf(m_DateString, L"%s %s %02d %04d", days_of_week[GetDayOfWeek()],months_of_year[m_systemTime.wMonth],m_systemTime.wDay,m_systemTime.wYear);
	//m_DateString.Format("%s %s %02d %04d", days_of_week[GetDayOfWeek()],months_of_year[m_systemTime.wMonth],m_systemTime.wDay,m_systemTime.wYear);
	return m_DateString;
}


//--------------------------------------------------------------------
// Function name	: CSimpleDateTime::GetJulianDate
// Description	    : 
// Return type		: long 
//--------------------------------------------------------------------
long CSimpleDateTime::GetJulianDate()
{
	if(!IsValid())
		return 0L;
	return m_JulianDate;
}


//--------------------------------------------------------------------
//	overloaded operators and copy constructors here
//--------------------------------------------------------------------
// Function name	: LPCWSTR
// Description	    : 
// Return type		: CSimpleDateTime::operator return LPCWSTR
//--------------------------------------------------------------------
//CSimpleDateTime::operator LPCWSTR()
//{
//	if(!IsValid())
//		return (LPCWSTR)0;
//	switch(m_Format)
//	{
//	case MMDDYY:
//		m_DateString.Format("%02d/%02d/%02d",m_systemTime.wMonth,m_systemTime.wDay,m_systemTime.wYear > 99 ? m_systemTime.wYear-1900:m_systemTime.wYear);		
//		return (LPCWSTR)m_DateString;
//	case DDMMYY:
//		m_DateString.Format("%02d/%02d/%02d",m_systemTime.wDay,m_systemTime.wMonth,m_systemTime.wYear > 99 ? m_systemTime.wYear-1900:m_systemTime.wYear);		
//		return (LPCWSTR)m_DateString;
//	case YYMMDD:
//		m_DateString.Format("%02d/%02d/%02d",m_systemTime.wYear > 99 ? m_systemTime.wYear-1900:m_systemTime.wYear,m_systemTime.wMonth,m_systemTime.wDay);		
//		return (LPCWSTR)m_DateString;
//	case MMDDYYYY:
//		m_DateString.Format("%02d/%02d/%04d",m_systemTime.wMonth,m_systemTime.wDay,m_systemTime.wYear );		
//		return (LPCWSTR)m_DateString;
//	case DDMMYYYY:
//		m_DateString.Format("%02d/%02d/%04d",m_systemTime.wDay,m_systemTime.wMonth,m_systemTime.wYear);		
//		return (LPCWSTR)m_DateString;
//	case YYYYMMDD:
//		m_DateString.Format("%04d/%02d/%02d",m_systemTime.wYear ,m_systemTime.wMonth,m_systemTime.wDay);		
//		return (LPCWSTR)m_DateString;
//	default:
//		return (LPCWSTR)NULL;
//	}
//}


//--------------------------------------------------------------------
// OPERATORS
//--------------------------------------------------------------------
// Function name	: long
// Description	    : 
// Return type		: CSimpleDateTime::operator return long
//--------------------------------------------------------------------
CSimpleDateTime::operator long()
{
	if(!IsValid())
		return 0L;
	return m_JulianDate;
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
	m_JulianDate=Date.m_JulianDate;
	m_Format=Date.m_Format;
	return *this;
}

// save inTime, get LocalTime and then set DAY, hour and minute of inTime
// added support for negaitive days, hours and minutes in v2.34
SYSTEMTIME CSimpleDateTime::DT_Add(SYSTEMTIME& Date, short Years, short Months, short Days, short Hours, short Minutes, short Seconds, short Milliseconds) {
	FILETIME ft; SYSTEMTIME st; ULARGE_INTEGER ul1;
	
	SYSTEMTIME inTime;
	//v2.28 GetLocalTime(&inTime); //actual time and date
	extern SYSTEMTIME g_CurrentStartTime;
	memcpy(&inTime, &g_CurrentStartTime, sizeof(SYSTEMTIME));
	inTime.wDay = Date.wDay;
	inTime.wHour = Date.wHour;
	inTime.wMinute = Date.wMinute;
	inTime.wSecond=0;
	inTime.wMilliseconds=0;

	memcpy((void*)&Date, &inTime, sizeof(SYSTEMTIME));

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
	m_JulianDate=ConvertToJulian();	
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
	m_JulianDate=ConvertToJulian();	
	return  *this;
}

//--------------------------------------------------------------------
// Function name	: =
// Description	    : 
// Return type		: CSimpleDateTime::operator 
//--------------------------------------------------------------------
const CSimpleDateTime& CSimpleDateTime::operator =(LPCWSTR DateTime)
{
	ParseDateTimeString(DateTime);
	m_JulianDate=ConvertToJulian();	
	return *this;
}

//--------------------------------------------------------------------
LONG CSimpleDateTime::getLong(SYSTEMTIME sysTime){
	LONG lDateTime =  m_systemTime.wYear	*1000000000 
					+ m_systemTime.wMonth	*10000000 
					+ m_systemTime.wDay		*100000
					+ m_systemTime.wHour	*1000
					+ m_systemTime.wMinute	*10
					+ m_systemTime.wSecond;
	return lDateTime;
}

//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator > (const CSimpleDateTime& Date)	
{                        
	if(!IsValid())
		return FALSE;
	return getLong(m_systemTime) > getLong(Date.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator >= (const CSimpleDateTime& Date)	
{                        
	if(!IsValid())
		return FALSE;
	return getLong(m_systemTime) >= getLong(Date.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator < (const CSimpleDateTime& Date)	
{                        
	if(!IsValid())
		return FALSE;
	return getLong(m_systemTime) < getLong(Date.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator <= (const CSimpleDateTime& Date)	
{                        
	if(!IsValid())
		return FALSE;
	return getLong(m_systemTime) <= getLong(Date.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator == (const CSimpleDateTime& Date)	
{                        
	if(!IsValid())
		return FALSE;
	return getLong(m_systemTime) == getLong(Date.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator != (const CSimpleDateTime& Date)	
{                        
	if(!IsValid())
		return FALSE;
	return getLong(m_systemTime) != getLong(Date.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator > (LPCWSTR Date)	
{                        
	if(!IsValid())
		return FALSE;
	CSimpleDateTime TheDate(Date);
	if(!TheDate.IsValid())
		return FALSE;
	return getLong(m_systemTime) > getLong(TheDate.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator < (LPCWSTR Date)	
{                        
	if(!IsValid())
		return FALSE;
	CSimpleDateTime TheDate(Date);
	if(!TheDate.IsValid())
		return FALSE;
	return getLong(m_systemTime) < getLong(TheDate.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator >= (LPCWSTR Date)	
{                        
	if(!IsValid())
		return FALSE;
	CSimpleDateTime TheDate(Date);
	if(!TheDate.IsValid())
		return FALSE;
	return getLong(m_systemTime) >= getLong(TheDate.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator <= (LPCWSTR Date)	
{                        
	if(!IsValid())
		return FALSE;
	CSimpleDateTime TheDate(Date);
	if(!TheDate.IsValid())
		return FALSE;
	return getLong(m_systemTime) <= getLong(TheDate.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator == (LPCWSTR Date)	
{                        
	if(!IsValid())
		return FALSE;
	CSimpleDateTime TheDate(Date);
	if(!TheDate.IsValid())
		return FALSE;
	return getLong(m_systemTime) == getLong(TheDate.m_systemTime);
}			
//--------------------------------------------------------------------
BOOL CSimpleDateTime::operator != (LPCWSTR Date)	
{                        
	if(!IsValid())
		return FALSE;
	CSimpleDateTime TheDate(Date);
	if(!TheDate.IsValid())
		return FALSE;
	return getLong(m_systemTime) != getLong(TheDate.m_systemTime);
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
	long t=ToDay.GetJulianDate();
	long b=GetJulianDate();
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
   	m_JulianDate=ConvertToJulian();
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
   	m_JulianDate=ConvertToJulian();
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
	m_JulianDate=ConvertToJulian();
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
	m_JulianDate=ConvertToJulian();
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
	m_JulianDate+=Days;
	ConvertFromJulian(m_systemTime.wMonth,m_systemTime.wDay,m_systemTime.wYear);
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
	m_JulianDate-=Days;
	ConvertFromJulian(m_systemTime.wMonth,m_systemTime.wDay,m_systemTime.wYear);
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

