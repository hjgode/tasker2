// SimpleDateTime.h: interface for the CSimpleDateTime class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLEDATE_H__3DD52FF4_4E78_11D3_82D6_00A0CC28BFE2__INCLUDED_)
#define AFX_SIMPLEDATE_H__3DD52FF4_4E78_11D3_82D6_00A0CC28BFE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


enum{
	MMDDYY,
	DDMMYY,
	YYMMDD,
	MMDDYYYY,
	DDMMYYYY,
	YYYYMMDD,
	DDMMYYYYhhmm,
	hhmm
};

// NOTE:: If you choose any of the two digit year formats you had better be sure all
//		  dates will be in the range of 1900-1999 else there will be unexpected results


//#include "Parseit.h"

class CSimpleDateTime  
{
public:
	//	constructors and destructors
	CSimpleDateTime(int FormatType=MMDDYYYY);
	CSimpleDateTime(LPCWSTR DateString, int FormatType=MMDDYYYY);
	CSimpleDateTime(long JD,int FormatType=MMDDYYYY);
	CSimpleDateTime(SYSTEMTIME systemTime);
	
	//CSimpleDateTime(LPCWSTR DateString);
	
	virtual ~CSimpleDateTime();

	//	Date math routines
	const	CSimpleDateTime& AddDays(int Days);
	const	CSimpleDateTime& AddYears(int Yrs);
	const	CSimpleDateTime& AddMonths(int Mon);
	const	CSimpleDateTime& SubtractYears(int Yrs);
	const	CSimpleDateTime& SubtractDays(int Days);
	const	CSimpleDateTime& SubtractMonths(int Mon);
	virtual int			YearsOld();

	// access routines;
	LPCWSTR		GetFullDateString();
	LPCWSTR		GetFullDateStringLong();
	virtual		int GetDayOfWeek();
	virtual		BOOL IsValid();
	long		GetJulianDate(); // easier to use (long)CSimpelDate but some prefer this

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
		return GetFullDateString();
	}
	operator	long();
	const CSimpleDateTime& operator = (const CSimpleDateTime& Date);
	const CSimpleDateTime& operator = (LPCWSTR Date);

	const CSimpleDateTime& operator + (LPCWSTR Date);
	const CSimpleDateTime& operator + (SYSTEMTIME Date);

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
	virtual	BOOL	SetToday();
	virtual	BOOL	SetNow();
	virtual	BOOL	CSimpleDateTime::ParseDateString(LPCWSTR,WORD& m,WORD& d,WORD& y);
	virtual BOOL	CSimpleDateTime::ParseDateString(LPCWSTR date,WORD& m,WORD& d,WORD& y, WORD& hour, WORD& min);
	//virtual	BOOL	ParseDateString(LPCWSTR);
	virtual	BOOL	ParseDateTimeString(LPCWSTR TheDateTime);
	virtual	long	ConvertToJulian( WORD month, WORD day, WORD year);
	virtual	long	ConvertToJulian();
	virtual	void	ConvertFromJulian(WORD& Month, WORD& Day, WORD& Year);
	virtual void	ConvertFromJulian();
	virtual void	AdjustDays();
	virtual void	SetTime();
	virtual long	getLong(SYSTEMTIME sysTime);

	//	STATIC MEMBER FUNCTIONS
public:  
	
	// the static functions assume a date format of MMDDYY or MMDDYYYY
	//	They also do not call this IsValid() function
	static	BOOL	VerifyDateFormat(LPCWSTR date);
	//static	BOOL	FixDateFormat(CString & date);
	static	BOOL	FixDateFormat(LPSTR date);
	

	//	class data
protected:
	SYSTEMTIME	m_systemTime;

	long		m_JulianDate;
	int			m_Format;
	TCHAR		m_DateString[MAX_PATH];
};

#endif // !defined(AFX_SIMPLEDATE_H__3DD52FF4_4E78_11D3_82D6_00A0CC28BFE2__INCLUDED_)
