#pragma once
#include "DuiPanel.h"

namespace SOUI
{

enum SPLITMODE {SM_COL=0,SM_ROW};


class SOUI_EXP CDuiSplitPane : public CDuiWindow
{
    friend class CDuiSplitWnd;
    DUIOBJ_DECLARE_CLASS_NAME(CDuiSplitPane, "splitpane")
public:
    CDuiSplitPane();
    virtual ~CDuiSplitPane() {}

protected:
    DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
    DUIWIN_INT_ATTRIBUTE("idealsize", m_nSizeIdeal, TRUE)
    DUIWIN_INT_ATTRIBUTE("minsize", m_nSizeMin, TRUE)
    DUIWIN_INT_ATTRIBUTE("priority", m_nPriority, TRUE)
    DUIWIN_DECLARE_ATTRIBUTES_END()
protected:
    int m_nSizeIdeal;
    int m_nSizeMin;
    int m_nPriority;
};

class SOUI_EXP CDuiSplitWnd :
    public CDuiWindow
{
    DUIOBJ_DECLARE_CLASS_NAME(CDuiSplitWnd, "splitwnd")

	enum {
		layout_vert=1,		//纵向布局改变
		layout_horz=2,		//横向布局改变
		layout_pos=4,		//窗口位置发生改变
	};
    struct PANEORDER
    {
        int idx;
        CDuiSplitPane *pPane;
    };
public:
    CDuiSplitWnd(void);
    virtual ~CDuiSplitWnd(void);

    BOOL SetPaneInfo(UINT iPane,int nIdealSize,int nMinSize,int nPriority);

    BOOL GetPaneInfo(UINT iPane,int *pnIdealSize,int *pnMinSize,int *pnPriority);

    BOOL ShowPanel(UINT iPane);

    BOOL HidePanel(UINT iPane);

protected:
	virtual void UpdateChildrenPosition(){}//empty

    int GetVisiblePanelCount();

    int GetNextVisiblePanel(UINT iPanel);

    virtual BOOL LoadChildren(pugi::xml_node xmlNode);

    virtual BOOL OnDuiSetCursor(const CPoint &pt);

    void OnDestroy();

    void OnPaint(CDCHandle dc);

    LRESULT OnWindowPosChanged(LPRECT lpWndPos);

    void OnLButtonDown(UINT nFlags,CPoint pt);

    void OnLButtonUp(UINT nFlags,CPoint pt);

    void OnMouseMove(UINT nFlags,CPoint pt);

    static int FunComp(const void * p1,const void * p2);

    void Relayout(UINT uMode);

    DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
    DUIWIN_INT_ATTRIBUTE("sepsize", m_nSepSize, TRUE)
    DUIWIN_INT_ATTRIBUTE("adjustable", m_bAdjustable, TRUE)
    DUIWIN_INT_ATTRIBUTE("colmode", m_bColMode, TRUE)
    DUIWIN_SKIN_ATTRIBUTE("skinsep",m_pSkinSep,TRUE)
    DUIWIN_DECLARE_ATTRIBUTES_END()

    DUIWIN_BEGIN_MSG_MAP()
    MSG_WM_PAINT(OnPaint)
    MSG_WM_DESTROY(OnDestroy)
    MSG_WM_DUIWINPOSCHANGED(OnWindowPosChanged)
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_LBUTTONUP(OnLButtonUp)
    MSG_WM_MOUSEMOVE(OnMouseMove)
    DUIWIN_END_MSG_MAP()

protected:
    CDuiArray<CDuiSplitPane *> m_arrPane;
    CDuiSkinBase *m_pSkinSep;
    int m_nSepSize;
    BOOL m_bAdjustable;
    BOOL m_bColMode;

    int m_iDragBeam;
    CPoint m_ptClick;
};

class SOUI_EXP CDuiSplitWnd_Col : public CDuiSplitWnd
{
    DUIOBJ_DECLARE_CLASS_NAME(CDuiSplitWnd_Col, "splitcol")
public:
    CDuiSplitWnd_Col()
    {
        m_bColMode=TRUE;
    }
};

class SOUI_EXP CDuiSplitWnd_Row : public CDuiSplitWnd
{
    DUIOBJ_DECLARE_CLASS_NAME(CDuiSplitWnd_Row, "splitrow")
public:
    CDuiSplitWnd_Row()
    {
        m_bColMode=FALSE;
    }
};

}//namespace SOUI