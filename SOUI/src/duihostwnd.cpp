#include "duistd.h"

#include "duihostwnd.h"
#include "DuiTipCtrl.h"
#include "DuiCmnCtrl.h"
#include "DuiSystem.h"
#include "mybuffer.h"


namespace SOUI
{

#define TIMER_CARET	1
#define TIMER_NEXTFRAME 2

//////////////////////////////////////////////////////////////////////////
// CDuiHostWnd
//////////////////////////////////////////////////////////////////////////
CDuiHostWnd::CDuiHostWnd( LPCTSTR pszResName /*= NULL*/ )
: CDuiFrame(this)
, m_strXmlLayout(pszResName)
, m_uRetCode(0)
, m_nIdleCount(0)
, m_bExitModalLoop(FALSE)
, m_bTrackFlag(FALSE)
, m_dwDlgStyle(0)
, m_dwDlgExStyle(0)
, m_bTranslucent(FALSE)
, m_bCaretShowing(FALSE)
, m_hBmpCaret(NULL)
, m_bCaretActive(FALSE)
, m_bNeedRepaint(FALSE)
, m_bNeedAllRepaint(TRUE)
, m_bResizable(FALSE)
, m_szMin(200, 200)
, m_pTipCtrl(NULL)
, m_dummyWnd(this)
{
	SetContainer(this);
}

HWND CDuiHostWnd::Create(HWND hWndParent,LPCTSTR lpWindowName, DWORD dwStyle,DWORD dwExStyle, int x, int y, int nWidth, int nHeight,LPVOID lpParam)
{
    if (NULL != m_hWnd)
        return m_hWnd;

    HWND hWnd = CSimpleWnd::Create(lpWindowName,dwStyle,dwExStyle, x,y,nWidth,nHeight,hWndParent,lpParam);
    if(!hWnd) return NULL;

    //tooltip
    m_pTipCtrl=new CDuiTipCtrl;
    m_pTipCtrl->Create(m_hWnd);

    SetContainer(this);

    if(!m_strXmlLayout.IsEmpty())  Load(m_strXmlLayout);

	if(nWidth==0 || nHeight==0) CenterWindow(hWnd);
    return hWnd;
}

HWND CDuiHostWnd::Create(HWND hWndParent,int x,int y,int nWidth,int nHeight)
{
    return Create(hWndParent, NULL,WS_POPUP | WS_CLIPCHILDREN | WS_TABSTOP,0,x,y,nWidth,nHeight,0);
}

BOOL CDuiHostWnd::Load(LPCTSTR pszXmlName)
{
	pugi::xml_document xmlDoc;
	if(!LOADXML(xmlDoc,pszXmlName,DUIRES_XML_TYPE)) return FALSE;

    return SetXml(xmlDoc.child("SOUI"));
}

BOOL CDuiHostWnd::SetXml(LPSTR lpszXml,int nLen)
{
	pugi::xml_document xmlDoc;
	if(!xmlDoc.load_buffer(lpszXml,nLen,pugi::parse_default,pugi::encoding_utf8)) return FALSE;
 
	return SetXml(xmlDoc.child("SOUI"));
}

BOOL CDuiHostWnd::SetXml(pugi::xml_node xmlNode )
{
	if(!xmlNode) return FALSE;

	m_dwDlgStyle =CSimpleWnd::GetStyle();
	m_dwDlgExStyle  = CSimpleWnd::GetExStyle();

	DuiSendMessage(WM_DESTROY);


	CSize szDefault;
	szDefault.cx = xmlNode.attribute("width").as_int(200);
	szDefault.cy = xmlNode.attribute("height").as_int(200);

	CDuiStringA strMargin=xmlNode.attribute("margin").value();
	sscanf(strMargin,"%d,%d,%d,%d",&m_rcNC.left,&m_rcNC.top,&m_rcNC.right,&m_rcNC.bottom);
	CDuiStringA strMin = xmlNode.attribute("minsize").value();
	sscanf(strMin,"%d,%d",&m_szMin.cx, &m_szMin.cy);

	m_bTranslucent=xmlNode.attribute("translucent").as_bool(false);
	m_strName = xmlNode.attribute("name").value();

	CDuiStringT strTitle=DUI_CA2T(xmlNode.attribute("title").value(),CP_UTF8);

	BUILDSTRING(strTitle);

	if(m_bTranslucent)
	{
		m_dwDlgExStyle |= WS_EX_LAYERED;
	}

	if (xmlNode.attribute("appwin").as_bool(false))
	{
		m_dwDlgExStyle |= WS_EX_APPWINDOW;
		m_dwDlgStyle |= WS_SYSMENU;
	}else if(xmlNode.attribute("toolwindow").as_bool(false))
	{
		m_dwDlgExStyle |= WS_EX_TOOLWINDOW;
	}


	m_bResizable = xmlNode.attribute("resize").as_bool(false);

	if (m_bResizable)
	{
		m_dwDlgStyle |= WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
	}

	ModifyStyle(0,m_dwDlgStyle);
	ModifyStyleEx(0,m_dwDlgExStyle);

	if(m_bTranslucent)
	{
		SetWindowLongPtr(GWL_EXSTYLE, GetWindowLongPtr(GWL_EXSTYLE) | WS_EX_LAYERED);
		m_dummyWnd.Create(_T("dummyLayeredWnd"),WS_POPUP,WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,0,0,10,10,m_hWnd,NULL);
		m_dummyWnd.SetWindowLongPtr(GWL_EXSTYLE,m_dummyWnd.GetWindowLongPtr(GWL_EXSTYLE) | WS_EX_LAYERED);
		::SetLayeredWindowAttributes(m_dummyWnd.m_hWnd,0,0,LWA_ALPHA);
		m_dummyWnd.ShowWindow(SW_SHOWNOACTIVATE);
	}


	SetWindowText(strTitle);

	CRect rcClient;
	GetClientRect(&rcClient);
	if(rcClient.IsRectEmpty())//APP没有指定窗口大小，使用XML中的值
	{
		SetWindowPos(NULL,0,0,szDefault.cx,szDefault.cy,SWP_NOZORDER|SWP_NOMOVE);
		GetClientRect(&rcClient);
	}

	if(!m_strName.IsEmpty())
	{
		DuiSystem::getSingleton().GetCurResMgr()->GetSkinPool()->LoadSkins(m_strName);	//load skin only used in the host window
	}

	CDuiWindow::Load(xmlNode.child("body"));

	Move(rcClient);
	DuiSendMessage(WM_ENABLE,1);
	DuiSendMessage(WM_SHOWWINDOW,1);

	_Redraw();

	RedrawRegion(CDCHandle(m_memDC),CRgn());

	return TRUE;
}

UINT_PTR CDuiHostWnd::DoModal(HWND hWndParent/* = NULL*/, LPRECT rect /*= NULL*/)
{
    BOOL bEnableParent = FALSE;

    if (NULL == hWndParent)
    {
        hWndParent = DuiThreadActiveWndMgr::GetActive();
        if (NULL == hWndParent)
            hWndParent = ::GetActiveWindow();
    }

    if (hWndParent && hWndParent != ::GetDesktopWindow() && ::IsWindowEnabled(hWndParent))
    {
        ::EnableWindow(hWndParent, FALSE);
        bEnableParent = TRUE;
    }

    m_bExitModalLoop = FALSE;

    CRect rc;
    if(rect) rc=*rect;
	if(!IsWindow()) Create(hWndParent, rc.left,rc.top,rc.Width(),rc.Height());
    if (!IsWindow())
    {
        ::EnableWindow(hWndParent, TRUE);
        return 0;
    }

    HWND hWndLastActive = DuiThreadActiveWndMgr::SetActive(m_hWnd);

	if(INITCODE_NOTSHOW!=SendMessage(WM_INITDIALOG, (WPARAM)m_hWnd))
	{//根据INITDIALOG的返回值还判断是不是将窗口显示出来。返回INITCODE_NOTSHOW时窗口不显示。
	
		if(m_dwDlgExStyle&WS_EX_TOOLWINDOW)
			::ShowWindow(m_hWnd,SW_SHOWNOACTIVATE);
		else
			::ShowWindow(m_hWnd,SW_SHOWNORMAL);
	}

    _ModalMessageLoop();

    m_bExitModalLoop = FALSE;

    // From MFC
    // hide the window before enabling the parent, etc.

    if ( IsWindow() )
    {
        SetWindowPos(
            NULL, 0, 0, 0, 0,
            SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
    }

    if (bEnableParent)
    {
        ::EnableWindow(hWndParent, TRUE);
    }

    if (hWndParent != NULL && ::GetActiveWindow() == m_hWnd)
        ::SetActiveWindow(hWndParent);

    DuiThreadActiveWndMgr::SetActive(hWndLastActive);

    if ( IsWindow() )
        DestroyWindow();

    return m_uRetCode;
}


void CDuiHostWnd::EndDialog(UINT uRetCode)
{
    m_uRetCode = uRetCode;

    m_bExitModalLoop = TRUE;

    // DestroyWindow里面直接Send了WM_DESTROY，所以不会跑到DoModal的消息循环里，所以有了下面那行代码
    // DestroyWindow();

    // 这句非常重要，可以让DoModal消息循环再跑一次，防止卡死在GetMessage，泪奔~~~~~~~
    ::PostThreadMessage(::GetCurrentThreadId(), WM_NULL, 0, 0);
}


BOOL CDuiHostWnd::OnIdle(int nCount)
{
    return FALSE;
}

void CDuiHostWnd::_ModalMessageLoop()
{
    BOOL bRet;
    MSG msg;

    for(;;)
    {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            if (WM_QUIT == msg.message)
                break;
        }
        else
        {
            OnIdle(m_nIdleCount++);
        }

        if (m_bExitModalLoop || NULL == m_hWnd || !::IsWindow(m_hWnd))
            break;

        bRet = ::GetMessage(&msg, NULL, 0, 0);

        if (bRet == -1)
        {
            continue;   // error, don't process
        }
        else if (!bRet)
        {
            break;   // WM_QUIT, exit message loop
        }
        if (!_PreTranslateMessage(&msg))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }
}


void CDuiHostWnd::_Redraw()
{
    m_bNeedAllRepaint = TRUE;
    m_bNeedRepaint = TRUE;
    if (!m_rgnInvalidate.IsNull())
        m_rgnInvalidate.DeleteObject();

    if(!m_bTranslucent)
        Invalidate(FALSE);
	else if(m_dummyWnd.IsWindow()) 
		m_dummyWnd.Invalidate(FALSE);
}

BOOL CDuiHostWnd::_PreTranslateMessage(MSG* pMsg)
{
    if(m_pTipCtrl  && m_pTipCtrl->IsWindow()) m_pTipCtrl->RelayEvent(pMsg);

    // loop backwards
    for(int i = m_aMsgFilter.GetCount() - 1; i >= 0; i--)
    {
        CDuiMessageFilter* pMessageFilter = m_aMsgFilter[i];
        if(pMessageFilter != NULL && pMessageFilter->PreTranslateMessage(pMsg))
            return TRUE;
    }
    return FALSE;   // not translated
}


void CDuiHostWnd::OnPrint(CDCHandle dc, UINT uFlags)
{
    if((m_bTranslucent && !uFlags) && !m_bNeedAllRepaint && !m_bNeedRepaint) return;
    if (m_bNeedAllRepaint)
    {
        if (!m_rgnInvalidate.IsNull())
            m_rgnInvalidate.DeleteObject();

        m_bNeedAllRepaint = FALSE;
        m_bNeedRepaint=TRUE;
    }


    if (m_bNeedRepaint)
    {
        DuiThreadActiveWndMgr::EnterPaintLock();

        HFONT hftOld = m_memDC.SelectFont(DuiFontPool::getSingleton().GetFont(DUIF_DEFAULTFONT));

        m_memDC.SetBkMode(TRANSPARENT);
        m_memDC.SetTextColor(0);

        CRect rcInvalid=m_rcWindow;
        if (!m_rgnInvalidate.IsNull())
        {
            m_memDC.SelectClipRgn(m_rgnInvalidate);
            m_rgnInvalidate.GetRgnBox(&rcInvalid);
        }
        else
        {
            m_memDC.SelectClipRgn(NULL);
        }

        m_memDC.FillSolidRect(&rcInvalid,0);//清除残留的alpha值

        //m_rgnInvalidate有可能在RedrawRegion时被修改，必须生成一个临时的区域对象
        CRgn rgnUpdate;
        if(m_rgnInvalidate)
        {
            rgnUpdate.Attach(m_rgnInvalidate.Detach());
        }
		if(m_bCaretActive) DrawCaret(m_ptCaret);//clear old caret 
        RedrawRegion(CDCHandle(m_memDC), rgnUpdate);
		if(m_bCaretActive) DrawCaret(m_ptCaret);//redraw caret 
        m_memDC.SelectClipRgn(NULL);

        m_memDC.SelectFont(hftOld);

        m_bNeedRepaint = FALSE;
        DuiThreadActiveWndMgr::LeavePaintLock();
    }

    CRect rc;
    GetClientRect(&rc);
    UpdateHost(dc,rc);
}

void CDuiHostWnd::OnPaint(CDCHandle dc)
{
	CPaintDC dc1(m_hWnd);
    OnPrint(m_bTranslucent?NULL:dc1.m_hDC, 0);
}

BOOL CDuiHostWnd::OnEraseBkgnd(CDCHandle dc)
{
    return TRUE;
}


int CDuiHostWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    CDCHandle dc=::GetDC(NULL);
    m_memDC.CreateCompatibleDC(dc);
    ::ReleaseDC(NULL,dc);
    return 0;
}

void CDuiHostWnd::OnDestroy()
{
    if(m_pTipCtrl)
    {
        if (m_pTipCtrl->IsWindow())
            m_pTipCtrl->DestroyWindow();
        delete m_pTipCtrl;
    }
	if(m_bTranslucent && m_dummyWnd.IsWindow())
	{
		m_dummyWnd.DestroyWindow();
	}

    DuiSendMessage(WM_DESTROY);

    if(::GetCurrentObject(m_memDC,OBJ_BITMAP))
    {
        HBITMAP hBmp=m_memDC.SelectBitmap(NULL);
        DeleteObject(hBmp);
    }
    m_memDC.DeleteDC();

    if(!m_strName.IsEmpty())
    {
        DuiSystem::getSingleton().GetCurResMgr()->GetSkinPool()->FreeSkins(m_strName);
    }
}

void CDuiHostWnd::OnSize(UINT nType, CSize size)
{
    if(IsIconic()) return;

    if (size.cx==0 || size.cy==0)
        return;

    if(m_memDC.HasBitmap())
    {
        HBITMAP hBmp=m_memDC.SelectBitmap(NULL);
        DeleteObject(hBmp);
    }


    CDCHandle dcDesk=::GetDC(NULL);
    m_memDC.SelectBitmap(CGdiAlpha::CreateBitmap32(dcDesk,size.cx,size.cy));
    ::ReleaseDC(NULL,dcDesk);

    CRect rcClient;
    GetClientRect(rcClient);

    Move(rcClient);

    _Redraw();

    SetMsgHandled(FALSE);//交给其它处理器继续处理
}

void CDuiHostWnd::OnMouseMove(UINT nFlags, CPoint point)
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

void CDuiHostWnd::OnMouseLeave()
{
    m_bTrackFlag = FALSE;
    DoFrameEvent(WM_MOUSELEAVE,0,0);
}

void CDuiHostWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetFocus();
    DoFrameEvent(WM_LBUTTONDOWN,nFlags,MAKELPARAM(point.x,point.y));
}

void CDuiHostWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    DoFrameEvent(WM_LBUTTONDBLCLK,nFlags,MAKELPARAM(point.x,point.y));
}

BOOL CDuiHostWnd::OnSetCursor(HWND hwnd, UINT nHitTest, UINT message)
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

void CDuiHostWnd::OnTimer(UINT_PTR idEvent)
{
    CDuiTimerID duiTimerID((DWORD)idEvent);
    if(duiTimerID.bDuiTimer)
    {
        CDuiWindow *pDuiWnd=DuiWindowMgr::GetWindow((HDUIWND)duiTimerID.hDuiWnd);
        if(pDuiWnd)
        {
            if(pDuiWnd==this) OnDuiTimer(duiTimerID.uTimerID);//由于DUIWIN采用了ATL一致的消息映射表模式，因此在HOST中不能有DUI的消息映射表（重复会导致SetMsgHandled混乱)
            else pDuiWnd->DuiSendMessage(WM_TIMER,duiTimerID.uTimerID,0);
        }
        else
        {
            //窗口已经删除，自动清除该窗口的定时器
            KillTimer(idEvent);
        }
    }
    else
    {
        SetMsgHandled(FALSE);
    }
}

void CDuiHostWnd::OnDuiTimer( char cTimerID )
{
    if(cTimerID==TIMER_CARET)
    {
		DUIASSERT(m_bCaretShowing);
        DrawCaret(m_ptCaret);
		m_bCaretActive=!m_bCaretActive;
    }else if(cTimerID==TIMER_NEXTFRAME)
	{
		OnNextFrame();
	}
}

void CDuiHostWnd::DrawCaret(CPoint pt)
{
    BITMAP bm;
    GetObject(m_hBmpCaret,sizeof(bm),&bm);

    CMemDC dcCaret(m_memDC,m_hBmpCaret);
	
	CRect rcCaret(pt,CSize(bm.bmWidth,bm.bmHeight));
	CRect rcShowCaret;
	rcShowCaret.IntersectRect(m_rcValidateCaret,rcCaret);
	

    ALPHAINFO ai;
    CGdiAlpha::AlphaBackup(m_memDC,rcCaret,ai);
	m_memDC.BitBlt(rcShowCaret.left,rcShowCaret.top,rcShowCaret.Width(),rcShowCaret.Height(),dcCaret,rcShowCaret.left-pt.x,rcShowCaret.top-pt.y,DSTINVERT);
    CGdiAlpha::AlphaRestore(m_memDC,ai);

	if(!m_bTranslucent)
	{
		InvalidateRect(rcCaret, FALSE);
	}else
	{
		if(m_dummyWnd.IsWindow()) 
			m_dummyWnd.Invalidate(FALSE);
	}
}

LRESULT CDuiHostWnd::OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DoFrameEvent(uMsg,wParam,lParam);	//将鼠标消息转发到DuiWindow处理

    if(m_pTipCtrl && m_pTipCtrl->IsWindow())
    {
        CDuiWindow *pHover=DuiWindowMgr::GetWindow(m_hHover);
        if(!pHover || pHover->IsDisabled(TRUE))
        {
            m_pTipCtrl->ShowTip(FALSE);
        }
        else
        {
            HDUIWND hNewTipHost=0;
            CRect rcTip;
            CDuiStringT strTip;
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

LRESULT CDuiHostWnd::OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg==WM_SYSKEYDOWN || uMsg==WM_SYSKEYUP)
    {
        CDuiWindow *pFocus=DuiWindowMgr::GetWindow(m_focusMgr.GetFocusedHwnd());
        if(!pFocus  || !(pFocus->OnGetDuiCode()&DUIC_WANTSYSKEY))
        {
			SetMsgHandled(FALSE);
            return 0;
        }
    }
    LRESULT lRet = DoFrameEvent(uMsg,wParam,lParam);
	SetMsgHandled(CDuiWindow::IsMsgHandled());
	return lRet;
}

BOOL CDuiHostWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    ScreenToClient(&pt);
    return DoFrameEvent(WM_MOUSEWHEEL,MAKEWPARAM(nFlags,zDelta),MAKELPARAM(pt.x,pt.y))!=0;
}


void CDuiHostWnd::OnActivate( UINT nState, BOOL bMinimized, HWND wndOther )
{
	if(nState==WA_ACTIVE)
		::SetFocus(m_hWnd);
	else
		::SetFocus(NULL);
}

LRESULT CDuiHostWnd::OnDuiNotify(LPDUINMHDR pHdr)
{
    if(pHdr->code==NM_REALWND_CREATE) return (LRESULT)OnRealWndCreate(((LPDUINMREALWNDCMN)pHdr)->pRealWnd);
    else if(pHdr->code==NM_REALWND_INIT) return OnRealWndInit(((LPDUINMREALWNDCMN)pHdr)->pRealWnd);
    else if(pHdr->code==NM_REALWND_DESTROY)
    {
        OnRealWndDestroy(((LPDUINMREALWNDCMN)pHdr)->pRealWnd);
        return 0;
    }
    else if(pHdr->code==NM_REALWND_SIZE)
    {
        OnRealWndSize(((LPDUINMREALWNDCMN)pHdr)->pRealWnd);
        return 0;
    }

    return SendMessage(UM_DUI_NOTIFY,IDC_RICHVIEW_WIN,(LPARAM)pHdr);
}

CRect CDuiHostWnd::GetContainerRect()
{
    return m_rcWindow;
}

HWND CDuiHostWnd::GetHostHwnd()
{
    return m_hWnd;
}

HDC CDuiHostWnd::OnGetDuiDC(const CRect & rc,DWORD gdcFlags)
{
	m_memDC.SelectFont(DuiFontPool::getSingleton().GetFont(DUIF_DEFAULTFONT));
	m_memDC.SetBkMode(TRANSPARENT);
	m_memDC.SetTextColor(0);

    if(!(gdcFlags & OLEDC_NODRAW))
    {
		if(m_bCaretActive)
		{
			DrawCaret(m_ptCaret);//clear old caret
		}
        CRgn rgnRc;
        rgnRc.CreateRectRgnIndirect(&rc);
        m_memDC.SelectClipRgn(rgnRc);
    }
    return m_memDC;
}

void CDuiHostWnd::OnReleaseDuiDC(HDC hdcSour,const CRect &rc,DWORD gdcFlags)
{
	m_memDC.SelectClipRgn(NULL);
    if(gdcFlags & OLEDC_NODRAW) return;
	if(m_bCaretActive)
	{
		DrawCaret(m_ptCaret);//clear old caret
	}
    CDCHandle dc=GetDC();
    UpdateHost(dc,rc);
    ReleaseDC(dc);
}

void CDuiHostWnd::UpdateHost(CDCHandle dc, const CRect &rcInvalid )
{
	if(m_bTranslucent)
	{
		CRect rc;
		GetWindowRect(&rc);
		BLENDFUNCTION bf= {AC_SRC_OVER,0,0xFF,AC_SRC_ALPHA};
		CDCHandle hdcSrc=::GetDC(NULL);
		UpdateLayeredWindow(hdcSrc,&rc.TopLeft(),&rc.Size(),m_memDC,&CPoint(0,0),0,&bf,ULW_ALPHA);
		::ReleaseDC(NULL,hdcSrc);
	}
	else
	{
		dc.BitBlt(rcInvalid.left,rcInvalid.top,rcInvalid.Width(),rcInvalid.Height(),m_memDC,rcInvalid.left,rcInvalid.top,SRCCOPY);
	}
}

void CDuiHostWnd::OnRedraw(const CRect &rc)
{
    if(!IsWindow()) return;

    if (m_rgnInvalidate.IsNull())
    {
        m_rgnInvalidate.CreateRectRgnIndirect(rc);
    }
    else
    {
        CRgn rgnInvalidate;

        rgnInvalidate.CreateRectRgnIndirect(rc);

        m_rgnInvalidate.CombineRgn(rgnInvalidate, RGN_OR);
    }
    m_bNeedRepaint = TRUE;

    if(!m_bTranslucent)
    {
        InvalidateRect(rc, FALSE);
    }else
	{
		if(m_dummyWnd.IsWindow()) 
			m_dummyWnd.Invalidate(FALSE);
	}
}

BOOL CDuiHostWnd::OnReleaseDuiCapture()
{
    if(!__super::OnReleaseDuiCapture()) return FALSE;
    ReleaseCapture();
    CPoint pt;
    GetCursorPos(&pt);
    ScreenToClient(&pt);
    PostMessage(WM_MOUSEMOVE,0,MAKELPARAM(pt.x,pt.y));
    return TRUE;
}

HDUIWND CDuiHostWnd::OnSetDuiCapture(HDUIWND hDuiWnd)
{
    SetCapture();
    return __super::OnSetDuiCapture(hDuiWnd);
}

HDUIWND CDuiHostWnd::GetDuiCapture()
{
    if(GetCapture()!=m_hWnd) return NULL;
    return __super::OnGetDuiCapture();
}

BOOL CDuiHostWnd::IsTranslucent()
{
    return m_bTranslucent;
}


BOOL CDuiHostWnd::DuiCreateCaret( HBITMAP hBmp,int nWidth,int nHeight )
{
	::CreateCaret(m_hWnd,hBmp,nWidth,nHeight);
	CDCHandle dc=GetDC();
	if(m_hBmpCaret)
	{
		DeleteObject(m_hBmpCaret);
		m_hBmpCaret=NULL;
	}
	m_hBmpCaret=CreateCompatibleBitmap(dc,nWidth,nHeight);
	CDCHandle memdc1,memdc2;
	memdc1.CreateCompatibleDC(dc);
	memdc1.SelectBitmap(m_hBmpCaret);
	if(hBmp)
	{
		//以拉伸方式创建一个插入符位图
		memdc2.CreateCompatibleDC(dc);
		memdc2.SelectBitmap(hBmp);
		BITMAP bm;
		GetObject(hBmp,sizeof(bm),&bm);
		StretchBlt(memdc1,0,0,nWidth,nHeight,memdc2,0,0,bm.bmWidth,bm.bmHeight,SRCCOPY);
		memdc2.DeleteDC();
	}
	else
	{
		//创建一个黑色插入符的位图
		memdc1.FillSolidRect(0,0,nWidth,nHeight,0);
	}
	memdc1.DeleteDC();
	ReleaseDC(dc);
    return TRUE;
}

BOOL CDuiHostWnd::DuiShowCaret( BOOL bShow )
{
    m_bCaretShowing=bShow;

	if(bShow)
	{
		SetDuiTimer(TIMER_CARET,GetCaretBlinkTime());
		if(!m_bCaretActive)
		{
			DrawCaret(m_ptCaret);
			m_bCaretActive=TRUE;
		}
	}
	else
	{
		KillDuiTimer(TIMER_CARET);
		if(m_bCaretActive)
		{
			DrawCaret(m_ptCaret);
		}
		m_bCaretActive=FALSE;
	}
   return TRUE;
}

BOOL CDuiHostWnd::DuiSetCaretPos( int x,int y )
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


BOOL CDuiHostWnd::DuiUpdateWindow()
{
	if(m_bTranslucent) UpdateWindow(m_dummyWnd.m_hWnd);
	else UpdateWindow(m_hWnd);
	return TRUE;
}

LRESULT CDuiHostWnd::OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
{
    if (bCalcValidRects && (CSimpleWnd::GetStyle() & WS_POPUP))
    {
        CRect rcWindow;
        GetWindowRect(rcWindow);

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

void CDuiHostWnd::OnGetMinMaxInfo(LPMINMAXINFO lpMMI)
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
        lpMMI->ptMinTrackSize = CPoint(m_szMin.cx, m_szMin.cy);
    }
}

BOOL CDuiHostWnd::OnNcActivate(BOOL bActive)
{
    return TRUE;
}

UINT CDuiHostWnd::OnWndNcHitTest(CPoint point)
{
    if (m_bResizable)
    {
        ScreenToClient(&point);
        if (point.x > m_rcWindow.right - m_rcNC.right)
        {
            if (point.y > m_rcWindow.bottom - m_rcNC.bottom)
            {
                return HTBOTTOMRIGHT;
            }
            else if (point.y < m_rcNC.top)
            {
                return HTTOPRIGHT;
            }
            return HTRIGHT;
        }
        else if (point.x < m_rcNC.left)
        {
            if (point.y > m_rcWindow.bottom - m_rcNC.bottom)
            {
                return HTBOTTOMLEFT;
            }
            else if (point.y < m_rcNC.top)
            {
                return HTTOPLEFT;
            }
            return HTLEFT;
        }
        else if (point.y > m_rcWindow.bottom - m_rcNC.bottom)
        {
            return HTBOTTOM;
        }
        else if (point.y < m_rcNC.top)
        {
            return HTTOP;
        }
    }
    return HTCLIENT;
}

void CDuiHostWnd::OnClose()
{
    EndDialog(IDCANCEL);
}

void CDuiHostWnd::OnOK()
{
	EndDialog(IDOK);
}

LRESULT CDuiHostWnd::OnMsgFilter(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    CDuiMessageFilter *pMsgFilter=(CDuiMessageFilter*)lParam;
    if(wParam)
    {
        m_aMsgFilter.Add(pMsgFilter);
    }
    else
    {
        UINT i = 0;
        while(i < m_aMsgFilter.GetCount())
        {
            if(m_aMsgFilter.GetAt(i) == pMsgFilter)
            {
                m_aMsgFilter.RemoveAt(i);
                break;
            }
            i++;
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
// IDuiRealWnd
HWND CDuiHostWnd::OnRealWndCreate(CDuiRealWnd *pRealWnd)
{
    CRect rcWindow;
    UINT uCmdID=pRealWnd->GetCmdID();
    pRealWnd->GetRect(&rcWindow);

    const CDuiRealWndParam & paramRealWnd=pRealWnd->GetRealWndParam();
    return CreateWindowEx(paramRealWnd.m_dwExStyle,paramRealWnd.m_strClassName,paramRealWnd.m_strWindowName,paramRealWnd.m_dwStyle,
                          rcWindow.left,rcWindow.top,rcWindow.Width(),rcWindow.Height(),
                          m_hWnd,(HMENU)(ULONG_PTR)uCmdID,0,NULL);
}

BOOL CDuiHostWnd::OnRealWndInit( CDuiRealWnd *pRealWnd )
{
    return FALSE;
}

void CDuiHostWnd::OnRealWndDestroy(CDuiRealWnd *pRealWnd)
{
    if(::IsWindow(pRealWnd->GetRealHwnd(FALSE)))
    {
        ::DestroyWindow(pRealWnd->GetRealHwnd(FALSE));
    }
    if(pRealWnd->GetData())
    {
        delete pRealWnd->GetData();
        pRealWnd->SetData(0);
    }
}


void CDuiHostWnd::OnRealWndSize( CDuiRealWnd *pRealWnd )
{
    if(::IsWindow(pRealWnd->GetRealHwnd(FALSE)))
    {
        CRect rcClient;
        pRealWnd->GetClient(&rcClient);
        ::SetWindowPos(pRealWnd->GetRealHwnd(FALSE),0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER);
    }
}

void CDuiHostWnd::OnSetFocus( HWND wndOld )
{
	DoFrameEvent(WM_SETFOCUS,0,0);
}

void CDuiHostWnd::OnKillFocus( HWND wndFocus )
{
	DoFrameEvent(WM_KILLFOCUS,0,0);
}

void CDuiHostWnd::UpdateLayerFromDC(HDC hdc,BYTE byAlpha)
{
	DUIASSERT(IsTranslucent());
	CRect rc;
	GetWindowRect(&rc);
	BLENDFUNCTION bf= {AC_SRC_OVER,0,byAlpha,AC_SRC_ALPHA};
	CDCHandle dc=GetDC();
	UpdateLayeredWindow(dc,&rc.TopLeft(),&rc.Size(),hdc,&CPoint(0,0),0,&bf,ULW_ALPHA);
	ReleaseDC(dc);
}

BOOL _BitBlt(HDC hDst,HDC hSrc,CRect rcDst,CPoint ptSrc)
{
	return BitBlt(hDst,rcDst.left,rcDst.top,rcDst.Width(),rcDst.Height(),hSrc,ptSrc.x,ptSrc.y,SRCCOPY);
}

BOOL CDuiHostWnd::AnimateHostWindow(DWORD dwTime,DWORD dwFlags)
{
	if(!IsTranslucent())
	{
		return ::AnimateWindow(m_hWnd,dwTime,dwFlags);
	}else
	{
		CRect rcWnd;//窗口矩形
		GetClientRect(&rcWnd);
		CRect rcShow(rcWnd);//动画过程中可见部分
		HBITMAP hBmp=CGdiAlpha::CreateBitmap32(m_memDC,rcShow.Width(),rcShow.Height(),NULL,0);
		CMemDC memdc(m_memDC,hBmp);

		memdc.SetBitmapOwner(TRUE); 

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
					memdc.FillSolidRect(rcWnd,0);
					CPoint ptAnchor;
					if(dwFlags & AW_VER_NEGATIVE)
						ptAnchor.y=rcWnd.bottom-rcShow.Height();
					if(dwFlags & AW_HOR_NEGATIVE)
						ptAnchor.x=rcWnd.right-rcShow.Width();
					_BitBlt(memdc,m_memDC,rcShow,ptAnchor);
					UpdateLayerFromDC(memdc,0xFF);
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
					memdc.FillSolidRect(rcWnd,0);
					_BitBlt(memdc,m_memDC,rcShow,rcShow.TopLeft());
					UpdateLayerFromDC(memdc,0xFF);
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
					UpdateLayerFromDC(m_memDC,byAlpha);
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
					memdc.FillSolidRect(rcWnd,0);
					CPoint ptAnchor;
					if(dwFlags & AW_VER_POSITIVE)
						ptAnchor.y=rcWnd.bottom-rcShow.Height();
					if(dwFlags & AW_HOR_POSITIVE)
						ptAnchor.x=rcWnd.right-rcShow.Width();
 					_BitBlt(memdc,m_memDC,rcShow,ptAnchor);
					UpdateLayerFromDC(memdc,0xFF);
					Sleep(10);
				}
				UpdateLayerFromDC(m_memDC,0xFF);
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
					memdc.FillSolidRect(rcWnd,0);
					_BitBlt(memdc,m_memDC,rcShow,rcShow.TopLeft());
					UpdateLayerFromDC(memdc,0xFF);
					Sleep(10);
				}
				UpdateLayerFromDC(m_memDC,0xFF);
				return TRUE;
			}else if(dwFlags&AW_BLEND)
			{
				BYTE byAlpha=0;
				for(int i=0;i<nSteps;i++)
				{
					byAlpha+=255/nSteps;
					UpdateLayerFromDC(m_memDC,byAlpha);
					Sleep(10);
				}
				UpdateLayerFromDC(m_memDC,255);
				return TRUE;
			}
		}
		return FALSE;
	}
}

void CDuiHostWnd::OnSetCaretValidateRect( LPCRECT lpRect )
{
	m_rcValidateCaret=lpRect;
}

BOOL CDuiHostWnd::RegisterTimelineHandler( ITimelineHandler *pHandler )
{
	BOOL bRet = CDuiFrame::RegisterTimelineHandler(pHandler);
	if(bRet && m_lstTimelineHandler.GetCount()==1) SetDuiTimer(TIMER_NEXTFRAME,10);
	return bRet;
}

BOOL CDuiHostWnd::UnregisterTimelineHandler( ITimelineHandler *pHandler )
{
	BOOL bRet=CDuiFrame::UnregisterTimelineHandler(pHandler);
	if(bRet && m_lstTimelineHandler.IsEmpty()) KillDuiTimer(TIMER_NEXTFRAME);
	return bRet;
}

//////////////////////////////////////////////////////////////////////////
//	CTranslucentHostWnd
//////////////////////////////////////////////////////////////////////////
void CTranslucentHostWnd::OnPaint( CDCHandle dc )
{
	CPaintDC dc1(m_hWnd);
	m_pOwner->OnPrint(NULL,1);
}

}//namespace SOUI
