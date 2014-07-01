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
    virtual BOOL RegisterDragDrop(SWND hDuiWnd,IDropTarget *pDropTarget)=NULL;

    virtual BOOL RevokeDragDrop(SWND hDuiWnd)=NULL;

    virtual LRESULT OnDuiNotify(LPSNMHDR pHdr)=NULL;

    virtual HWND GetHostHwnd()=NULL;

    virtual BOOL IsTranslucent()=NULL;

    virtual CRect GetContainerRect()=NULL;

    virtual IRenderTarget * OnGetRenderTarget(const CRect & rc,DWORD gdcFlags)=NULL;

    virtual void OnReleaseRenderTarget(IRenderTarget *pRT,const CRect &rc,DWORD gdcFlags)=NULL;

    virtual void OnRedraw(const CRect &rc)=NULL;

    virtual SWND OnGetDuiCapture()=NULL;

    virtual BOOL OnReleaseDuiCapture()=NULL;

    virtual SWND OnSetDuiCapture(SWND hDuiWnd)=NULL;

    virtual void OnSetDuiFocus(SWND hDuiWnd)=NULL;

    virtual SWND GetDuiHover()=NULL;

    virtual SWND GetDuiFocus()=NULL;

    virtual BOOL DuiCreateCaret(HBITMAP hBmp,int nWidth,int nHeight)=NULL;

    virtual BOOL DuiShowCaret(BOOL bShow)=NULL;

    virtual BOOL DuiSetCaretPos(int x,int y)=NULL;

    virtual BOOL DuiUpdateWindow()=NULL;

    virtual IAcceleratorMgr* GetAcceleratorMgr()=NULL;

    virtual BOOL RegisterTimelineHandler(ITimelineHandler *pHandler)=NULL;

    virtual BOOL UnregisterTimelineHandler(ITimelineHandler *pHandler)=NULL;
};


}//namespace SOUI

