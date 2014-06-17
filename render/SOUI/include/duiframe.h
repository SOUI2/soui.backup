//////////////////////////////////////////////////////////////////////////
//  Class Name: CDuiFrame
// Description: A DuiWindow Frame.
//     Creator: Huang Jianxiong
//     Version: 2011.9.1 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma  once

#include "duipanel.h"
#include "DuiFrameDropTarget.h"
#include "FocusManager.h"

namespace SOUI
{

class SOUI_EXP CDuiFrame : public IDuiContainer
{
public:
    CDuiFrame(SWindow *pHost);

    virtual BOOL RegisterDragDrop(HSWND hDuiWnd,IDropTarget *pDropTarget);

    virtual BOOL RevokeDragDrop(HSWND hDuiWnd);

    IDropTarget * GetDropTarget(){return &m_dropTarget;}

    CFocusManager * GetFocusManager() {return &m_focusMgr;}

    virtual LRESULT DoFrameEvent(UINT uMsg,WPARAM wParam,LPARAM lParam);


    virtual BOOL OnReleaseDuiCapture();

    virtual HSWND OnSetDuiCapture(HSWND hDuiWnd);
    virtual void OnSetDuiFocus(HSWND hDuiWnd);

    virtual HSWND OnGetDuiCapture();

    virtual HSWND GetDuiFocus();

    virtual HSWND GetDuiHover();

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
    HSWND m_hCapture;
    HSWND m_hHover;
    BOOL    m_bNcHover;

    CFocusManager m_focusMgr;

    CDuiFrameDropTarget m_dropTarget;

    SWindow    *m_pHost;

    CDuiList<ITimelineHandler*>    m_lstTimelineHandler;
};

}//namespace SOUI

