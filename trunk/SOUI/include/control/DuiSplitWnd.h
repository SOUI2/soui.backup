#pragma once
#include "DuiPanel.h"

namespace SOUI
{

enum SPLITMODE {SM_COL=0,SM_ROW};


class SOUI_EXP SSplitPane : public SWindow
{
    friend class SSplitWnd;
    SOUI_CLASS_NAME(SSplitPane, L"splitpane")
public:
    SSplitPane();
    virtual ~SSplitPane() {}

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"idealsize", m_nSizeIdeal, TRUE)
        ATTR_INT(L"minsize", m_nSizeMin, TRUE)
        ATTR_INT(L"priority", m_nPriority, TRUE)
    SOUI_ATTRS_END()
protected:
    int m_nSizeIdeal;
    int m_nSizeMin;
    int m_nPriority;
};

class SOUI_EXP SSplitWnd :
    public SWindow
{
    SOUI_CLASS_NAME(SSplitWnd, L"splitwnd")

    enum {
        layout_vert=1,        //纵向布局改变
        layout_horz=2,        //横向布局改变
        layout_pos=4,        //窗口位置发生改变
    };
    struct PANEORDER
    {
        int idx;
        SSplitPane *pPane;
    };
public:
    SSplitWnd(void);
    virtual ~SSplitWnd(void);

    BOOL SetPaneInfo(UINT iPane,int nIdealSize,int nMinSize,int nPriority);

    BOOL GetPaneInfo(UINT iPane,int *pnIdealSize,int *pnMinSize,int *pnPriority);

    BOOL ShowPanel(UINT iPane);

    BOOL HidePanel(UINT iPane);

protected:
    virtual void UpdateChildrenPosition(){}//empty

    int GetVisiblePanelCount();

    int GetNextVisiblePanel(UINT iPanel);

    virtual BOOL CreateChildren(pugi::xml_node xmlNode);

    virtual BOOL OnDuiSetCursor(const CPoint &pt);

    void OnDestroy();

    void OnPaint(IRenderTarget * pRT);

    LRESULT OnWindowPosChanged(LPRECT lpWndPos);

    void OnLButtonDown(UINT nFlags,CPoint pt);

    void OnLButtonUp(UINT nFlags,CPoint pt);

    void OnMouseMove(UINT nFlags,CPoint pt);

    static int FunComp(const void * p1,const void * p2);

    void Relayout(UINT uMode);

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"sepsize", m_nSepSize, TRUE)
        ATTR_INT(L"adjustable", m_bAdjustable, TRUE)
        ATTR_INT(L"colmode", m_bColMode, TRUE)
        ATTR_SKIN(L"skinsep",m_pSkinSep,TRUE)
    SOUI_ATTRS_END()

    WND_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_WINPOSCHANGED_EX(OnWindowPosChanged)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
    WND_MSG_MAP_END()

protected:
    SArray<SSplitPane *> m_arrPane;
    ISkinObj *m_pSkinSep;
    int m_nSepSize;
    BOOL m_bAdjustable;
    BOOL m_bColMode;

    int m_iDragBeam;
    CPoint m_ptClick;
};

class SOUI_EXP SSplitWnd_Col : public SSplitWnd
{
    SOUI_CLASS_NAME(SSplitWnd_Col, L"splitcol")
public:
    SSplitWnd_Col()
    {
        m_bColMode=TRUE;
    }
};

class SOUI_EXP SSplitWnd_Row : public SSplitWnd
{
    SOUI_CLASS_NAME(SSplitWnd_Row, L"splitrow")
public:
    SSplitWnd_Row()
    {
        m_bColMode=FALSE;
    }
};

}//namespace SOUI