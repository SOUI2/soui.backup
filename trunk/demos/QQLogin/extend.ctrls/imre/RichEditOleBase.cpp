#include "stdafx.h"
#include "souistd.h"
#include <richole.h>
#include "RichEditOleBase.h"
#include "helper\SplitString.h"

namespace SOUI
{

    //////////////////////////////////////////////////////////////////////////
    // OleWindow
    BOOL OleWindow::m_bTiemrRegistered;
    OleTimerHandler OleWindow::m_timerHandler;

    OleWindow::OleWindow() :m_bDelayDraw(FALSE)
    {
        static int nIDGen = 0;
        m_nWindowID = nIDGen++;
    }

    OleWindow:: ~OleWindow()
    {

    }

    BOOL OleWindow::OnFireEvent(EventArgs &evt)
    {
        return FALSE;
    }

    CRect OleWindow::GetContainerRect()
    {
        return m_rcOleWindow;
    }

    HWND OleWindow::GetHostHwnd()
    {
        return m_pHostContainer->GetHostHwnd();
    }

    const SStringW & OleWindow::GetTranslatorContext()
    {
        return m_pHostContainer->GetTranslatorContext();
    }

    IRenderTarget * OleWindow::OnGetRenderTarget(const CRect & rc, DWORD gdcFlags)
    {
        return m_pHostContainer->OnGetRenderTarget(rc, gdcFlags);
    }

    void OleWindow::OnReleaseRenderTarget(IRenderTarget * pRT, const CRect &rc, DWORD gdcFlags)
    {
        m_pHostContainer->OnReleaseRenderTarget(pRT, rc, gdcFlags);
    }

    void OleWindow::RealDraw(CRect rc)
    {
        m_pHostRichEdit->DirectDraw(rc);
    }

    void OleWindow::OnRedraw(const CRect &rc)
    {
        if (m_rcOleWindow.IsRectNull())
        {
            return;
        }

        if (m_bDelayDraw)
        {
            m_pHostRichEdit->DelayDraw(m_rcOleWindow);
        }
        else
        {
            m_pHostRichEdit->DirectDraw(m_rcOleWindow);
        }
    }

    BOOL OleWindow::IsTranslucent() const
    {
        return TRUE;
    }

    BOOL OleWindow::IsSendWheel2Hover() const
    {
        return TRUE;
    }

    BOOL OleWindow::UpdateWindow()
    {
        return m_pHostContainer->UpdateWindow();
    }

    BOOL OleWindow::RegisterTimelineHandler(ITimelineHandler *pHandler)
    {
        if (!m_bTiemrRegistered)
        {
            m_pHostContainer->RegisterTimelineHandler(&m_timerHandler);
            m_bTiemrRegistered = TRUE;
        }

        //return m_pHostContainer->RegisterTimelineHandler(pHandler);
        return m_timerHandler.RegisterHandler(pHandler);
    }

    BOOL OleWindow::UnregisterTimelineHandler(ITimelineHandler *pHandler)
    {
        //return m_pHostContainer->UnregisterTimelineHandler(pHandler);
        m_bTiemrRegistered = FALSE;
        return m_timerHandler.UnregisterHandler(pHandler);
    }

    SMessageLoop * OleWindow::GetMsgLoop()
    {
        return m_pHostContainer->GetMsgLoop();
    }

    LRESULT OleWindow::HandleEvent(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return DoFrameEvent(msg, wParam, lParam);
    }

    void OleWindow::SetHostRichEdit(IRichEditObjHost * pRichEdit)
    {
        m_pHostRichEdit = pRichEdit;
        m_pHostContainer = pRichEdit->GetHostContainer();
    }

    //////////////////////////////////////////////////////////////////////////
    // RichEditOleBase

    RichEditOleBase::RichEditOleBase() :_canBeSelect(TRUE)
        //,m_dwRef(0)
    {
        _sizeNatural.cx = _sizeNatural.cy = 0;
        _sizeExtent.cx = _sizeExtent.cy = 0;
    }

    RichEditOleBase::~RichEditOleBase()
    {
        _oleView.SSendMessage(WM_DESTROY);
    }

    //
    // IUnknown methods
    //
    HRESULT RichEditOleBase::QueryInterface(REFIID riid, void ** ppvObject)
    {
        if (::IsEqualIID(riid, IID_IUnknown) || ::IsEqualIID(riid, IID_IOleObject))
        {
            *ppvObject = static_cast<IOleObject*>(this);
        }
        else if (::IsEqualIID(riid, IID_IViewObject))
        {
            *ppvObject = static_cast<IViewObject*>(this);
        }
        else if (::IsEqualIID(riid, IID_IViewObject2))
        {
            *ppvObject = static_cast<IViewObject2*>(this);
        }
        //else if(::IsEqualIID(riid, m_guidOle))
        //{
        //    *ppvObject = static_cast<IOleObject*>(this);
        //}
        else
        {
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    ULONG RichEditOleBase::AddRef(void)
    {
        return RichEditObj::AddRef();
    }

    ULONG RichEditOleBase::Release(void)
    {
        return RichEditObj::Release();
    }

    //
    // IOleObject methods
    //
    HRESULT RichEditOleBase::Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection)
    {
        HRESULT hr = S_OK;
        if (_spOleAdviseHolder == NULL)
            hr = CreateOleAdviseHolder(&_spOleAdviseHolder);
        if (SUCCEEDED(hr))
            hr = _spOleAdviseHolder->Advise(pAdvSink, pdwConnection);
        return hr;
    }

    HRESULT RichEditOleBase::Unadvise(DWORD dwConnection)
    {
        HRESULT hRes = E_FAIL;
        if (_spOleAdviseHolder != NULL)
            hRes = _spOleAdviseHolder->Unadvise(dwConnection);
        return hRes;
    }

    HRESULT RichEditOleBase::SetClientSite(IOleClientSite *pClientSite)
    {
        _spClientSite = pClientSite;
        return S_OK;
    }

    HRESULT RichEditOleBase::GetClientSite(IOleClientSite **ppClientSite)
    {
        SASSERT(ppClientSite);
        if (ppClientSite == NULL)
            return E_POINTER;

        *ppClientSite = _spClientSite;
        if (_spClientSite != NULL)
            _spClientSite.p->AddRef();
        return S_OK;
    }

    HRESULT RichEditOleBase::GetExtent(DWORD dwDrawAspect, SIZEL *psizel)
    {
        if (dwDrawAspect != DVASPECT_CONTENT)
            return E_FAIL;

        if (psizel == NULL)
            return E_POINTER;

        *psizel = _sizeExtent;
        return S_OK;
    }

    HRESULT RichEditOleBase::SetExtent(DWORD dwDrawAspect, SIZEL *psizel)
    {
        if (dwDrawAspect != DVASPECT_CONTENT)
            return DV_E_DVASPECT;

        if (psizel == NULL)
            return E_POINTER;

        _sizeExtent = *psizel;

        return S_OK;
    }

    HRESULT RichEditOleBase::GetUserClassID(CLSID *pClsid)
    {
        if (pClsid == NULL)
        {
            return E_POINTER;
        }
        *pClsid = _oleGuid;

        return S_OK;
    }

    //
    // IViewObject2 methods
    //
    HRESULT RichEditOleBase::SetAdvise(DWORD aspects, DWORD advf, IAdviseSink *pAdvSink)
    {
        _spAdviseSink = pAdvSink;
        return S_OK;
    }

    HRESULT RichEditOleBase::GetAdvise(DWORD *pAspects, DWORD *pAdvf, IAdviseSink **ppAdvSink)
    {
        HRESULT hr = E_POINTER;
        if (ppAdvSink != NULL)
        {
            *ppAdvSink = _spAdviseSink;
            if (_spAdviseSink)
                _spAdviseSink.p->AddRef();
            hr = S_OK;
        }
        return hr;
    }

    HRESULT RichEditOleBase::Draw(
        DWORD dwDrawAspect,
        LONG lindex,
        void *pvAspect,
        DVTARGETDEVICE *ptd,
        HDC hdcTargetDev,
        HDC hdcDraw,
        LPCRECTL lprcBounds,
        LPCRECTL lprcWBounds,
        BOOL(STDMETHODCALLTYPE *pfnContinue)(ULONG_PTR dwContinue),
        ULONG_PTR dwContinue)
    {
        InvertBorder(hdcDraw, (RECT*)lprcBounds);

        _objRect = (RECT*)lprcBounds;
        _objRect.InflateRect(-1, -1, -1, -1); // 四周留一个像素给RichEdit画反色框
        _oleView.SetOleWindowRect(_objRect);

        if (_objRect.Width() <= 0 || _objRect.Height() <= 0)
        {
            return S_OK;
        }

        CAutoRefPtr<IRegion> rgn;
        GETRENDERFACTORY->CreateRegion(&rgn);
        rgn->CombineRect((RECT*)lprcBounds, RGN_AND);

        CAutoRefPtr<IRenderTarget> pRT;
        GETRENDERFACTORY->CreateRenderTarget(&pRT, _objRect.Width(), _objRect.Height());

        // 画背景
        HDC hdc = pRT->GetDC(0);
        ::BitBlt(hdc, 0, 0, _objRect.Width(), _objRect.Height(),
            hdcDraw, _objRect.left, _objRect.top,
            SRCCOPY);

        // 画richedit
        _oleView.RedrawRegion(pRT, rgn);

        // 贴到目标DC
        ::BitBlt(hdcDraw, _objRect.left, _objRect.top, _objRect.Width(), _objRect.Height(),
            hdc, 0, 0,
            SRCCOPY);

        pRT->ReleaseDC(hdc);

        return S_OK;
    }

    HRESULT RichEditOleBase::GetExtent(
        DWORD dwDrawAspect,
        LONG lindex,
        DVTARGETDEVICE *ptd,
        LPSIZEL lpsizel)
    {
        SASSERT(lpsizel != NULL);
        if (lpsizel == NULL)
            return E_POINTER;

        *lpsizel = _sizeExtent;
        return S_OK;
    }

    //
    // RichEditObj methods
    //
    void RichEditOleBase::SetDirty(BOOL bDirty)
    {
        RichEditObj::SetDirty(bDirty);
        if (bDirty)
        {
            _objRect.SetRect(0, 0, 0, 0);
            _oleView.SetOleWindowRect(CRect(0, 0, 0, 0));
        }
        else
        {
            _oleView.SetOleWindowRect(_objRect);
        }
    }

    BOOL RichEditOleBase::InsertIntoHost(IRichEditObjHost * pHost)
    {
        SASSERT(pHost);
        _pObjHost = pHost;    // 要放在第一句,其它函数要用到m_pRichEditHost

        InitOleWindow(pHost);
        return SUCCEEDED(InsertOleObject(pHost));
    }

    // 
    // 单选的时候才需要画反色框 
    //
    void RichEditOleBase::InvertBorder(HDC hdc, LPRECT lpBorder)
    {
        if (!_canBeSelect)
        {
            CHARRANGE chr = { 0 };
            _pObjHost->SendMessage(EM_EXGETSEL, 0, (LPARAM)&chr, NULL);

            if (chr.cpMax - chr.cpMin == 1 &&       // |
                chr.cpMin <= _contentChr.cpMin &&  // -> 单选,并且选中了自己 
                _contentChr.cpMin < chr.cpMax)     // |
            {
                SComPtr<IRichEditOle> ole;
                _pObjHost->SendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&ole);

                REOBJECT reobj = { 0 };
                reobj.cbStruct = sizeof(REOBJECT);
                reobj.cp = _contentChr.cpMin;
                HRESULT hr = ole->GetObject(REO_IOB_USE_CP, &reobj, REO_GETOBJ_NO_INTERFACES);
                if (SUCCEEDED(hr))
                {
                    CRect rcBorder = lpBorder;
                    InvertRect(hdc, rcBorder);
                    rcBorder.InflateRect(-1, -1, -1, -1);
                    InvertRect(hdc, rcBorder);
                }
            }
        }
    }

    void RichEditOleBase::RequestDraw()
    {
        SWindow*  pRoot = _oleView.GetWindow(GSW_FIRSTCHILD);
        if (pRoot)
        {
            pRoot->Invalidate();
        }
    }

    //
    // 避免在UpdateLayout期间崩溃，需要先禁止richedit画图
    // UpdateLayout()-> ... -> RichEdit::OnPaint -> pWindow::DrawText(崩在这里)
    // 所以需要先禁止RichEdit的OnPaint
    //
    void RichEditOleBase::UpdateWindowLayout(SWindow* pWindow)
    {
        if (!pWindow || !_pObjHost)
        {
            return;
        }

        _pObjHost->EnableDraw(FALSE);
        pWindow->RequestRelayout();
        pWindow->GetParent()->UpdateLayout();
        _pObjHost->EnableDraw(TRUE);
    }

    HRESULT RichEditOleBase::InsertOleObject(IRichEditObjHost * pHost)
    {
        //insert this to host
        SComPtr<IOleObject>	pOleObject;
        SComPtr<IOleClientSite> pClientSite;

        HRESULT hr = E_FAIL;
        REOBJECT reobject = { 0 };

        SComPtr<IRichEditOle> ole;
        pHost->SendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&ole);

        // Get site
        ole->GetClientSite(&pClientSite);
        SASSERT(pClientSite != NULL);

        SComPtr<IRichEditOleCallback> pCallback;
        hr = ole->QueryInterface(IID_IRichEditOleCallback, (void**)&pCallback);
        if (!SUCCEEDED(hr)) return E_FAIL;

        //get the IOleObject
        hr = QueryInterface(IID_IOleObject, (void**)&pOleObject);
        if (FAILED(hr))
        {
            return	 E_FAIL;
        }

        //to insert into richedit, you need a struct of REOBJECT
        ZeroMemory(&reobject, sizeof(REOBJECT));

        pOleObject->GetUserClassID(&reobject.clsid);
        pCallback->GetNewStorage(&reobject.pstg);

        reobject.cbStruct = sizeof(REOBJECT);
        reobject.cp = REO_CP_SELECTION;
        reobject.dvaspect = DVASPECT_CONTENT;
        reobject.dwFlags = REO_BELOWBASELINE;
        reobject.poleobj = pOleObject;
        reobject.polesite = pClientSite;

        hr = pOleObject->SetClientSite(pClientSite);
        if (SUCCEEDED(hr))
        {
            PreInsertObject(reobject);  // 给子类一个机会去修改reobject
            hr = ole->InsertObject(&reobject);
            _contentChr.cpMin = pHost->GetCharCount() - 1;
            _contentChr.cpMax = _contentChr.cpMin + 1;
        }

        if (reobject.pstg)
        {
            reobject.pstg->Release();
        }

        return hr;
    }

    BOOL RichEditOleBase::InitOleWindow(IRichEditObjHost* pHost)
    {
        BOOL bRet = FALSE;
        if (!_xmlLayout.IsEmpty())
        {
            pugi::xml_document xmlDoc;
            SStringTList strLst;

            if (2 == ParseResID(_xmlLayout, strLst))
            {
                LOADXML(xmlDoc, strLst[1], strLst[0]);
            }
            else
            {
                LOADXML(xmlDoc, strLst[0], RT_LAYOUT);
            }

            if (xmlDoc)
            {
                _oleView.SetHostRichEdit(pHost);
                bRet = _oleView.InitFromXml(xmlDoc.child(L"root"));
                SASSERT(bRet);
                _oleView.Move(0, 0, _sizeNatural.cx, _sizeNatural.cy);
                CalculateExtentSize(_sizeNatural);
            }
        }

        return bRet;
    }

    void RichEditOleBase::CalculateExtentSize(const SIZE& sizeNature)
    {
        // 周围留一个像素作为选中时的黑框
        HDC hDC = ::GetDC(NULL);
        _sizeExtent.cx = ::MulDiv(sizeNature.cx + 2, 2540, GetDeviceCaps(hDC, LOGPIXELSX));
        _sizeExtent.cy = ::MulDiv(sizeNature.cy + 2, 2540, GetDeviceCaps(hDC, LOGPIXELSY));
        ::ReleaseDC(NULL, hDC);
    }

    LRESULT RichEditOleBase::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        AdjustMessageParam(msg, wParam, lParam);
        _oleView.HandleEvent(msg, wParam, lParam);
        bHandled = _oleView.IsMsgHandled();

        //
        // bHandled在此时总是为TRUE。
        // 通常情况下，我们希望消息继续由RichEdit控件继续处理，所以这里直接设为FALSE。
        // 如果某些OLE不希望消息由RichEdit处理，需要在子类中重载处理。比如点击一个OLE，
        // 默认情况会出现一个黑框，如果这里的bHandled为TRUE，则RichEdit就没机会处理这个点击事件，也就不会出现黑框
        //
        bHandled = FALSE;
        if (!_canBeSelect && (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK))
        {
            bHandled = TRUE; // 文件OLE不让RichEdit继续左击，否则会画出一个黑框
        }

        return 0;
    }

    BOOL RichEditOleBase::OnUpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
    {
        SWindow *pChild = _oleView.GetWindow(GSW_LASTCHILD);
        if (!pChild)
        {
            return FALSE;
        }

        SWND hHover = pChild->SwndFromPoint(pt, FALSE);
        SWindow * pHover = SWindowMgr::GetWindow(hHover);

        if (pHover)
        {
            if (pHover->OnUpdateToolTip(pt, tipInfo))
            {
                tipInfo.dwCookie = pHover->GetSwnd();
                return TRUE;
            }
        }
        return FALSE;
    }

}// namespace SOUI