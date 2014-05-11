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
    friend class CDuiWindow;
public:
	virtual BOOL RegisterDragDrop(HDUIWND hDuiWnd,IDropTarget *pDropTarget)=NULL;

	virtual BOOL RevokeDragDrop(HDUIWND hDuiWnd)=NULL;

    virtual LRESULT OnDuiNotify(LPDUINMHDR pHdr)=NULL;

    virtual HWND GetHostHwnd()=NULL;

    virtual BOOL IsTranslucent()=NULL;

    virtual CRect GetContainerRect()=NULL;

    virtual HDC OnGetDuiDC(const CRect & rc,DWORD gdcFlags)=NULL;

    virtual void OnReleaseDuiDC(HDC hdc,const CRect &rc,DWORD gdcFlags)=NULL;

    virtual void OnRedraw(const CRect &rc)=NULL;

    virtual HDUIWND OnGetDuiCapture()=NULL;

    virtual BOOL OnReleaseDuiCapture()=NULL;

    virtual HDUIWND OnSetDuiCapture(HDUIWND hDuiWnd)=NULL;

    virtual void OnSetDuiFocus(HDUIWND hDuiWnd)=NULL;

    virtual HDUIWND GetDuiHover()=NULL;

    virtual HDUIWND GetDuiFocus()=NULL;

    virtual BOOL DuiCreateCaret(HBITMAP hBmp,int nWidth,int nHeight)=NULL;

    virtual BOOL DuiShowCaret(BOOL bShow)=NULL;

    virtual BOOL DuiSetCaretPos(int x,int y)=NULL;

	virtual BOOL DuiUpdateWindow()=NULL;

	virtual IAcceleratorMgr* GetAcceleratorMgr()=NULL;

	virtual BOOL RegisterTimelineHandler(ITimelineHandler *pHandler)=NULL;

	virtual BOOL UnregisterTimelineHandler(ITimelineHandler *pHandler)=NULL;
};


}//namespace SOUI

