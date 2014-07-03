#pragma once

namespace SOUI
{

struct IAcceleratorMgr;


struct SOUI_EXP ITimelineHandler
{
    virtual void OnNextFrame()=NULL;
};

class SOUI_EXP ISwndContainer : public ITimelineHandler
{
    friend class SWindow;
public:
    virtual BOOL RegisterDragDrop(SWND swnd,IDropTarget *pDropTarget)=NULL;

    virtual BOOL RevokeDragDrop(SWND swnd)=NULL;

    virtual BOOL OnFireEvent(EventArgs &evt)=NULL;

    virtual HWND GetHostHwnd()=NULL;

    virtual BOOL IsTranslucent()=NULL;

    virtual CRect GetContainerRect()=NULL;

    virtual IRenderTarget * OnGetRenderTarget(const CRect & rc,DWORD gdcFlags)=NULL;

    virtual void OnReleaseRenderTarget(IRenderTarget *pRT,const CRect &rc,DWORD gdcFlags)=NULL;

    virtual void OnRedraw(const CRect &rc)=NULL;

    virtual SWND OnGetSwndCapture()=NULL;

    virtual BOOL OnReleaseSwndCapture()=NULL;

    virtual SWND OnSetSwndCapture(SWND hDuiWnd)=NULL;

    virtual void OnSetSwndFocus(SWND hDuiWnd)=NULL;

    virtual SWND SwndGetHover()=NULL;

    virtual SWND SwndGetFocus()=NULL;

    virtual BOOL SwndCreateCaret(HBITMAP hBmp,int nWidth,int nHeight)=NULL;

    virtual BOOL SwndShowCaret(BOOL bShow)=NULL;

    virtual BOOL SwndSetCaretPos(int x,int y)=NULL;

    virtual BOOL SwndUpdateWindow()=NULL;

    virtual IAcceleratorMgr* GetAcceleratorMgr()=NULL;

    virtual BOOL RegisterTimelineHandler(ITimelineHandler *pHandler)=NULL;

    virtual BOOL UnregisterTimelineHandler(ITimelineHandler *pHandler)=NULL;
};


}//namespace SOUI

