//////////////////////////////////////////////////////////////////////////
//  Class Name: CDuiFrame
// Description: A DuiWindow Frame.
//     Creator: Huang Jianxiong
//     Version: 2011.9.1 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma  once

#include "DuiFrameDropTarget.h"
#include "FocusManager.h"

namespace SOUI
{

class SOUI_EXP CSwndContainer : public ISwndContainer
{
public:
    CSwndContainer(SWindow *pHost);

    virtual BOOL RegisterDragDrop(SWND hDuiWnd,IDropTarget *pDropTarget);

    virtual BOOL RevokeDragDrop(SWND hDuiWnd);

    IDropTarget * GetDropTarget(){return &m_dropTarget;}

    CFocusManager * GetFocusManager() {return &m_focusMgr;}

    virtual LRESULT DoFrameEvent(UINT uMsg,WPARAM wParam,LPARAM lParam);

    virtual BOOL OnReleaseSwndCapture();

    virtual SWND OnSetSwndCapture(SWND hDuiWnd);
    virtual void OnSetSwndFocus(SWND hDuiWnd);

    virtual SWND OnGetSwndCapture();

    virtual SWND SwndGetFocus();

    virtual SWND SwndGetHover();

    virtual IAcceleratorMgr* GetAcceleratorMgr(){return &m_focusMgr;}

    virtual BOOL RegisterTimelineHandler(ITimelineHandler *pHandler);

    virtual BOOL UnregisterTimelineHandler(ITimelineHandler *pHandler);

    virtual void OnNextFrame();
protected:
    virtual void OnFrameMouseMove(UINT uFlag,CPoint pt);

    virtual void OnFrameMouseLeave();


    virtual BOOL OnFrameSetCursor(const CPoint &pt);

    virtual void OnFrameMouseEvent(UINT uMsg,WPARAM wParam,LPARAM lParam);

    virtual void OnFrameKeyEvent(UINT uMsg,WPARAM wParam,LPARAM lParam);

    virtual void OnFrameKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    virtual void OnActivate(UINT nState);

protected:
    SWND m_hCapture;
    SWND m_hHover;
    BOOL    m_bNcHover;

    CFocusManager m_focusMgr;

    CDuiFrameDropTarget m_dropTarget;

    SWindow    *m_pHost;

    SList<ITimelineHandler*>    m_lstTimelineHandler;
};

}//namespace SOUI

