#include "souistd.h"

#include "SApp.h"
#include "core/shostwnd.h"
#include "helper/mybuffer.h"
#include "helper/STipCtrl.h"
#include "helper/color.h"
#include "helper/SplitString.h"

namespace SOUI
{

#define TIMER_CARET    1
#define TIMER_NEXTFRAME 2

//////////////////////////////////////////////////////////////////////////
//    SDummyWnd
//////////////////////////////////////////////////////////////////////////
void SDummyWnd::OnPaint( HDC dc )
{
    PAINTSTRUCT ps;
    ::BeginPaint(m_hWnd, &ps);
    ::EndPaint(m_hWnd, &ps);
    m_pOwner->OnPrint(NULL,1);
}

//////////////////////////////////////////////////////////////////////////
// SHostWnd
//////////////////////////////////////////////////////////////////////////
SHostWnd::SHostWnd( LPCTSTR pszResName /*= NULL*/ )
: SwndContainerImpl(this)
, m_strXmlLayout(pszResName)
, m_bTrackFlag(FALSE)
, m_bCaretShowing(FALSE)
, m_bCaretActive(FALSE)
, m_bNeedRepaint(FALSE)
, m_bNeedAllRepaint(TRUE)
, m_pTipCtrl(NULL)
, m_dummyWnd(this)
{
    m_privateStylePool.Attach(new SStylePool);
    m_privateSkinPool.Attach(new SSkinPool);
    SetContainer(this);
}

SHostWnd::~SHostWnd()
{
    GETSTYLEPOOLMGR->PopStylePool(m_privateStylePool);
    GETSKINPOOLMGR->PopSkinPool(m_privateSkinPool);
}

HWND SHostWnd::Create(HWND hWndParent,DWORD dwStyle,DWORD dwExStyle, int x, int y, int nWidth, int nHeight)
{
    if (NULL != m_hWnd)
        return m_hWnd;

    HWND hWnd = CSimpleWnd::Create(_T("HOSTWND"),dwStyle,dwExStyle, x,y,nWidth,nHeight,hWndParent,NULL);
    if(!hWnd) return NULL;


    if(!m_strXmlLayout.IsEmpty())
    {
        pugi::xml_document xmlDoc;
        SStringTList strLst;

        if(2 == SplitString(m_strXmlLayout,_T(':'),strLst))
        {
            LOADXML(xmlDoc,strLst[1],strLst[0]);
        }else
        {
            LOADXML(xmlDoc,strLst[0],RT_LAYOUT);
        }

        if(xmlDoc)
        {
            InitFromXml(xmlDoc.child(L"SOUI"));
        }
    }

    if(nWidth==0 || nHeight==0) CenterWindow(hWnd);
    return hWnd;
}

HWND SHostWnd::Create(HWND hWndParent,int x,int y,int nWidth,int nHeight)
{
    return Create(hWndParent, WS_POPUP | WS_CLIPCHILDREN | WS_TABSTOP,0,x,y,nWidth,nHeight);
}


BOOL SHostWnd::InitFromXml(pugi::xml_node xmlNode )
{
    if(!xmlNode) return FALSE;
    if(!CSimpleWnd::IsWindow()) return FALSE;
    
    SSendMessage(WM_DESTROY);   //为了能够重入，先销毁原有的SOUI窗口
    if(m_privateStylePool->GetCount())
    {
        m_privateStylePool->RemoveAll();
        GETSTYLEPOOLMGR->PopStylePool(m_privateStylePool);
    }
    m_privateStylePool->Init(xmlNode.child(L"style"));
    if(m_privateStylePool->GetCount())
    {
        GETSTYLEPOOLMGR->PushStylePool(m_privateStylePool);
    }
    if(m_privateSkinPool->GetCount())
    {
        m_privateSkinPool->RemoveAll();
        GETSKINPOOLMGR->PopSkinPool(m_privateSkinPool);
    }
    m_privateSkinPool->LoadSkins(xmlNode.child(L"skin"));//从xmlNode加加载私有skin
    if(m_privateSkinPool->GetCount())
    {
        GETSKINPOOLMGR->PushSkinPool(m_privateSkinPool);
    }    
    DWORD dwStyle =CSimpleWnd::GetStyle();
    DWORD dwExStyle  = CSimpleWnd::GetExStyle();
    
    SHostWndAttr hostAttr;
    hostAttr.InitFromXml(xmlNode);
    m_hostAttr=hostAttr;

    if (m_hostAttr.m_bResizable)
    {
        dwStyle |= WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
    }
    if(m_hostAttr.m_bAppWnd)
    {
        dwStyle |= WS_SYSMENU ;
        dwExStyle |= WS_EX_APPWINDOW;
    }else if(m_hostAttr.m_bToolWnd)
    {
        dwExStyle |= WS_EX_TOOLWINDOW;
    }
    if(m_hostAttr.m_bTranslucent)
    {
        dwExStyle |= WS_EX_LAYERED;
    }
    
    if(m_hostAttr.m_dwStyle!=0) dwStyle=m_hostAttr.m_dwStyle&(~WS_VISIBLE);
    if(m_hostAttr.m_dwExStyle != 0) dwExStyle =m_hostAttr.m_dwExStyle;
    
    ModifyStyle(0,dwStyle);
    ModifyStyleEx(0,dwExStyle);
    CSimpleWnd::SetWindowText(S_CW2T(m_hostAttr.m_strTitle));
    
    if(m_hostAttr.m_bTranslucent)
    {
        SetWindowLongPtr(GWL_EXSTYLE, GetWindowLongPtr(GWL_EXSTYLE) | WS_EX_LAYERED);
        m_dummyWnd.Create(_T("SOUI_DUMMY_WND"),WS_POPUP,WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,0,0,10,10,m_hWnd,NULL);
        m_dummyWnd.SetWindowLongPtr(GWL_EXSTYLE,m_dummyWnd.GetWindowLongPtr(GWL_EXSTYLE) | WS_EX_LAYERED);
        ::SetLayeredWindowAttributes(m_dummyWnd.m_hWnd,0,0,LWA_ALPHA);
        m_dummyWnd.ShowWindow(SW_SHOWNOACTIVATE);
    }else if(dwExStyle & WS_EX_LAYERED || m_hostAttr.m_byAlpha!=0xFF)
    {
        if(!(dwExStyle & WS_EX_LAYERED)) ModifyStyleEx(0,WS_EX_LAYERED);
        ::SetLayeredWindowAttributes(m_hWnd,0,m_hostAttr.m_byAlpha,LWA_ALPHA);
    }
    if(m_hostAttr.m_hAppIconSmall)
    {
        SendMessage(WM_SETICON,FALSE,(LPARAM)m_hostAttr.m_hAppIconSmall);
    }
    if(m_hostAttr.m_hAppIconBig)
    {
        SendMessage(WM_SETICON,TRUE,(LPARAM)m_hostAttr.m_hAppIconBig);
    }

    SWindow::InitFromXml(xmlNode.child(L"root"));

    CRect rcClient;
    CSimpleWnd::GetClientRect(&rcClient);
    if(rcClient.IsRectEmpty())//APP没有指定窗口大小，使用XML中的值
    {
        SetWindowPos(NULL,0,0,m_hostAttr.m_szInit.cx,m_hostAttr.m_szInit.cy,SWP_NOZORDER|SWP_NOMOVE);
    }else
    {
        Move(&rcClient);
        OnWindowPosChanged(NULL);
    }

    _Redraw();

    RedrawRegion(m_memRT,m_rgnInvalidate);

    return TRUE;
}

void SHostWnd::_Redraw()
{
    m_bNeedAllRepaint = TRUE;
    m_bNeedRepaint = TRUE;
    m_rgnInvalidate->Clear();

    if(!m_hostAttr.m_bTranslucent)
        CSimpleWnd::Invalidate(FALSE);
    else if(m_dummyWnd.IsWindow()) 
        m_dummyWnd.Invalidate(FALSE);
}

void SHostWnd::OnPrint(HDC dc, UINT uFlags)
{
    if((m_hostAttr.m_bTranslucent && !uFlags) && !m_bNeedAllRepaint && !m_bNeedRepaint) return;
    if (m_bNeedAllRepaint)
    {
        m_rgnInvalidate->Clear();
        m_bNeedAllRepaint = FALSE;
        m_bNeedRepaint=TRUE;
    }


    if (m_bNeedRepaint)
    {
        SThreadActiveWndMgr::EnterPaintLock();
        CAutoRefPtr<IFont> defFont,oldFont;
        defFont = SFontPool::getSingleton().GetFont(FF_DEFAULTFONT);
        m_memRT->SelectObject(defFont,(IRenderObj**)&oldFont);
        m_memRT->SetTextColor(RGBA(0,0,0,0xFF));

        //m_rgnInvalidate有可能在RedrawRegion时被修改，必须生成一个临时的区域对象
        CAutoRefPtr<IRegion> pRgnUpdate=m_rgnInvalidate;
        m_rgnInvalidate=NULL;
        GETRENDERFACTORY->CreateRegion(&m_rgnInvalidate);

        CRect rcInvalid=m_rcWindow;
        if (!pRgnUpdate->IsEmpty())
        {
            m_memRT->PushClipRegion(pRgnUpdate,RGN_COPY);
            pRgnUpdate->GetRgnBox(&rcInvalid);
        }else
        {
            m_memRT->PushClipRect(&rcInvalid,RGN_COPY);
        }
        //清除残留的alpha值
        m_memRT->FillSolidRect(rcInvalid,0);

        if(m_bCaretActive) DrawCaret(m_ptCaret);//clear old caret 
        RedrawRegion(m_memRT, pRgnUpdate);
        if(m_bCaretActive) DrawCaret(m_ptCaret);//redraw caret 
        
        m_memRT->PopClip();
        
        m_memRT->SelectObject(oldFont);

        m_bNeedRepaint = FALSE;
        SThreadActiveWndMgr::LeavePaintLock();
    }

    CRect rc;
    CSimpleWnd::GetClientRect(&rc);
    UpdateHost(dc,rc);
}

void SHostWnd::OnPaint(HDC dc)
{
    PAINTSTRUCT ps;
    dc=::BeginPaint(m_hWnd, &ps);
    OnPrint(m_hostAttr.m_bTranslucent?NULL:dc, 0);
    ::EndPaint(m_hWnd, &ps);
}

BOOL SHostWnd::OnEraseBkgnd(HDC dc)
{
    return TRUE;
}


int SHostWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    GETRENDERFACTORY->CreateRenderTarget(&m_memRT,0,0);
    GETRENDERFACTORY->CreateRegion(&m_rgnInvalidate);
    
    //tooltip
    m_pTipCtrl=new STipCtrl;
    m_pTipCtrl->Create(m_hWnd);
    GetMsgLoop()->AddMessageFilter(m_pTipCtrl);

    SWindow::SetContainer(this);

    return 0;
}

void SHostWnd::OnDestroy()
{
    SWindow::SSendMessage(WM_DESTROY);

    if(m_pTipCtrl)
    {
        GetMsgLoop()->RemoveMessageFilter(m_pTipCtrl);
        if (m_pTipCtrl->IsWindow())
            m_pTipCtrl->DestroyWindow();
        delete m_pTipCtrl;
    }
    if(m_hostAttr.m_bTranslucent && m_dummyWnd.IsWindow())
    {
        m_dummyWnd.DestroyWindow();
    }
}

void SHostWnd::OnSize(UINT nType, CSize size)
{
    if(IsIconic()) return;

    if (size.cx==0 || size.cy==0)
        return;
    
    m_memRT->Resize(size);

    CRect rcClient;
    CSimpleWnd::GetClientRect(rcClient);

    Move(rcClient);

    _Redraw();

    SetMsgHandled(FALSE);//交给其它处理器继续处理
}

void SHostWnd::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!m_bTrackFlag)
    {
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(tme);
        tme.hwndTrack = m_hWnd;
        tme.dwFlags = TME_LEAVE;
        tme.dwHoverTime = 0;
        m_bTrackFlag = TrackMouseEvent(&tme);
    }

    OnMouseEvent(WM_MOUSEMOVE,nFlags,MAKELPARAM(point.x,point.y));
}

void SHostWnd::OnMouseLeave()
{
    m_bTrackFlag = FALSE;
    DoFrameEvent(WM_MOUSELEAVE,0,0);
}

void SHostWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
    DoFrameEvent(WM_LBUTTONDOWN,nFlags,MAKELPARAM(point.x,point.y));
}

void SHostWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    DoFrameEvent(WM_LBUTTONDBLCLK,nFlags,MAKELPARAM(point.x,point.y));
}

BOOL SHostWnd::OnSetCursor(HWND hwnd, UINT nHitTest, UINT message)
{
    if(hwnd!=m_hWnd) return FALSE;
    if(nHitTest==HTCLIENT)
    {
        CPoint pt;
        GetCursorPos(&pt);
        ScreenToClient(&pt);
        return DoFrameEvent(WM_SETCURSOR,0,MAKELPARAM(pt.x,pt.y))!=0;
    }
    return DefWindowProc()!=0;
}

void SHostWnd::OnTimer(UINT_PTR idEvent)
{
    STimerID sTimerID((DWORD)idEvent);
    if(sTimerID.bSwndTimer)
    {
        SWindow *pSwnd=SWindowMgr::GetWindow((SWND)sTimerID.Swnd);
        if(pSwnd)
        {
            if(pSwnd==this) OnSwndTimer(sTimerID.uTimerID);//由于DUIWIN采用了ATL一致的消息映射表模式，因此在HOST中不能有DUI的消息映射表（重复会导致SetMsgHandled混乱)
            else pSwnd->SSendMessage(WM_TIMER,sTimerID.uTimerID,0);
        }
        else
        {
            //窗口已经删除，自动清除该窗口的定时器
            ::KillTimer(m_hWnd,idEvent);
        }
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}

void SHostWnd::OnSwndTimer( char cTimerID )
{
    if(cTimerID==TIMER_CARET)
    {
        SASSERT(m_bCaretShowing);
        STraceA("!!OnSwndTimer:pt=%d,%d\n",m_ptCaret.x,m_ptCaret.y);
        DrawCaret(m_ptCaret);
        m_bCaretActive=!m_bCaretActive;
    }else if(cTimerID==TIMER_NEXTFRAME)
    {
        OnNextFrame();
    }
}

void SHostWnd::DrawCaret(CPoint pt)
{
    CAutoRefPtr<IRenderTarget> pRTCaret;
    GETRENDERFACTORY->CreateRenderTarget(&pRTCaret,0,0);
    pRTCaret->SelectObject(m_bmpCaret);

    CRect rcCaret(pt,m_szCaret);
    CRect rcShowCaret;
    rcShowCaret.IntersectRect(m_rcValidateCaret,rcCaret);
    
    m_memRT->BitBlt(&rcShowCaret,pRTCaret,rcShowCaret.left-pt.x,rcShowCaret.top-pt.y,DSTINVERT);
    
    if(!m_hostAttr.m_bTranslucent)
    {
        CSimpleWnd::InvalidateRect(rcCaret, FALSE);
    }else
    {
        if(m_dummyWnd.IsWindow()) 
            m_dummyWnd.Invalidate(FALSE);
    }
}

LRESULT SHostWnd::OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DoFrameEvent(uMsg,wParam,lParam);    //将鼠标消息转发到SWindow处理

    if(m_pTipCtrl && m_pTipCtrl->IsWindow())
    {
        SWindow *pHover=SWindowMgr::GetWindow(m_hHover);
        if(!pHover || pHover->IsDisabled(TRUE))
        {
            m_pTipCtrl->ShowTip(FALSE);
        }
        else
        {
            SWND hNewTipHost=0;
            CRect rcTip;
            SStringT strTip;
            BOOL bUpdate=pHover->OnUpdateToolTip(m_pTipCtrl->m_dwHostID,hNewTipHost,rcTip,strTip);
            if(bUpdate)
            {
                m_pTipCtrl->m_dwHostID=hNewTipHost;
                m_pTipCtrl->UpdateTip(rcTip,strTip);
            }
        }
    }

    return 0;
}

LRESULT SHostWnd::OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(uMsg==WM_SYSKEYDOWN || uMsg==WM_SYSKEYUP)
    {
        SWindow *pFocus=SWindowMgr::GetWindow(m_focusMgr.GetFocusedHwnd());
        if(!pFocus  || !(pFocus->OnGetDlgCode()&SC_WANTSYSKEY))
        {
            SetMsgHandled(FALSE);
            return 0;
        }
    }
    LRESULT lRet = DoFrameEvent(uMsg,wParam,lParam);
    SetMsgHandled(SWindow::IsMsgHandled());
    return lRet;
}

LRESULT SHostWnd::OnHostMsg( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return DoFrameEvent(uMsg,wParam,lParam);
}

BOOL SHostWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    ScreenToClient(&pt);
    return DoFrameEvent(WM_MOUSEWHEEL,MAKEWPARAM(nFlags,zDelta),MAKELPARAM(pt.x,pt.y))!=0;
}


void SHostWnd::OnActivate( UINT nState, BOOL bMinimized, HWND wndOther )
{
    if(nState!=WA_INACTIVE)
        ::SetFocus(m_hWnd);
    else
        ::SetFocus(NULL);
}

BOOL SHostWnd::OnFireEvent(EventArgs &evt)
{
    return _HandleEvent(&evt);
}

CRect SHostWnd::GetContainerRect()
{
    return m_rcWindow;
}

HWND SHostWnd::GetHostHwnd()
{
    return m_hWnd;
}

IRenderTarget * SHostWnd::OnGetRenderTarget(const CRect & rc,DWORD gdcFlags)
{
    IRenderTarget *pRT=NULL;
    GETRENDERFACTORY->CreateRenderTarget(&pRT,rc.Width(),rc.Height());
    pRT->OffsetViewportOrg(-rc.left,-rc.top);
    
    pRT->SelectObject(SFontPool::getSingleton().GetFont(FF_DEFAULTFONT));
    pRT->SetTextColor(RGBA(0,0,0,0xFF));

    if(!(gdcFlags & OLEDC_NODRAW))
    {
        if(m_bCaretActive)
        {
            DrawCaret(m_ptCaret);//clear old caret
        }
        pRT->BitBlt(&rc,m_memRT,rc.left,rc.top,SRCCOPY);
    }
    return pRT;
}

void SHostWnd::OnReleaseRenderTarget(IRenderTarget * pRT,const CRect &rc,DWORD gdcFlags)
{
    if(!(gdcFlags & OLEDC_NODRAW))
    {
        m_memRT->BitBlt(&rc,pRT,rc.left,rc.top,SRCCOPY);
        if(m_bCaretActive)
        {
            DrawCaret(m_ptCaret);//clear old caret
        }
        HDC dc=GetDC();
        UpdateHost(dc,rc);
        ReleaseDC(dc);
    }
    pRT->Release();
}

void SHostWnd::UpdateHost(HDC dc, const CRect &rcInvalid )
{
    HDC hdc=m_memRT->GetDC(0);
    if(m_hostAttr.m_bTranslucent)
    {
        CRect rc;
        CSimpleWnd::GetWindowRect(&rc);
        BLENDFUNCTION bf= {AC_SRC_OVER,0,0xFF,AC_SRC_ALPHA};
        HDC hdcDst=::GetDC(NULL);
        /*
        typedef 
        BOOL (WINAPI *Fun_UpdateLayeredWindow)(
            __in HWND hWnd,
            __in_opt HDC hdcDst,
            __in_opt POINT* pptDst,
            __in_opt SIZE* psize,
            __in_opt HDC hdcSrc,
            __in_opt POINT* pptSrc,
            __in COLORREF crKey,
            __in_opt BLENDFUNCTION* pblend,
            __in DWORD dwFlags);
        typedef 
        BOOL (WINAPI *Fun_UpdateLayeredWindowIndirect)(
            __in HWND hWnd,
            __in_ecount(1) UPDATELAYEREDWINDOWINFO CONST *pULWInfo);
        
        static Fun_UpdateLayeredWindow s_FunUpdateLayeredWindow = (Fun_UpdateLayeredWindow)GetProcAddress(GetModuleHandle(_T("user32.dll")),"UpdateLayeredWindow");
        static Fun_UpdateLayeredWindowIndirect s_FunUpdateLayeredWindowIndirect = (Fun_UpdateLayeredWindowIndirect)GetProcAddress(GetModuleHandle(_T("user32.dll")),"UpdateLayeredWindowIndirect");
        
        if(s_FunUpdateLayeredWindowIndirect && 0)
        {
            UPDATELAYEREDWINDOWINFO info;
            info.cbSize = sizeof(info);
            info.hdcDst=hdcDst;
            info.pptDst = & rc.TopLeft();
            info.psize = & rc.Size();
            info.hdcSrc=hdc;
            info.pptSrc = & CPoint(0,0);
            info.crKey  = 0;
            info.pblend = &bf;
            info.prcDirty = & rcInvalid;
            info.dwFlags = ULW_ALPHA;
            s_FunUpdateLayeredWindowIndirect(m_hWnd,&info);
        }
        else if(s_FunUpdateLayeredWindow)
        {
            s_FunUpdateLayeredWindow(m_hWnd,hdcDst,&rc.TopLeft(),&rc.Size(),hdc,&CPoint(0,0),0,&bf,ULW_ALPHA);
        }
        */
        ::UpdateLayeredWindow(m_hWnd,hdcDst,&rc.TopLeft(),&rc.Size(),hdc,&CPoint(0,0),0,&bf,ULW_ALPHA);
        ::ReleaseDC(NULL,hdcDst);
    }
    else
    {
        ::BitBlt(dc,rcInvalid.left,rcInvalid.top,rcInvalid.Width(),rcInvalid.Height(),hdc,rcInvalid.left,rcInvalid.top,SRCCOPY);
    }
    m_memRT->ReleaseDC(hdc);
}

void SHostWnd::OnRedraw(const CRect &rc)
{
    if(!IsWindow()) return;
    
    m_rgnInvalidate->CombineRect(&rc,RGN_OR);
    
    m_bNeedRepaint = TRUE;

    if(!m_hostAttr.m_bTranslucent)
    {
        CSimpleWnd::InvalidateRect(rc, FALSE);
    }else
    {
        if(m_dummyWnd.IsWindow()) 
            m_dummyWnd.Invalidate(FALSE);
    }
}

BOOL SHostWnd::OnReleaseSwndCapture()
{
    if(!SwndContainerImpl::OnReleaseSwndCapture()) return FALSE;
    ::ReleaseCapture();
    CPoint pt;
    GetCursorPos(&pt);
    ScreenToClient(&pt);
    PostMessage(WM_MOUSEMOVE,0,MAKELPARAM(pt.x,pt.y));
    return TRUE;
}

SWND SHostWnd::OnSetSwndCapture(SWND swnd)
{
    CSimpleWnd::SetCapture();
    return SwndContainerImpl::OnSetSwndCapture(swnd);
}

BOOL SHostWnd::IsTranslucent()
{
    return m_hostAttr.m_bTranslucent;
}


BOOL SHostWnd::SwndCreateCaret( HBITMAP hBmp,int nWidth,int nHeight )
{
    ::CreateCaret(m_hWnd,hBmp,nWidth,nHeight);
    if(m_bmpCaret)
    {
        m_bmpCaret=NULL;
    }
    
    CAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT,nWidth,nHeight);
    m_bmpCaret = (IBitmap*) pRT->GetCurrentObject(OT_BITMAP);
    m_szCaret.cx=nWidth;
    m_szCaret.cy=nHeight;
        
    if(hBmp)
    {
        //以拉伸方式创建一个插入符位图
        HDC hdc=pRT->GetDC(0);
        HDC hdc2=CreateCompatibleDC(hdc);
        SelectObject(hdc2,hBmp);
        
        BITMAP bm;
        GetObject(hBmp,sizeof(bm),&bm);
        StretchBlt(hdc,0,0,nWidth,nHeight,hdc2,0,0,bm.bmWidth,bm.bmHeight,SRCCOPY);
        DeleteDC(hdc2);
        pRT->ReleaseDC(hdc);
    }
    else
    {
        //创建一个黑色插入符的位图
        pRT->FillSolidRect(&CRect(0,0,nWidth,nHeight),RGBA(0,0,0,0xFF));
    }
    return TRUE;
}

BOOL SHostWnd::SwndShowCaret( BOOL bShow )
{
    m_bCaretShowing=bShow;

    if(bShow)
    {
        SWindow::SetTimer(TIMER_CARET,GetCaretBlinkTime());
        if(!m_bCaretActive)
        {
            DrawCaret(m_ptCaret);
            m_bCaretActive=TRUE;
        }
    }
    else
    {
        SWindow::KillTimer(TIMER_CARET);
        if(m_bCaretActive)
        {
            DrawCaret(m_ptCaret);
        }
        m_bCaretActive=FALSE;
    }
   return TRUE;
}

BOOL SHostWnd::SwndSetCaretPos( int x,int y )
{
    if(!SetCaretPos(x,y)) return FALSE;
    if(m_bCaretShowing && m_bCaretActive)
    {
        //clear old caret
        DrawCaret(m_ptCaret);
    }
    m_ptCaret=CPoint(x,y);
    if(m_bCaretShowing && m_bCaretActive)
    {
        //draw new caret
        DrawCaret(m_ptCaret);
    }
    return TRUE;
}


BOOL SHostWnd::SwndUpdateWindow()
{
    if(m_hostAttr.m_bTranslucent) ::UpdateWindow(m_dummyWnd.m_hWnd);
    else ::UpdateWindow(m_hWnd);
    return TRUE;
}

LRESULT SHostWnd::OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
{
    if (bCalcValidRects && (CSimpleWnd::GetStyle() & WS_POPUP))
    {
        CRect rcWindow;
        CSimpleWnd::GetWindowRect(rcWindow);

        LPNCCALCSIZE_PARAMS pParam = (LPNCCALCSIZE_PARAMS)lParam;

        if (SWP_NOSIZE == (SWP_NOSIZE & pParam->lppos->flags))
            return 0;

        if (0 == (SWP_NOMOVE & pParam->lppos->flags))
        {
            rcWindow.left = pParam->lppos->x;
            rcWindow.top = pParam->lppos->y;
        }

        rcWindow.right = rcWindow.left + pParam->lppos->cx;
        rcWindow.bottom = rcWindow.top + pParam->lppos->cy;
        pParam->rgrc[0] = rcWindow;
    }

    return 0;
}

void SHostWnd::OnGetMinMaxInfo(LPMINMAXINFO lpMMI)
{
    HMONITOR hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONULL);

    if (hMonitor)
    {
        MONITORINFO mi = {sizeof(MONITORINFO)};
        ::GetMonitorInfo(hMonitor, &mi);

        CRect rcWork = mi.rcWork, rcMonitor = mi.rcMonitor;
        lpMMI->ptMaxPosition.x = abs(rcWork.left - rcMonitor.left) - 1;
        lpMMI->ptMaxPosition.y = abs(rcWork.top - rcMonitor.top) - 1;
        lpMMI->ptMaxSize.x = abs(rcWork.Width()) + 2;
        lpMMI->ptMaxSize.y = abs(rcWork.Height()) + 2;
        lpMMI->ptMaxTrackSize.x = abs(rcWork.Width()) + 2;
        lpMMI->ptMaxTrackSize.y = abs(rcWork.Height()) + 2;
        lpMMI->ptMinTrackSize = CPoint(m_hostAttr.m_szMin.cx, m_hostAttr.m_szMin.cy);
    }
}

BOOL SHostWnd::OnNcActivate(BOOL bActive)
{
    return TRUE;
}

UINT SHostWnd::OnWndNcHitTest(CPoint point)
{
    if (m_hostAttr.m_bResizable && !::IsZoomed(m_hWnd))
    {
        ScreenToClient(&point);
        if (point.x > m_rcWindow.right - m_hostAttr.m_rcMargin.right)
        {
            if (point.y > m_rcWindow.bottom - m_hostAttr.m_rcMargin.bottom)
            {
                return HTBOTTOMRIGHT;
            }
            else if (point.y < m_hostAttr.m_rcMargin.top)
            {
                return HTTOPRIGHT;
            }
            return HTRIGHT;
        }
        else if (point.x < m_hostAttr.m_rcMargin.left)
        {
            if (point.y > m_rcWindow.bottom - m_hostAttr.m_rcMargin.bottom)
            {
                return HTBOTTOMLEFT;
            }
            else if (point.y < m_hostAttr.m_rcMargin.top)
            {
                return HTTOPLEFT;
            }
            return HTLEFT;
        }
        else if (point.y > m_rcWindow.bottom - m_hostAttr.m_rcMargin.bottom)
        {
            return HTBOTTOM;
        }
        else if (point.y < m_hostAttr.m_rcMargin.top)
        {
            return HTTOP;
        }
    }
    return HTCLIENT;
}

void SHostWnd::OnSetFocus( HWND wndOld )
{
    DoFrameEvent(WM_SETFOCUS,0,0);
}

void SHostWnd::OnKillFocus( HWND wndFocus )
{
    DoFrameEvent(WM_KILLFOCUS,0,0);
}

void SHostWnd::UpdateLayerFromRenderTarget(IRenderTarget *pRT,BYTE byAlpha)
{
    SASSERT(IsTranslucent());
    HDC hdc=pRT->GetDC(0);
    CRect rc;
    CSimpleWnd::GetWindowRect(&rc);
    BLENDFUNCTION bf= {AC_SRC_OVER,0,byAlpha,AC_SRC_ALPHA};
    HDC dc=GetDC();
    UpdateLayeredWindow(dc,&rc.TopLeft(),&rc.Size(),hdc,&CPoint(0,0),0,&bf,ULW_ALPHA);
    ReleaseDC(dc);
    pRT->ReleaseDC(hdc);
}
/*
BOOL _BitBlt(IRenderTarget *pRTDst,IRenderTarget * pRTSrc,CRect rcDst,CPoint ptSrc)
{
    return S_OK == pRTDst->BitBlt(&rcDst,pRTSrc,ptSrc.x,ptSrc.y,SRCCOPY);
}
*/
BOOL _BitBlt(IRenderTarget *pRTDst,IRenderTarget * pRTSrc,CRect rcDst,CPoint ptSrc)
{
    HDC dcSrc=pRTSrc->GetDC();
    HDC dcDst=pRTDst->GetDC();
    ::BitBlt(dcDst,rcDst.left,rcDst.top,rcDst.Width(),rcDst.Height(),dcSrc,ptSrc.x,ptSrc.y,SRCCOPY);
    pRTDst->ReleaseDC(dcDst);
    pRTSrc->ReleaseDC(dcSrc);
    return TRUE;
}


BOOL SHostWnd::AnimateHostWindow(DWORD dwTime,DWORD dwFlags)
{
    if(!IsTranslucent())
    {
        return ::AnimateWindow(m_hWnd,dwTime,dwFlags);
    }else
    {
        CRect rcWnd;//窗口矩形
        CSimpleWnd::GetClientRect(&rcWnd);
        CRect rcShow(rcWnd);//动画过程中可见部分
        
        CAutoRefPtr<IRenderTarget> pRT;
        GETRENDERFACTORY->CreateRenderTarget(&pRT,rcShow.Width(),rcShow.Height());

        int nSteps=dwTime/10;
        if(dwFlags & AW_HIDE)
        {
            if(dwFlags& AW_SLIDE)
            {
                LONG  x1 = rcShow.left;
                LONG  x2 = rcShow.left;
                LONG  y1 = rcShow.top;
                LONG  y2 = rcShow.top;
                LONG * x =&rcShow.left;
                LONG * y =&rcShow.top;

                if(dwFlags & AW_HOR_POSITIVE)
                {//left->right:move left
                    x1=rcShow.left,x2=rcShow.right;
                    x=&rcShow.left;
                }else if(dwFlags & AW_HOR_NEGATIVE)
                {//right->left:move right
                    x1=rcShow.right,x2=rcShow.left;
                    x=&rcShow.right;
                }
                if(dwFlags & AW_VER_POSITIVE)
                {//top->bottom
                    y1=rcShow.top,y2=rcShow.bottom;
                    y=&rcShow.top;
                }else if(dwFlags & AW_VER_NEGATIVE)
                {//bottom->top
                    y1=rcShow.bottom,y2=rcShow.top;
                    y=&rcShow.bottom;
                }
                LONG xStepLen=(x2-x1)/nSteps;
                LONG yStepLen=(y2-y1)/nSteps;

                for(int i=0;i<nSteps;i++)
                {
                    *x+=xStepLen;
                    *y+=yStepLen;
                    pRT->FillSolidRect(rcWnd,0);
                    CPoint ptAnchor;
                    if(dwFlags & AW_VER_NEGATIVE)
                        ptAnchor.y=rcWnd.bottom-rcShow.Height();
                    if(dwFlags & AW_HOR_NEGATIVE)
                        ptAnchor.x=rcWnd.right-rcShow.Width();
                    _BitBlt(pRT,m_memRT,rcShow,ptAnchor);
                    UpdateLayerFromRenderTarget(pRT,0xFF);
                    Sleep(10);
                }
                ShowWindow(SW_HIDE);
                return TRUE;
            }else if(dwFlags&AW_CENTER)
            {
                int xStep=rcShow.Width()/(2*nSteps);
                int yStep=rcShow.Height()/(2*nSteps);
                for(int i=0;i<nSteps;i++)
                {
                    rcShow.DeflateRect(xStep,yStep);
                    pRT->FillSolidRect(rcWnd,0);
                    _BitBlt(pRT,m_memRT,rcShow,rcShow.TopLeft());
                    UpdateLayerFromRenderTarget(pRT,0xFF);
                    Sleep(10);
                }
                ShowWindow(SW_HIDE);
                return TRUE;
            }else if(dwFlags&AW_BLEND)
            {
                BYTE byAlpha=255;
                for(int i=0;i<nSteps;i++)
                {
                    byAlpha-=255/nSteps;
                    UpdateLayerFromRenderTarget(m_memRT,byAlpha);
                    Sleep(10);
                }
                ShowWindow(SW_HIDE);
                return TRUE;
            }
            return FALSE;
        }else
        {
            if(!IsWindowVisible())
            {
                SetWindowPos(0,0,0,0,0,SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOZORDER|SWP_NOSIZE);
            }
            SetWindowPos(HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
            if(dwFlags& AW_SLIDE)
            {
                LONG  x1 = rcShow.left;
                LONG  x2 = rcShow.left;
                LONG  y1 = rcShow.top;
                LONG  y2 = rcShow.top;
                LONG * x =&rcShow.left;
                LONG * y =&rcShow.top;
                
                if(dwFlags & AW_HOR_POSITIVE)
                {//left->right:move right
                    x1=rcShow.left,x2=rcShow.right;
                    rcShow.right=rcShow.left,x=&rcShow.right;
                }else if(dwFlags & AW_HOR_NEGATIVE)
                {//right->left:move left
                    x1=rcShow.right,x2=rcShow.left;
                    rcShow.left=rcShow.right,x=&rcShow.left;
                }
                if(dwFlags & AW_VER_POSITIVE)
                {//top->bottom
                    y1=rcShow.top,y2=rcShow.bottom;
                    rcShow.bottom=rcShow.top,y=&rcShow.bottom;
                }else if(dwFlags & AW_VER_NEGATIVE)
                {//bottom->top
                    y1=rcShow.bottom,y2=rcShow.top;
                    rcShow.top=rcShow.bottom,y=&rcShow.top;
                }
                LONG xStepLen=(x2-x1)/nSteps;
                LONG yStepLen=(y2-y1)/nSteps;
                
                for(int i=0;i<nSteps;i++)
                {
                    *x+=xStepLen;
                    *y+=yStepLen;
                    pRT->FillSolidRect(rcWnd,0);
                    CPoint ptAnchor;
                    if(dwFlags & AW_VER_POSITIVE)
                        ptAnchor.y=rcWnd.bottom-rcShow.Height();
                    if(dwFlags & AW_HOR_POSITIVE)
                        ptAnchor.x=rcWnd.right-rcShow.Width();
                     _BitBlt(pRT,m_memRT,rcShow,ptAnchor);
                    UpdateLayerFromRenderTarget(pRT,0xFF);
                    Sleep(10);
                }
                UpdateLayerFromRenderTarget(m_memRT,0xFF);
                return TRUE;
            }else if(dwFlags&AW_CENTER)
            {
                int xStep=rcShow.Width()/(2*nSteps);
                int yStep=rcShow.Height()/(2*nSteps);
                rcShow.left=rcShow.right=(rcShow.left+rcShow.right)/2;
                rcShow.top=rcShow.bottom=(rcShow.top+rcShow.bottom)/2;
                for(int i=0;i<nSteps;i++)
                {
                    rcShow.InflateRect(xStep,yStep);
                    pRT->FillSolidRect(rcWnd,0);
                    _BitBlt(pRT,m_memRT,rcShow,rcShow.TopLeft());
                    UpdateLayerFromRenderTarget(pRT,0xFF);
                    Sleep(10);
                }
                UpdateLayerFromRenderTarget(m_memRT,0xFF);
                return TRUE;
            }else if(dwFlags&AW_BLEND)
            {
                BYTE byAlpha=0;
                for(int i=0;i<nSteps;i++)
                {
                    byAlpha+=255/nSteps;
                    UpdateLayerFromRenderTarget(m_memRT,byAlpha);
                    Sleep(10);
                }
                UpdateLayerFromRenderTarget(m_memRT,255);
                return TRUE;
            }
        }
        return FALSE;
    }
}

void SHostWnd::OnSetCaretValidateRect( LPCRECT lpRect )
{
    m_rcValidateCaret=lpRect;
}

BOOL SHostWnd::RegisterTimelineHandler( ITimelineHandler *pHandler )
{
    BOOL bRet = SwndContainerImpl::RegisterTimelineHandler(pHandler);
    if(bRet && m_lstTimelineHandler.GetCount()==1) SWindow::SetTimer(TIMER_NEXTFRAME,10);
    return bRet;
}

BOOL SHostWnd::UnregisterTimelineHandler( ITimelineHandler *pHandler )
{
    BOOL bRet=SwndContainerImpl::UnregisterTimelineHandler(pHandler);
    if(bRet && m_lstTimelineHandler.IsEmpty()) SWindow::KillTimer(TIMER_NEXTFRAME);
    return bRet;
}

const SStringW & SHostWnd::GetHostName()
{
    return m_hostAttr.m_strName;
}

SMessageLoop * SHostWnd::GetMsgLoop()
{
    return SApplication::getSingletonPtr();
}


}//namespace SOUI
