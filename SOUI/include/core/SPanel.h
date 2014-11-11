/**
* Copyright (C) 2014-2050 
* All rights reserved.
* 
* @file       SPanel.h
* @brief      
* @version    v1.0      
* @author     SOUI group   
* @date       2014/08/02
* 
* Describe    SOUI的窗口面板，实现在非客户区的滚动条支持
*/

#pragma once

#include "SWnd.h"

namespace SOUI
{

#define SSB_NULL    0
#define SSB_HORZ    1
#define SSB_VERT    2
#define SSB_BOTH    (SSB_HORZ|SSB_VERT)
#define TIMER_SBWAIT    1        //启动连续滚动的定时器
#define TIMER_SBGO    2        //连续滚动的定时器


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

    class SOUI_EXP SPanel: public SWindow
    {
        SOUI_CLASS_NAME(SPanel, L"div")

    public:
        SPanel();
        virtual ~SPanel() {}

        BOOL ShowScrollBar(int wBar, BOOL bShow);

        BOOL EnableScrollBar(int wBar,BOOL bEnable);

        BOOL IsScrollBarEnable(BOOL bVertical);

        void SetScrollInfo(SCROLLINFO si,BOOL bVertical);

        BOOL SetScrollPos(BOOL bVertical, int nNewPos,BOOL bRedraw);

        int GetScrollPos(BOOL bVertical);

        BOOL SetScrollRange(BOOL bVertical,int nMinPos,int nMaxPos,BOOL bRedraw);

        BOOL GetScrollRange(BOOL bVertical,    LPINT lpMinPos,    LPINT lpMaxPos);

        BOOL HasScrollBar(BOOL bVertical);


        SBHITINFO HitTest(CPoint pt);

        virtual void GetClientRect(LPRECT pRect);

    protected:
        CRect GetSbPartRect(BOOL bVertical,UINT uSBCode);
        CRect GetSbRailwayRect(BOOL bVertical);
        CRect GetScrollBarRect(BOOL bVertical);

        int OnCreate(LPVOID);

        void OnNcPaint(IRenderTarget *pRT);

        virtual BOOL OnNcHitTest(CPoint pt);

        void OnNcLButtonDown(UINT nFlags, CPoint point);

        void OnNcLButtonUp(UINT nFlags,CPoint pt);

        void OnNcMouseMove(UINT nFlags, CPoint point) ;


        void OnNcMouseLeave();

        //滚动条显示或者隐藏时发送该消息
        LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);

        BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

        void OnTimer(char cTimerID);

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

        HRESULT OnAttrScrollbarSkin(SStringW strValue,BOOL bLoading);

        SCROLLINFO m_siVer,m_siHoz;
        SSkinScrollbar *m_pSkinSb;
        int            m_nSbArrowSize;
        int            m_nSbWid;
        CPoint        m_ptDragSb;
        BOOL        m_bDragSb;
        SBHITINFO        m_HitInfo;
        int            m_nDragPos;

        CRect        m_rcClient;
        int            m_wBarVisible;    //滚动条显示信息
        int            m_wBarEnable;    //滚动条可操作信息

        DWORD        m_dwUpdateTime;    //记录调用UpdateSWindow的时间
        DWORD        m_dwUpdateInterval;

        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"sbSkin",OnAttrScrollbarSkin)
            ATTR_INT(L"sbArrowSize", m_nSbArrowSize, FALSE)
            ATTR_INT(L"sbWid", m_nSbWid, FALSE)
            ATTR_INT(L"sbEnable", m_wBarEnable, FALSE)
            ATTR_UINT(L"updateInterval", m_dwUpdateInterval, FALSE)
        SOUI_ATTRS_END()

        SOUI_MSG_MAP_BEGIN()
            MSG_WM_CREATE(OnCreate)
            MSG_WM_NCPAINT_EX(OnNcPaint)
            MSG_WM_NCCALCSIZE(OnNcCalcSize)
            MSG_WM_NCLBUTTONDOWN(OnNcLButtonDown)
            MSG_WM_NCLBUTTONUP(OnNcLButtonUp)
            MSG_WM_NCMOUSEMOVE(OnNcMouseMove)
            MSG_WM_NCMOUSELEAVE(OnNcMouseLeave)
            MSG_WM_MOUSEWHEEL(OnMouseWheel)
            MSG_WM_TIMER_EX(OnTimer)
            MSG_WM_SHOWWINDOW(OnShowWindow)
        SOUI_MSG_MAP_END()
    };

    class SOUI_EXP SScrollView : public SPanel
    {
        SOUI_CLASS_NAME(SScrollView, L"scrollview")
    public:
        SScrollView();
        virtual ~SScrollView() {}

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
        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"viewwid", m_szView.cx, FALSE)
            ATTR_INT(L"viewhei", m_szView.cy, FALSE)
            ATTR_INT(L"origin-x", m_ptOrigin.x, FALSE)
            ATTR_INT(L"origin-y", m_ptOrigin.y, FALSE)
        SOUI_ATTRS_END()

        SOUI_MSG_MAP_BEGIN()
            MSG_WM_CREATE(OnCreate)
            MSG_WM_SIZE(OnSize)
        SOUI_MSG_MAP_END()
    protected:
        CSize m_szView;
        CPoint m_ptOrigin;
    };

}//namespace SOUI