// SimpleDateTime.h: interface for the CSimpleDateTime class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLEDATE_H__3DD52FF4_4E78_11D3_82D6_00A0CC28BFE2__INCLUDED_)
#define AFX_SIMPLEDATE_H__3DD52FF4_4E78_11D3_82D6_00A0CC28BFE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


enum formatTypes{
	MMDDYY,
	DDMMYY,
	YYMMDD,
	MMDDYYYY,
	DDMMYYYY,
	YYYYMMDD,
	DDMMYYYYhhmm,
	YYYYMMDDhhmm,
	hhmm
};

// NOTE:: If you choose any of the two digit year formats you had better be sure all
//		  dates will be in the range of 1900-1999 else there will be unexpected results


//#include "Parseit.h"

class CSimpleDateTime  
{
public:
	//	constructors and destructors
	CSimpleDateTime(int FormatType=YYYYMMDDhhmm);
	CSimpleDateTime(LPCWSTR DateString, int FormatType=MMDDYYYY);
	CSimpleDateTime(SYSTEMTIME systemTime);
	
	//CSimpleDateTime(LPCWSTR DateString);
	
	virtual ~CSimpleDateTime();

	//	Date math routines
	const	CSimpleDateTime& AddDays(int Days);
	const	CSimpleDateTime& GetNextSchedule(SYSTEMTIME start, UINT time, UINT interval);

	const	CSimpleDateTime& AddDay();
	const	CSimpleDateTime& AddYears(int Yrs);
	const	CSimpleDateTime& AddMonths(int Mon);
	const	CSimpleDateTime& SubtractYears(int Yrs);
	const	CSimpleDateTime& SubtractDays(int Days);
	const	CSimpleDateTime& SubtractMonths(int Mon);
	virtual int			YearsOld();

	// access routines;
	LPCWSTR		GetFullDateString();
	LPCWSTR		GetDateTimeString();
	LPCWSTR		GetFullDateStringLong();

	virtual		int GetDayOfWeek();
	virtual		BOOL IsValid();

	virtual		int		GetDay()
								{if(!IsValid()) return 0;return m_systemTime.wDay;};
	virtual		int		GetMonth()
								{if(!IsValid()) return 0;return m_systemTime.wMonth;};
	virtual		int		GetYear()
								{if(!IsValid()) return 0;return m_systemTime.wYear;};
	virtual		void	GetIntegerDate(int& m, int& d,int& y)
								{if(!IsValid()) return;m=m_systemTime.wMonth;y=m_systemTime.wYear;d=m_systemTime.wDay;};
	virtual		int		GetHour()
								{if(!IsValid()) return 0;return m_systemTime.wHour;};
	virtual		int		GetMin()
								{if(!IsValid()) return 0;return m_systemTime.wMinute;};
	virtual		int		GetSeconds()
								{if(!IsValid()) return 0;return m_systemTime.wSecond;};
	virtual		SYSTEMTIME		GetSystemTime()
				{
					if(!IsValid()){
						SYSTEMTIME st;
						memset(&st, 0, sizeof(SYSTEMTIME));
						return st;
					}
					return m_systemTime;
				};
	
	//virtual		void	GetTimeString(CString& s,BOOL AmPm=TRUE);
	//virtual		void	GetTimeString(LPSTR s,int nLen,BOOL AmPm=TRUE);
	//virtual		void	GetTimeStringShort(CString& s,BOOL AmPm=TRUE);
	//virtual		void	GetTimeStringShort(LPSTR s,int nLen,BOOL AmPm=TRUE);

	//	operator assignments conversions equality etc...
	operator	LPCWSTR(){
		return GetDateTimeString();
	}
	operator TCHAR(){
		return (TCHAR)GetDateTimeString();
	}
//	operator	long();	//long cannot store values like 201103152322 2011 03 15 23:22
	operator	DOUBLE();
	const CSimpleDateTime& operator = (const CSimpleDateTime& Date);
	const CSimpleDateTime& operator = (LPCWSTR Date);

	const CSimpleDateTime& operator + (LPCWSTR Date);
	const CSimpleDateTime& operator + (SYSTEMTIME Date);
	const CSimpleDateTime& operator + (CSimpleDateTime& Date);

	BOOL operator > (const CSimpleDateTime& Date);
	BOOL operator < (const CSimpleDateTime& Date);
	BOOL operator >= (const CSimpleDateTime& Date);
	BOOL operator <= (const CSimpleDateTime& Date);
	BOOL operator == (const CSimpleDateTime& Date);
	BOOL operator != (const CSimpleDateTime& Date);
	BOOL operator > (LPCWSTR Date);
	BOOL operator < (LPCWSTR Date);
	BOOL operator >= (LPCWSTR Date);
	BOOL operator <= (LPCWSTR Date);
	BOOL operator == (LPCWSTR Date);
	BOOL operator != (LPCWSTR Date);



protected:
	//	internal class stuff
	SYSTEMTIME		DT_Add(SYSTEMTIME& Date, short Years, short Months, short Days, short Hours, short Minutes, short Seconds, short Milliseconds);
	SYSTEMTIME		DT_AddDay(const SYSTEMTIME st);
	SYSTEMTIME		DT_AddDiff (	const __int64 datepart, /*datepart with we want to manipulate, 
					{nano100SecInDay ...}*/
					const __int64 num, /*value used to increment/decrement datepart*/
					const SYSTEMTIME* pst /*valid datetime which we want change*/
					);
	virtual	BOOL	SetToday();
	virtual	BOOL	SetNow();

	virtual	SYSTEMTIME		CSimpleDateTime::DT_AddDays(const SYSTEMTIME st, int days);

	virtual	BOOL	CSimpleDateTime::ParseDateString(LPCWSTR,WORD& m,WORD& d,WORD& y);
	virtual BOOL	CSimpleDateTime::ParseDateString(LPCWSTR date,WORD& m,WORD& d,WORD& y, WORD& hour, WORD& min);
	//virtual	BOOL	ParseDateString(LPCWSTR);
	virtual	BOOL	ParseDateTimeString(LPCWSTR TheDateTime);
	//virtual	long	ConvertToJulian( WORD month, WORD day, WORD year);
	//virtual	long	ConvertToJulian();
	//virtual	void	ConvertFromJulian(WORD& Month, WORD& Day, WORD& Year);
	//virtual void	ConvertFromJulian();
	virtual void	AdjustDays();
	virtual void	SetTime();
//	virtual long	getLong(SYSTEMTIME sysTime);
	virtual DOUBLE	getDouble(SYSTEMTIME sysTime);

	//	STATIC MEMBER FUNCTIONS
public:  
	
	// the static functions assume a date format of MMDDYY or MMDDYYYY
	//	They also do not call this IsValid() function
	static	BOOL	VerifyDateFormat(LPCWSTR date);
	//static	BOOL	FixDateFormat(CString & date);
	static	BOOL	FixDateFormat(LPSTR date);
	
	TCHAR		m_DateString[MAX_PATH];

	//	class data
protected:
	SYSTEMTIME	m_systemTime;

//	long		m_JulianDate;
	int			m_Format;
	static const __int64 nano100SecInDay=(__int64)10000000*60*60*24;
	static const __int64 nano10Minutes=(__int64)10000000*60*10;
};

#endif // !defined(AFX_SIMPLEDATE_H__3DD52FF4_4E78_11D3_82D6_00A0CC28BFE2__INCLUDED_)
