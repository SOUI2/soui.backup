#pragma once

namespace SOUI
{

struct IAcceleratorMgr;


struct ITimelineHandler
{
    virtual void OnNextFrame()=NULL;
};

class SOUI_EXP IDuiContainer : public ITimelineHandler
{
    friend class SWindow;
public:
    virtual BOOL RegisterDragDrop(HSWND hDuiWnd,IDropTarget *pDropTarget)=NULL;

    virtual BOOL RevokeDragDrop(HSWND hDuiWnd)=NULL;

    virtual LRESULT OnDuiNotify(LPDUINMHDR pHdr)=NULL;

    virtual HWND GetHostHwnd()=NULL;

    virtual BOOL IsTranslucent()=NULL;

    virtual CRect GetContainerRect()=NULL;

    virtual IRenderTarget * OnGetRenderTarget(const CRect & rc,DWORD gdcFlags)=NULL;

    virtual void OnReleaseRenderTarget(IRenderTarget *pRT,const CRect &rc,DWORD gdcFlags)=NULL;

    virtual void OnRedraw(const CRect &rc)=NULL;

    virtual HSWND OnGetDuiCapture()=NULL;

    virtual BOOL OnReleaseDuiCapture()=NULL;

    virtual HSWND OnSetDuiCapture(HSWND hDuiWnd)=NULL;

    virtual void OnSetDuiFocus(HSWND hDuiWnd)=NULL;

    virtual HSWND GetDuiHover()=NULL;

    virtual HSWND GetDuiFocus()=NULL;

    virtual BOOL DuiCreateCaret(HBITMAP hBmp,int nWidth,int nHeight)=NULL;

    virtual BOOL DuiShowCaret(BOOL bShow)=NULL;

    virtual BOOL DuiSetCaretPos(int x,int y)=NULL;

    virtual BOOL DuiUpdateWindow()=NULL;

    virtual IAcceleratorMgr* GetAcceleratorMgr()=NULL;

    virtual BOOL RegisterTimelineHandler(ITimelineHandler *pHandler)=NULL;

    virtual BOOL UnregisterTimelineHandler(ITimelineHandler *pHandler)=NULL;
};


}//namespace SOUI

