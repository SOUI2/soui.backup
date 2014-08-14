//////////////////////////////////////////////////////////////////////////
//  Class Name: SHostWnd
//    Description: Real Container of SWindow
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "SThreadActiveWndMgr.h"

#include "SwndContainerImpl.h"

#include "SimpleWnd.h"
#include "SDropTargetDispatcher.h"
#include "event/eventcrack.h"

namespace SOUI
{

    class SHostWnd;
    class SDummyWnd : public CSimpleWnd
    {
    public:
        SDummyWnd(SHostWnd* pOwner):m_pOwner(pOwner)
        {
        }

        void OnPaint(HDC dc);

        BEGIN_MSG_MAP_EX(SDummyWnd)
            MSG_WM_PAINT(OnPaint)
        END_MSG_MAP()
    private:
        SHostWnd *m_pOwner;
    };

    class SHostWndAttr : public SObject
    {
        SOUI_CLASS_NAME(SWindow, L"hostwndattr")
    public:
        SHostWndAttr(void)
            : m_bResizable(FALSE)
            , m_bTranslucent(FALSE)
            , m_bAppWnd(FALSE)
            , m_bToolWnd(FALSE)
            , m_szMin(200, 200)
            , m_szInit(640,480)
            , m_dwStyle(0)
            , m_dwExStyle(0)
            , m_hAppIconSmall(NULL)
            , m_hAppIconBig(NULL)
        {

        }

        ~SHostWndAttr()
        {
            if(m_hAppIconSmall) DeleteObject(m_hAppIconSmall);
            if(m_hAppIconBig) DeleteObject(m_hAppIconBig);
        }
        
        SOUI_ATTRS_BEGIN()
            ATTR_STRINGW(L"name",m_strName,FALSE)
            ATTR_STRINGW(L"title",m_strTitle,FALSE)
            ATTR_SIZE(L"size",m_szInit,FALSE)
            ATTR_INT(L"width",m_szInit.cx,FALSE)
            ATTR_INT(L"height",m_szInit.cy,FALSE)
            ATTR_RECT(L"margin",m_rcMargin,FALSE)
            ATTR_SIZE(L"minsize",m_szMin,FALSE)
            ATTR_DWORD(L"wndStyle",m_dwStyle,FALSE)
            ATTR_DWORD(L"wndStyleEx",m_dwExStyle,FALSE)
            ATTR_INT(L"resizable",m_bResizable,FALSE)
            ATTR_INT(L"translucent",m_bTranslucent,FALSE)
            ATTR_INT(L"appWnd",m_bAppWnd,FALSE)
            ATTR_INT(L"toolWindow",m_bToolWnd,FALSE)
            ATTR_ICON(L"smallIcon",m_hAppIconSmall,FALSE)
            ATTR_ICON(L"bigIcon",m_hAppIconBig,FALSE)
        SOUI_ATTRS_END()

        CRect m_rcMargin;
        CSize m_szMin;
        CSize m_szInit;

        BOOL m_bResizable;
        BOOL m_bAppWnd;
        BOOL m_bToolWnd;
        BOOL m_bTranslucent;    //窗口的半透明属性

        DWORD m_dwStyle;
        DWORD m_dwExStyle;

        SStringW m_strName;     //Host Name，在语言翻译时作为context使用
        SStringW m_strTitle;
        HICON   m_hAppIconSmall;
        HICON   m_hAppIconBig;
    };

class STipCtrl;

class SOUI_EXP SHostWnd
    : public CSimpleWnd
    , public SwndContainerImpl
    , public SWindow
{
    friend class SDummyWnd;
public:
    SHostWnd(LPCTSTR pszResName = NULL);
    virtual ~SHostWnd();

public:
    SWindow * GetRoot(){return this;}
    CSimpleWnd * GetNative(){return this;}

    HWND Create(HWND hWndParent,int x,int y,int nWidth,int nHeight);
    HWND Create(HWND hWndParent,DWORD dwStyle,DWORD dwExStyle, int x, int y, int nWidth, int nHeight);

    BOOL InitFromXml(pugi::xml_node xmlNode);
    
    BOOL AnimateHostWindow(DWORD dwTime,DWORD dwFlags);
protected:
    void _Redraw();
    
    SDummyWnd            m_dummyWnd;    //半透明窗口使用的一个响应WM_PAINT消息的窗口
    SHostWndAttr         m_hostAttr;
    SStringT m_strXmlLayout;

    // Tracking flag
    BOOL m_bTrackFlag;


    BOOL m_bCaretShowing;    //当前有插入符正在显示
    CAutoRefPtr<IBitmap>    m_bmpCaret; //半透明窗口中的模拟插入符
    SIZE                    m_szCaret;  //插入符大小
    BOOL m_bCaretActive;    //模拟插入符正在显示标志
    CPoint m_ptCaret;        //插入符位置
    CRect    m_rcValidateCaret;//可以显示插入符的位置

    BOOL m_bNeedRepaint;
    BOOL m_bNeedAllRepaint;

    STipCtrl    * m_pTipCtrl;

    CAutoRefPtr<IRegion>    m_rgnInvalidate;
    CAutoRefPtr<IRenderTarget> m_memRT;
    
    CAutoRefPtr<SStylePool>  m_privateStylePool;
    CAutoRefPtr<SSkinPool>  m_privateSkinPool;
protected:
    //////////////////////////////////////////////////////////////////////////
    // Message handler

    void OnPrint(HDC dc, UINT uFlags);

    void OnPaint(HDC dc);

    BOOL OnEraseBkgnd(HDC dc);

    int OnCreate(LPCREATESTRUCT lpCreateStruct);

    void OnDestroy();

    void OnSize(UINT nType, CSize size);

    void OnMouseMove(UINT nFlags, CPoint point);

    void OnMouseLeave();

    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnLButtonDblClk(UINT nFlags, CPoint point);

    BOOL OnSetCursor(HWND hWnd, UINT nHitTest, UINT message);

    void OnTimer(UINT_PTR idEvent);

    void OnSwndTimer(char cTimerID);

    void DrawCaret(CPoint pt);

    LRESULT OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT OnHostMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    void OnActivate(UINT nState, BOOL bMinimized, HWND wndOther);

    //////////////////////////////////////////////////////////////////////////
    // IContainer

    virtual BOOL OnFireEvent(EventArgs &evt);

    virtual CRect GetContainerRect();

    virtual HWND GetHostHwnd();

    virtual const SStringW & GetHostName();

    virtual IRenderTarget * OnGetRenderTarget(const CRect & rc,DWORD gdcFlags);

    virtual void OnReleaseRenderTarget(IRenderTarget * pRT,const CRect &rc,DWORD gdcFlags);

    virtual void OnRedraw(const CRect &rc);

    virtual BOOL OnReleaseSwndCapture();

    virtual SWND OnSetSwndCapture(SWND swnd);

    virtual BOOL IsTranslucent();

    virtual BOOL SwndCreateCaret(HBITMAP hBmp,int nWidth,int nHeight);

    virtual BOOL SwndShowCaret(BOOL bShow);

    virtual BOOL SwndSetCaretPos(int x,int y);

    virtual BOOL SwndUpdateWindow();

    virtual BOOL RegisterTimelineHandler(ITimelineHandler *pHandler);

    virtual BOOL UnregisterTimelineHandler(ITimelineHandler *pHandler);
    
    virtual SMessageLoop * GetMsgLoop();

    //////////////////////////////////////////////////////////////////////////
 
    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);

    void OnGetMinMaxInfo(LPMINMAXINFO lpMMI);

    BOOL OnNcActivate(BOOL bActive);
    
    UINT OnWndNcHitTest(CPoint point);

    void OnSetFocus(HWND wndOld);
    void OnKillFocus(HWND wndFocus);

    void OnSetCaretValidateRect( LPCRECT lpRect );

    void UpdateHost(HDC dc,const CRect &rc);
    void UpdateLayerFromRenderTarget(IRenderTarget *pRT,BYTE byAlpha);
protected:
    virtual BOOL _HandleEvent(SOUI::EventArgs *pEvt){return FALSE;}

    BEGIN_MSG_MAP_EX(SHostWnd)
        MSG_WM_SIZE(OnSize)
        MSG_WM_PRINT(OnPrint)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_ACTIVATE(OnActivate)
        MSG_WM_SETFOCUS(OnSetFocus)
        MSG_WM_KILLFOCUS(OnKillFocus)
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST, WM_KEYLAST, OnKeyEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_IME_STARTCOMPOSITION,WM_IME_KEYLAST,OnKeyEvent)
        MESSAGE_HANDLER_EX(WM_IME_CHAR, OnKeyEvent)
        MESSAGE_HANDLER_EX(WM_ACTIVATEAPP,OnHostMsg)
        MSG_WM_SETCURSOR(OnSetCursor)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_NCACTIVATE(OnNcActivate)
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
        MSG_WM_NCHITTEST(OnWndNcHitTest)
        MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
        REFLECT_NOTIFY_CODE(NM_CUSTOMDRAW)
    END_MSG_MAP()
};

}//namespace SOUI
