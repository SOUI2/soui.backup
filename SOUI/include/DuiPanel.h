//////////////////////////////////////////////////////////////////////////
//   File Name: DuiPanel.h
// Description: CDuiPanel Definition, Virtual Container of DuiWindows
//     Creator: Zhang Xiaoxuan
//     Version: 2009.04.28 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "duiwnd.h"

namespace SOUI
{

#define DUISB_NULL	0
#define DUISB_HORZ	1
#define DUISB_VERT	2
#define DUISB_BOTH	(DUISB_HORZ|DUISB_VERT)
#define TIMER_SBWAIT	1		//启动连续滚动的定时器
#define TIMER_SBGO	2		//连续滚动的定时器

#define DEF_UPDATEINTERVAL	20

typedef struct tagSBHITINFO
{
	DWORD uSbCode:16;
	DWORD nState:8;
	DWORD bVertical:8;
} SBHITINFO,*PSBHITINFO;

inline bool operator !=(const SBHITINFO &a, const SBHITINFO &b)
{
	return memcmp(&a,&b,sizeof(SBHITINFO))!=0;
}

class SOUI_EXP CDuiPanel: public CDuiWindow
{
    DUIOBJ_DECLARE_CLASS_NAME(CDuiPanel, "div")

public:
    CDuiPanel();
    virtual ~CDuiPanel() {}

    BOOL ShowScrollBar(int wBar, BOOL bShow);

    BOOL EnableScrollBar(int wBar,BOOL bEnable);

    BOOL IsScrollBarEnable(BOOL bVertical);

    void SetScrollInfo(SCROLLINFO si,BOOL bVertical);

    BOOL SetScrollPos(BOOL bVertical, int nNewPos,BOOL bRedraw);

    int GetScrollPos(BOOL bVertical);

    BOOL SetScrollRange(BOOL bVertical,int nMinPos,int nMaxPos,BOOL bRedraw);

    BOOL GetScrollRange(BOOL bVertical,	LPINT lpMinPos,	LPINT lpMaxPos);

    BOOL HasScrollBar(BOOL bVertical);


    SBHITINFO HitTest(CPoint pt);

    virtual void GetClient(LPRECT pRect);

protected:
    CRect GetSbPartRect(BOOL bVertical,UINT uSBCode);
	CRect GetSbRailwayRect(BOOL bVertical);
    CRect GetScrollBarRect(BOOL bVertical);

    int OnCreate(LPVOID);

    void OnNcPaint(CDCHandle dc);

    virtual BOOL OnDuiNcHitTest(CPoint pt);

    void OnNcLButtonDown(UINT nFlags, CPoint point);

    void OnNcLButtonUp(UINT nFlags,CPoint pt);

    void OnNcMouseMove(UINT nFlags, CPoint point) ;


    void OnNcMouseLeave();

    //滚动条显示或者隐藏时发送该消息
    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);

    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    void OnDuiTimer(char cTimerID);

	void OnShowWindow(BOOL bShow, UINT nStatus);
protected:
    virtual int  GetScrollLineSize(BOOL bVertical)
    {
        return 10;
    }
    virtual BOOL OnScroll(BOOL bVertical,UINT uCode,int nPos);

    int GetSbSlideLength(BOOL bVertical);

    CRect GetSbSlideRectByPos(BOOL bVertical,int nPos);

	void ScrollUpdate();

	LRESULT OnAttrScrollbarSkin(CDuiStringA strValue,BOOL bLoading);

    SCROLLINFO m_siVer,m_siHoz;
    CDuiScrollbarSkin *m_pSkinSb;
    int			m_nSbArrowSize;
    int			m_nSbWid;
    CPoint		m_ptDragSb;
    BOOL		m_bDragSb;
    SBHITINFO		m_HitInfo;
    int			m_nDragPos;

    CRect		m_rcClient;
    int			m_wBarVisible;	//滚动条显示信息
    int			m_wBarEnable;	//滚动条可操作信息

	DWORD		m_dwUpdateTime;	//记录调用UpdateDuiWindow的时间
	DWORD		m_dwUpdateInterval;

    DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
		DUIWIN_CUSTOM_ATTRIBUTE("sbskin",OnAttrScrollbarSkin)
		DUIWIN_INT_ATTRIBUTE("sbarrowsize", m_nSbArrowSize, FALSE)
		DUIWIN_INT_ATTRIBUTE("sbwid", m_nSbWid, FALSE)
		DUIWIN_INT_ATTRIBUTE("sbenable", m_wBarEnable, FALSE)
		DUIWIN_UINT_ATTRIBUTE("updateinterval", m_dwUpdateInterval, FALSE)
    DUIWIN_DECLARE_ATTRIBUTES_END()

    DUIWIN_BEGIN_MSG_MAP()
    MSG_WM_CREATE(OnCreate)
    MSG_WM_NCPAINT_EX(OnNcPaint)
    MSG_WM_NCCALCSIZE(OnNcCalcSize)
    MSG_WM_NCLBUTTONDOWN(OnNcLButtonDown)
    MSG_WM_NCLBUTTONUP(OnNcLButtonUp)
    MSG_WM_NCMOUSEMOVE(OnNcMouseMove)
    MSG_WM_NCMOUSELEAVE(OnNcMouseLeave)
    MSG_WM_MOUSEWHEEL(OnMouseWheel)
    MSG_WM_DUITIMER(OnDuiTimer)
	MSG_WM_SHOWWINDOW(OnShowWindow)
    DUIWIN_END_MSG_MAP()
};

class SOUI_EXP CDuiScrollView : public CDuiPanel
{
	DUIOBJ_DECLARE_CLASS_NAME(CDuiScrollView, "scrollview")
public:
	CDuiScrollView();
	virtual ~CDuiScrollView() {}

	CSize GetViewSize();

	void SetViewSize(CSize szView);

	CPoint GetViewOrigin();

	void SetViewOrigin(CPoint pt);

protected:
	int OnCreate(LPVOID);

	void OnSize(UINT nType,CSize size);
protected:
	virtual void OnViewSizeChanged(CSize szOld,CSize szNew){}
	virtual void OnViewOriginChanged(CPoint ptOld,CPoint ptNew) {
		UpdateChildrenPosition();
	}

protected:
	virtual CRect GetChildrenLayoutRect()
	{
		CRect rcRet=__super::GetChildrenLayoutRect();
		rcRet.OffsetRect(-m_ptOrigin);
		rcRet.right=rcRet.left+m_szView.cx;
		rcRet.bottom=rcRet.top+m_szView.cy;
		return rcRet;
	}

	virtual BOOL OnScroll(BOOL bVertical,UINT uCode,int nPos);

	void UpdateScrollBar();
protected:
	DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
		DUIWIN_INT_ATTRIBUTE("viewwid", m_szView.cx, FALSE)
		DUIWIN_INT_ATTRIBUTE("viewhei", m_szView.cy, FALSE)
		DUIWIN_INT_ATTRIBUTE("origin-x", m_ptOrigin.x, FALSE)
		DUIWIN_INT_ATTRIBUTE("origin-y", m_ptOrigin.y, FALSE)
	DUIWIN_DECLARE_ATTRIBUTES_END()

	DUIWIN_BEGIN_MSG_MAP()
		MSG_WM_CREATE(OnCreate)
		MSG_WM_SIZE(OnSize)
	DUIWIN_END_MSG_MAP()
protected:
	CSize m_szView;
	CPoint m_ptOrigin;
};

}//namespace SOUI