#pragma once
// DuiCalendar.h : header file
/////////////////////////////////////////////////////////////////////////////

#include "duiwnd.h"

namespace SOUI
{
#define START_YEAR	1901
#define END_YEAR	2050

	class CCalendarCore{
	public:
		//判断iYear是不是闰年
		static BOOL IsLeapYear(WORD iYear)
		{return !(iYear%4)&&(iYear%100) || !(iYear%400);}

		//计算iYear,iMonth,iDay对应是星期几 1年1月1日 --- 65535年12月31日
		static WORD WeekDay(WORD iYear, WORD iMonth, WORD iDay);

		//计算出指定月份的周数
		static WORD MonthWeeks(WORD iYear, WORD iMonth);

		//计算指定天是该月的第几周
		static WORD DayWeek(WORD iYear, WORD iMonth, WORD iDay);

		//返回iYear年iMonth月的天数 1年1月 --- 65535年12月
		static WORD MonthDays(WORD iYear, WORD iMonth);

		//返回阴历iLunarYer年阴历iLunarMonth月的天数，如果iLunarMonth为闰月，
		//高字为第二个iLunarMonth月的天数，否则高字为0 
		// 1901年1月---2050年12月
		static LONG LunarMonthDays(WORD iLunarYear, WORD iLunarMonth);

		//返回阴历iLunarYear年的总天数
		// 1901年1月---2050年12月
		static WORD LunarYearDays(WORD iLunarYear);

		//返回阴历iLunarYear年的闰月月份，如没有返回0
		// 1901年1月---2050年12月
		static WORD GetLeapMonth(WORD iLunarYear);

		//把iYear年格式化成天干记年法表示的字符串
		static void FormatLunarYear(WORD  iYear, TCHAR *pBuffer);

		//把iMonth格式化成中文字符串
		static void FormatMonth(WORD iMonth, TCHAR *pBuffer, BOOL bLunar = TRUE);

		//把iDay格式化成中文字符串
		static void FormatLunarDay(WORD  iDay, TCHAR *pBuffer);

		//计算公历两个日期间相差的天数  1年1月1日 --- 65535年12月31日
		static LONG CalcDateDiff(WORD iEndYear, WORD iEndMonth, WORD iEndDay,
			WORD iStartYear = START_YEAR, 
			WORD iStartMonth =1, WORD iStartDay =1);

		//计算公历iYear年iMonth月iDay日对应的阴历日期,返回对应的阴历节气 0-24
		//1901年1月1日---2050年12月31日
		static WORD GetLunarDate(WORD iYear, WORD iMonth, WORD iDay,
			WORD &iLunarYear, WORD &iLunarMonth, WORD &iLunarDay);

	protected:
		//计算从1901年1月1日过iSpanDays天后的阴历日期
		static void   l_CalcLunarDate(WORD &iYear, WORD &iMonth ,WORD &iDay, LONG iSpanDays);
		//计算公历iYear年iMonth月iDay日对应的节气 0-24，0表不是节气
		static WORD   l_GetLunarHolDay(WORD iYear, WORD iMonth, WORD iDay);
	};

	class CDuiCalendar : public CDuiWindow
	{
	public:
		SOUI_CLASS_NAME(CDuiCalendar, "calendar")
		CDuiCalendar(WORD iYear, WORD iMonth, WORD iDay);
		CDuiCalendar();

	public:
		WORD GetYear(){return m_iYear;}
		WORD GetMonth(){return m_iMonth;}
		WORD GetDay(){return m_iDay;}
		void GetDate(WORD &iYear, WORD &iMonth, WORD &iDay);
		BOOL SetDate(WORD iYear, WORD iMonth, WORD iDay);

	protected:
		void Init();

		void OnPaint(CDCHandle dc);
		void OnLButtonDown(UINT nFlags, CPoint point);

		void OnMouseMove(UINT nFlags,CPoint pt);
		void OnMouseLeave();
		virtual BOOL Load(pugi::xml_node xmlNode);
	protected:
		CRect GetDayRect(WORD iDay);
		WORD HitTest(CPoint  pt);
		void DrawTitle(CDCHandle &dc);
		void DrawDate(CDCHandle &dc);
		void DrawDay(CDCHandle &dc,CRect & rcDay,WORD iDay );
		void RedrawDay(WORD iDay);

		bool OnTodayClick(CDuiWindow * pSender, LPDUINMHDR pNmhdr);

		SOUO_ATTRIBUTES_BEGIN()
			DUIWIN_INT_ATTRIBUTE("titleHeight", m_nTitleHei, FALSE)
			DUIWIN_INT_ATTRIBUTE("footerHeight", m_nFooterHei, FALSE)
			DUIWIN_COLOR_ATTRIBUTE("crWeekend", m_crWeekend, FALSE)
			DUIWIN_COLOR_ATTRIBUTE("crTitleBack", m_crTitleBack, FALSE)
			DUIWIN_COLOR_ATTRIBUTE("crDay", m_crDay, FALSE)
			DUIWIN_SKIN_ATTRIBUTE("daySkin", m_pDaySkin, FALSE)
			DUIWIN_SKIN_ATTRIBUTE("titleSkin", m_pTitleSkin, FALSE)
			DUIWIN_TSTRING_ATTRIBUTE("title-1", m_strTitle[0], FALSE)
			DUIWIN_TSTRING_ATTRIBUTE("title-2", m_strTitle[1], FALSE)
			DUIWIN_TSTRING_ATTRIBUTE("title-3", m_strTitle[2], FALSE)
			DUIWIN_TSTRING_ATTRIBUTE("title-4", m_strTitle[3], FALSE)
			DUIWIN_TSTRING_ATTRIBUTE("title-5", m_strTitle[4], FALSE)
			DUIWIN_TSTRING_ATTRIBUTE("title-6", m_strTitle[5], FALSE)
			DUIWIN_TSTRING_ATTRIBUTE("title-7", m_strTitle[6], FALSE)
		SOUI_ATTRIBUTES_END()

		WND_MSG_MAP_BEGIN()
			MSG_WM_PAINT(OnPaint)
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_MOUSELEAVE(OnMouseLeave)
		WND_MSG_MAP_END()
	protected:
		int				m_nTitleHei;	//表头高度
		int				m_nFooterHei;	//表尾高度
		CDuiSkinBase	*m_pDaySkin;	//日期项皮肤
		CDuiSkinBase	*m_pTitleSkin;	//表头皮肤

		COLORREF		m_crWeekend;	//周末文字颜色
		COLORREF		m_crTitleBack;	//表头背景色
		COLORREF		m_crDay;		//选中日期颜色
		COLORREF		m_crDayBack;	//选中日期背景颜色
		CDuiStringT		m_strTitle[7];	//表头文本

		WORD    m_iYear, m_iMonth, m_iDay;
		int		m_iHoverDay;

	};


}//end of namespace

