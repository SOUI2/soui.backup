//////////////////////////////////////////////////////////////////////////
//  Class Name: SItemPanel
// Description: A Framework wrapping frame to be used in a swindow.
//     Creator: Huang Jianxiong
//     Version: 2011.10.20 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma  once

#include "SwndContainerImpl.h"

namespace SOUI
{

class SItemPanel;

class SOUI_EXP IItemContainer
{
public:
    virtual void OnItemSetCapture(SItemPanel *pItem,BOOL bCapture)=NULL;//设置or释放鼠标捕获
    virtual BOOL OnItemGetRect(SItemPanel *pItem,CRect &rcItem)=NULL;    //获得表项的显示位置
    virtual BOOL IsItemRedrawDelay()=NULL;                                    //指示表项的更新方式
};

class SOUI_EXP SItemPanel : public SWindow, public SwndContainerImpl
{
public:
    SItemPanel(SWindow *pFrameHost,pugi::xml_node xmlNode,IItemContainer *pItemContainer=NULL);
    virtual ~SItemPanel() {}

    virtual void OnFinalRelease();

public://SwndContainerImpl
    virtual LRESULT DoFrameEvent(UINT uMsg,WPARAM wParam,LPARAM lParam);

    virtual BOOL OnFireEvent(EventArgs &evt);

    virtual CRect GetContainerRect();

    virtual IRenderTarget * OnGetRenderTarget(const CRect & rc ,DWORD gdcFlags);

    virtual void OnReleaseRenderTarget(IRenderTarget *pRT,const CRect &rc,DWORD gdcFlags);

    virtual void OnRedraw(const CRect &rc);

    virtual BOOL OnReleaseSwndCapture();

    virtual SWND OnSetSwndCapture(SWND swnd);
    virtual HWND GetHostHwnd();
    virtual const SStringW & GetTranslatorContext();

    virtual BOOL IsTranslucent();

    virtual BOOL SwndCreateCaret(HBITMAP hBmp,int nWidth,int nHeight);

    virtual BOOL SwndShowCaret(BOOL bShow);

    virtual BOOL SwndSetCaretPos(int x,int y);

    virtual BOOL SwndUpdateWindow();

    virtual BOOL RegisterTimelineHandler(ITimelineHandler *pHandler);

    virtual BOOL UnregisterTimelineHandler(ITimelineHandler *pHandler);

    virtual SMessageLoop *GetMsgLoop();
    
    virtual IScriptModule * GetScriptModule();

public://SWindow
    virtual void ModifyItemState(DWORD dwStateAdd, DWORD dwStateRemove);

    virtual SWND SwndFromPoint(POINT ptHitTest, BOOL bOnlyText);

    virtual void Draw(IRenderTarget *pRT,const CRect & rc);

    virtual void SetSkin(ISkinObj *pSkin);
    virtual void SetColor(COLORREF crBk,COLORREF crSelBk);

    virtual BOOL NeedRedrawWhenStateChange();
    virtual BOOL OnUpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo);
    virtual BOOL IsLayeredWindow() const {return FALSE;}
    virtual void OnSetCaretValidateRect(LPCRECT lpRect);
    
    CRect GetItemRect();
    void SetItemCapture(BOOL bCapture);
    void SetItemData(LPARAM dwData);
    LPARAM GetItemData();

    LPARAM GetItemIndex(){return m_lpItemIndex;}
    void SetItemIndex(LPARAM lp){m_lpItemIndex=lp;}

protected:
    SWindow * m_pFrmHost;
    IItemContainer * m_pItemContainer;
    COLORREF m_crBk, m_crSelBk;
    LPARAM        m_dwData;
    LPARAM        m_lpItemIndex;
};


}//namespace SOUI