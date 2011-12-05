// SimpleDate.h: interface for the CSimpleDate class.
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
};

// NOTE:: If you choose any of the two digit year formats you had better be sure all
//		  dates will be in the range of 1900-1999 else there will be unexpected results


//#include "Parseit.h"

class CSimpleDate  
{
public:
	//	constructors and destructors
	CSimpleDate(int FormatType=MMDDYYYY);
	CSimpleDate(LPCSTR DateString,int FormatType=MMDDYYYY);
	CSimpleDate(long JD,int FormatType=MMDDYYYY);
	virtual ~CSimpleDate();

	//	Date math routines
	const	CSimpleDate& AddDays(int Days);
	const	CSimpleDate& AddYears(int Yrs);
	const	CSimpleDate& AddMonths(int Mon);
	const	CSimpleDate& SubtractYears(int Yrs);
	const	CSimpleDate& SubtractDays(int Days);
	const	CSimpleDate& SubtractMonths(int Mon);
	virtual int			YearsOld();

	// access routines;
	LPCSTR		GetFullDateString();
	LPCSTR		GetFullDateStringLong();
	virtual		int GetDayOfWeek();
	virtual		BOOL IsValid();
	long		GetJulianDate(); // easier to use (long)CSimpelDate but some prefer this

	virtual		int	GetDay()
					{if(!IsValid()) return 0;return m_Day;};
	virtual		int	GetMonth()
					{if(!IsValid()) return 0;return m_Month;};
	virtual		int	GetYear()
					{if(!IsValid()) return 0;return m_Year;};
	virtual		void	GetIntegerDate(int& m, int& d,int& y)
					{if(!IsValid()) return;m=m_Month;y=m_Year;d=m_Day;};
	virtual		int		GetHour()
					{if(!IsValid()) return 0;return m_Hour+m_bPM*12;};
	virtual		int		GetMin()
					{if(!IsValid()) return 0;return m_Min;};
	virtual		int		GetSeconds()
					{if(!IsValid()) return 0;return m_Second;};

	//virtual		void	GetTimeString(CString& s,BOOL AmPm=TRUE);
	//virtual		void	GetTimeString(LPSTR s,int nLen,BOOL AmPm=TRUE);
	//virtual		void	GetTimeStringShort(CString& s,BOOL AmPm=TRUE);
	//virtual		void	GetTimeStringShort(LPSTR s,int nLen,BOOL AmPm=TRUE);

	//	operator assignments conversions equality etc...
	operator	LPCSTR();
	operator	long();
	const CSimpleDate& operator = (const CSimpleDate& Date);
	const CSimpleDate& operator = (LPCSTR Date);
	BOOL operator > (const CSimpleDate& Date);
	BOOL operator < (const CSimpleDate& Date);
	BOOL operator >= (const CSimpleDate& Date);
	BOOL operator <= (const CSimpleDate& Date);
	BOOL operator == (const CSimpleDate& Date);
	BOOL operator != (const CSimpleDate& Date);
	BOOL operator > (LPCSTR Date);
	BOOL operator < (LPCSTR Date);
	BOOL operator >= (LPCSTR Date);
	BOOL operator <= (LPCSTR Date);
	BOOL operator == (LPCSTR Date);
	BOOL operator != (LPCSTR Date);



protected:
	//	internal class stuff
	virtual	BOOL	SetToday();
	virtual	BOOL	CSimpleDate::ParseDateString(LPCSTR,int& m,int& d,int& y);
	virtual	BOOL	ParseDateString(LPCSTR);
	virtual	long	ConvertToJulian( int month,int day,int year);
	virtual	long	ConvertToJulian();
	virtual	void	ConvertFromJulian(int& Month,int& Day,int& Year);
	virtual void	ConvertFromJulian();
	virtual void	AdjustDays();
	virtual void SetTime();

	//	STATIC MEMBER FUNCTIONS
public:  
	
	// the static functions assume a date format of MMDDYY or MMDDYYYY
	//	They also do not call this IsValid() function
	static	BOOL	VerifyDateFormat(LPCSTR date);
	//static	BOOL	FixDateFormat(CString & date);
	static	BOOL	FixDateFormat(LPSTR date);
	

	//	class data
protected:
	int			m_Year;
	int			m_Month;
	int			m_Day;
	long		m_JulianDate;
	int			m_Format;
	//CString		m_DateString;

	int			m_Hour;
	int			m_Min;
	int			m_Second;
	BOOL		m_bPM;

};

#endif // !defined(AFX_SIMPLEDATE_H__3DD52FF4_4E78_11D3_82D6_00A0CC28BFE2__INCLUDED_)
