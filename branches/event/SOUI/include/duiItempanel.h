//////////////////////////////////////////////////////////////////////////
//  Class Name: CDuiItemPanel
// Description: A Framework wrapping frame to be used in a duiwindow.
//     Creator: Huang Jianxiong
//     Version: 2011.10.20 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma  once

#include "duiframe.h"

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

    //////////////////////////////////////////////////////////////////////////
    virtual LRESULT DoFrameEvent(UINT uMsg,WPARAM wParam,LPARAM lParam);

    virtual LRESULT OnFireEvent(EventArgs &evt);

    virtual CRect GetContainerRect();

    virtual IRenderTarget * OnGetRenderTarget(const CRect & rc ,DWORD gdcFlags);

    virtual void OnReleaseRenderTarget(IRenderTarget *pRT,const CRect &rc,DWORD gdcFlags);

    virtual void OnRedraw(const CRect &rc);

    virtual BOOL OnReleaseSwndCapture();

    virtual SWND OnSetSwndCapture(SWND hDuiWNd);
    virtual HWND GetHostHwnd();

    virtual BOOL IsTranslucent();

    virtual BOOL SwndCreateCaret(HBITMAP hBmp,int nWidth,int nHeight);

    virtual BOOL SwndShowCaret(BOOL bShow);

    virtual BOOL SwndSetCaretPos(int x,int y);

    virtual BOOL SwndUpdateWindow();

    virtual BOOL RegisterTimelineHandler(ITimelineHandler *pHandler);

    virtual BOOL UnregisterTimelineHandler(ITimelineHandler *pHandler);

    //////////////////////////////////////////////////////////////////////////
    virtual void ModifyItemState(DWORD dwStateAdd, DWORD dwStateRemove);

    virtual SWND SwndFromPoint(POINT ptHitTest, BOOL bOnlyText);

    virtual void Draw(IRenderTarget *pRT,const CRect & rc);

    virtual void SetSkin(ISkinObj *pSkin);
    virtual void SetColor(COLORREF crBk,COLORREF crSelBk);

    virtual BOOL NeedRedrawWhenStateChange();

    CRect GetItemRect();
    void SetItemCapture(BOOL bCapture);
    void SetItemData(LPARAM dwData);
    LPARAM GetItemData();

    BOOL OnUpdateToolTip(SWND hCurTipHost,SWND &hNewTipHost,CRect &rcTip,SStringT &strTip);
    
    void OnSetCaretValidateRect(LPCRECT lpRect);

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