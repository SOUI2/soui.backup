/*
基本 是 按 mfc  的 datetime 实现的 只实现了 天数 选择  月 和 年 没做
月 和 年 一般也用的少。 选择也可以。
*/

#pragma once
#include <core/swnd.h>

namespace SOUI
{
class SOUI_EXP SCalendarEx : public SWindow
{
	SOUI_CLASS_NAME(SCalendarEx, L"calendarex")   //
public:
	SCalendarEx(WORD iYeay, WORD iMonth, WORD iDay);
	SCalendarEx();
	~SCalendarEx();
	WORD GetYear();
	WORD GetMonth();
	WORD GetDay();
	void GetDate(WORD &iYear, WORD &iMonth, WORD &iDay);
	BOOL SetDate(WORD iYear, WORD iMonth, WORD iDay, bool bNotify=false);
protected:
	void Init();
	int OnCreate(LPVOID);
	void OnPaint(IRenderTarget *pRT);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint pt);
	void OnMouseLeave();
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	SOUI_ATTRS_BEGIN()
		ATTR_COLOR(L"colorOther", m_crOtherDayText, TRUE)
		ATTR_COLOR(L"colorSel", m_crSelDayBack, TRUE)
		ATTR_COLOR(L"colorHover", m_crHoverText, TRUE)
		ATTR_SKIN(L"daySkin", m_pSkinDay, FALSE)
	SOUI_ATTRS_END()

	SOUI_MSG_MAP_BEGIN()
		MSG_WM_CREATE(OnCreate)
		MSG_WM_PAINT_EX(OnPaint)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
	SOUI_MSG_MAP_END()
protected:
	// 定位 小于0  是 年月 3个按钮 暂时 没完成   0 - 41  天    42 today按钮
	int HitTest(const CPoint& pt);
	void DrawYearMonth(IRenderTarget* pRT, const CRect& rect);
	void DrawWeek(IRenderTarget* pRT, const CRect& rect);
	void DrawDay(IRenderTarget* pRT, CRect& rcDay, int nItem);
	void DrawToday(IRenderTarget* pRT, CRect& rcDay);
	void GetItemRect(int nItem, CRect& rcItem);
	void SetLastMonth();
	void SetNextMonth();
protected:
	SLayoutSize			m_nYearMonthHeight;   //年月
	SLayoutSize			m_nWeekHeight;				//星期高度
	SLayoutSize			m_nFooterHeight;			
	
	COLORREF				m_crOtherDayText;			//其他 天 的 字体颜色 
	COLORREF				m_crSelDayBack;			// 选中 的 天 背颜色
	COLORREF				m_crHoverText;
	
	ISkinObj*				m_pSkinDay;					// 天 皮肤 

	STrText					m_strWeek[7];  /**< 表头文本 */
	
	struct wDayInfo
	{
		WORD			iDay;				// 日历 天 
		int					nType;			// -1 前一个月 0 当月 1 下一个月
	};
	wDayInfo				m_arrDays[42];
	CRect						m_rcDays;
	CRect						m_rcToday;

	int							m_nSelItem;
	int							m_nHoverItem;

	WORD					m_iYear;
	WORD					m_iMonth;
	SYSTEMTIME			m_Today;
};
    //
    // 日期 选择控件
    //
class SOUI_EXP SDateTimePicker : public SWindow, public ISDropDownOwner
{
	SOUI_CLASS_NAME(SDateTimePicker, L"dateTimePicker")   //

public:
	enum EnDateType
	{
		eDT_NULL = 0,
		eDT_Year ,
		eDT_Month ,
		eDT_Day ,
		eDT_Hour ,
		eDT_Minute ,
		eDT_Second ,
	};
	SDateTimePicker();
    ~SDateTimePicker();

public:
	void CloseUp();
	EnDateType HitTest(CPoint pt);
protected:		// 继承 
	virtual SWindow* GetDropDownOwner();
	virtual void OnCreateDropDown(SDropDownWnd* pDropDown);
	virtual void OnDestroyDropDown(SDropDownWnd* pDropDown);
protected:
	virtual BOOL CreateChildren(pugi::xml_node xmlNode);
	bool OnDateChanged(EventCalendarExChanged* pEvt);
	bool OnDateCmd(EventCmd* pEvt);
	void GetDropBtnRect(LPRECT pBtnRc, LPRECT pSkinRc = NULL);
	bool CalcPopupRect(int nHeight, CRect& rcPopup);
	void Draw(EnDateType eType, IRenderTarget* pRT, SStringT& szText, CRect& rcText);
protected:
	void OnPaint(IRenderTarget* pRT);
	void OnLButtonDown(UINT nFlags, CPoint pt);
	void OnMouseMove(UINT nFlags, CPoint pt);
	void OnMouseLeave();
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags );
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnDestroy();
	BOOL IsFocusable();
	void OnSetFocus(SWND wndOld);
	void OnKillFocus(SWND wndFocus);
	void DrawFocus(IRenderTarget *pRT);
	void TimeWheel(bool bUp);
	void CircluNum(bool bUp, WORD& wNum, WORD wMin, WORD wMax);				// 循环 子 
	
	SOUI_ATTRS_BEGIN()
		ATTR_SKIN(L"btnSkin", m_pSkinBtn, FALSE)
	SOUI_ATTRS_END()

	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)        
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		MSG_WM_KEYDOWN(OnKeyDown) 
		//MSG_WM_CHAR(OnChar)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SETFOCUS_EX(OnSetFocus)
		MSG_WM_KILLFOCUS_EX(OnKillFocus)
	SOUI_MSG_MAP_END()

protected:
	
	DWORD								m_dwBtnState;  /**< 按钮状态      */
	ISkinObj*							m_pSkinBtn;    /**< 按钮资源      */
	bool									m_bDropdown;        /**< 是否按下   */
	SDropDownWnd*				m_pDropDownWnd;  /**< DropDown指针 */
	EnDateType						m_eSelDateType;
	int										m_nNumWidth;
	int										m_nNumHeight;
	int										m_nCharWidth;
	COLORREF							m_crSelBg;
	COLORREF							m_crSelText;
	SYSTEMTIME						m_sysTime;
	SStringT								m_sKey;

	SCalendarEx*						m_pCalendar;
};
}