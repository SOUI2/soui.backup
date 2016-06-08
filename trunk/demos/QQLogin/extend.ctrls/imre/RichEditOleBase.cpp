#include "stdafx.h"
#include <richole.h>
#include "RichEditOleBase.h"
#include "helper\SplitString.h"

//////////////////////////////////////////////////////////////////////////
// OleWindow
BOOL OleWindow::m_bTiemrRegistered;
OleTimerHandler OleWindow::m_timerHandler;

OleWindow::OleWindow():m_bDelayDraw(FALSE)
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

IRenderTarget * OleWindow::OnGetRenderTarget(const CRect & rc,DWORD gdcFlags)
{
    return m_pHostContainer->OnGetRenderTarget(rc, gdcFlags);
}

void OleWindow::OnReleaseRenderTarget(IRenderTarget * pRT,const CRect &rc,DWORD gdcFlags)
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


void OleWindow::UpdateTooltip()
{
	m_pHostContainer->UpdateTooltip();
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

RichEditOleBase::RichEditOleBase():m_bCanSelect(TRUE)
    //,m_dwRef(0)
{
    m_sizeNatural.cx = m_sizeNatural.cy = 0;
    m_sizeExtent.cx = m_sizeExtent.cy = 0;
}

RichEditOleBase::~RichEditOleBase()
{
    m_oleWindow.SSendMessage(WM_DESTROY);
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
    else if(::IsEqualIID(riid, m_guidOle))
    {
        *ppvObject = static_cast<IOleObject*>(this);
    }
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
HRESULT RichEditOleBase::Advise(IAdviseSink *pAdvSink,DWORD *pdwConnection)
{
    HRESULT hr = S_OK;
    if (m_spOleAdviseHolder == NULL)
        hr = CreateOleAdviseHolder(&m_spOleAdviseHolder);
    if (SUCCEEDED(hr))
        hr = m_spOleAdviseHolder->Advise(pAdvSink, pdwConnection);
    return hr;
}

HRESULT RichEditOleBase::Unadvise(DWORD dwConnection)
{
    HRESULT hRes = E_FAIL;
    if (m_spOleAdviseHolder != NULL)
        hRes = m_spOleAdviseHolder->Unadvise(dwConnection);
    return hRes;
}

HRESULT RichEditOleBase::SetClientSite(IOleClientSite *pClientSite)
{
    m_spClientSite = pClientSite;
    return S_OK;
}

HRESULT RichEditOleBase::GetClientSite(IOleClientSite **ppClientSite)
{
    SASSERT(ppClientSite);
    if (ppClientSite == NULL)
        return E_POINTER;

    *ppClientSite = m_spClientSite;
    if (m_spClientSite != NULL)
        m_spClientSite.p->AddRef();
    return S_OK;
}

HRESULT RichEditOleBase::GetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
    if (dwDrawAspect != DVASPECT_CONTENT)
        return E_FAIL;

    if (psizel == NULL)
        return E_POINTER;

    *psizel = m_sizeExtent;
    return S_OK;
}

HRESULT RichEditOleBase::SetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
    if (dwDrawAspect != DVASPECT_CONTENT)
        return DV_E_DVASPECT;

    if (psizel == NULL)
        return E_POINTER;

    m_sizeExtent = *psizel;

    return S_OK;
}

HRESULT RichEditOleBase::GetUserClassID(CLSID *pClsid)
{
    if (pClsid == NULL)
    {
        return E_POINTER;
    }
    *pClsid = m_guidOle;

    return S_OK;
}

//
// IViewObject2 methods
//
HRESULT RichEditOleBase::SetAdvise( DWORD aspects, DWORD advf, IAdviseSink *pAdvSink)
{
    m_spAdviseSink = pAdvSink;
    return S_OK;
}

HRESULT RichEditOleBase::GetAdvise(DWORD *pAspects, DWORD *pAdvf, IAdviseSink **ppAdvSink)
{
    HRESULT hr = E_POINTER;
    if (ppAdvSink != NULL)
    {
        *ppAdvSink = m_spAdviseSink;
        if (m_spAdviseSink)
            m_spAdviseSink.p->AddRef();
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
    BOOL ( STDMETHODCALLTYPE *pfnContinue )(ULONG_PTR dwContinue), 
    ULONG_PTR dwContinue)
{
    InvertBorder(hdcDraw, (RECT*)lprcBounds);

    m_rcObj = (RECT*)lprcBounds;
    m_rcObj.InflateRect(-1,-1,-1,-1); // 四周留一个像素给RichEdit画反色框
    m_oleWindow.SetOleWindowRect(m_rcObj);

    CAutoRefPtr<IRegion> rgn;
    GETRENDERFACTORY->CreateRegion(&rgn);
    rgn->CombineRect((RECT*)lprcBounds,RGN_AND);

    CAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, m_rcObj.Width(), m_rcObj.Height());

    // 画背景
    HDC hdc = pRT->GetDC(0);
    ::BitBlt(hdc, 0, 0, m_rcObj.Width(), m_rcObj.Height(),
        hdcDraw, m_rcObj.left, m_rcObj.top, 
        SRCCOPY);

    // 画richedit
    m_oleWindow.RedrawRegion(pRT, rgn);

    // 贴到目标DC
    ::BitBlt(hdcDraw, m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(),
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

    *lpsizel = m_sizeExtent;
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
        m_rcObj.SetRect(0,0,0,0);
        m_oleWindow.SetOleWindowRect(CRect(0,0,0,0));
    }
    else
    {
        m_oleWindow.SetOleWindowRect(m_rcObj);
    }
}

BOOL RichEditOleBase::InsertIntoHost(IRichEditObjHost * pHost)
{
    m_pObjectHost = pHost;    // 要放在第一句,其它函数要用到m_pRichEditHost

    InitOleWindow(pHost);
    return SUCCEEDED(InsertOleObject(pHost));
}

// 
// 单选的时候才需要画反色框 
//
void RichEditOleBase::InvertBorder(HDC hdc, LPRECT lpBorder)
{
    if (!m_bCanSelect)
    {
        CHARRANGE chr={0};
        m_pObjectHost->SendMessage(EM_EXGETSEL, 0, (LPARAM)&chr,NULL);

        if (chr.cpMax - chr.cpMin == 1 &&       // |
            chr.cpMin <= m_chrContent.cpMin &&  // -> 单选,并且选中了自己 
            m_chrContent.cpMin < chr.cpMax)     // |
        {
            SComPtr<IRichEditOle> ole;
            m_pObjectHost->SendMessage(EM_GETOLEINTERFACE,0,(LPARAM)&ole);

            REOBJECT reobj={0};
            reobj.cbStruct=sizeof(REOBJECT);
            reobj.cp = m_chrContent.cpMin;
            HRESULT hr=ole->GetObject(REO_IOB_USE_CP, &reobj, REO_GETOBJ_NO_INTERFACES);
            if (SUCCEEDED(hr))
            {
                CRect rcBorder = lpBorder;
                InvertRect(hdc, rcBorder);
                rcBorder.InflateRect(-1,-1,-1,-1);
                InvertRect(hdc, rcBorder);
            }
        }
    }
}

HRESULT RichEditOleBase::InsertOleObject(IRichEditObjHost * pHost)
{
    //insert this to host
    SComPtr<IOleObject>	pOleObject;
    SComPtr<IOleClientSite> pClientSite;  

    HRESULT hr          = E_FAIL;
    REOBJECT reobject   = {0};

    SComPtr<IRichEditOle> ole;
    pHost->SendMessage(EM_GETOLEINTERFACE,0,(LPARAM)&ole);

    // Get site
    ole->GetClientSite(&pClientSite);
    SASSERT(pClientSite != NULL);

    SComPtr<IRichEditOleCallback> pCallback;
    hr=ole->QueryInterface(IID_IRichEditOleCallback,(void**)&pCallback);
    if(!SUCCEEDED(hr)) return E_FAIL;

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

    reobject.cbStruct   = sizeof(REOBJECT);	
    reobject.cp         = REO_CP_SELECTION;
    reobject.dvaspect   = DVASPECT_CONTENT;
    reobject.dwFlags    = REO_BELOWBASELINE;
    reobject.poleobj    = pOleObject;
    reobject.polesite   = pClientSite;

    hr = pOleObject->SetClientSite(pClientSite);
    if (SUCCEEDED(hr))
    {
        PreInsertObject(reobject);  // 给子类一个机会去修改reobject
        hr = ole->InsertObject(&reobject);
        m_chrContent.cpMin = pHost->GetContentLength() - 1;
        m_chrContent.cpMax = m_chrContent.cpMin + 1;
    }

    if(reobject.pstg)
    {
        reobject.pstg->Release();
    }

    return hr;
}

BOOL RichEditOleBase::InitOleWindow(IRichEditObjHost* pHost)
{
    BOOL bRet = FALSE;
    if(!m_strXmlLayout.IsEmpty())
    {
        pugi::xml_document xmlDoc;
        SStringTList strLst;

        if(2 == ParseResID(m_strXmlLayout,strLst))
        {
            LOADXML(xmlDoc,strLst[1],strLst[0]);
        }else
        {
            LOADXML(xmlDoc,strLst[0],RT_LAYOUT);
        }    
        
        if (xmlDoc)
        {
            m_oleWindow.SetHostRichEdit(pHost);
            bRet = m_oleWindow.InitFromXml(xmlDoc.child(L"root"));
            SASSERT(bRet);
            m_oleWindow.Move(0, 0, m_sizeNatural.cx, m_sizeNatural.cy);
            CalculateExtentSize(m_sizeNatural);
        }
    }

    return bRet;
}

void RichEditOleBase::CalculateExtentSize(const SIZE& sizeNature)
{
    // 周围留一个像素作为选中时的黑框
    HDC hDC = ::GetDC(NULL);
    m_sizeExtent.cx = ::MulDiv(sizeNature.cx+2, 2540, GetDeviceCaps(hDC, LOGPIXELSX));
    m_sizeExtent.cy = ::MulDiv(sizeNature.cy+2, 2540, GetDeviceCaps(hDC, LOGPIXELSY));
    ::ReleaseDC(NULL, hDC);
}

LRESULT RichEditOleBase::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    AdjustMessageParam(msg, wParam, lParam);
    m_oleWindow.HandleEvent(msg, wParam, lParam);
    m_oleWindow.IsMsgHandled();
    bHandled = FALSE; // 通常情况下，我们希望消息继续由RichEdit控件继续处理
    return 0;
}

BOOL RichEditOleBase::OnUpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
{
    SWND hHover=m_oleWindow.SwndFromPoint(pt,FALSE);
    SWindow * pHover=SWindowMgr::GetWindow(hHover);

    if (pHover)
    {
        if(pHover->OnUpdateToolTip(pt, tipInfo))
        {
            tipInfo.dwCookie = pHover->GetSwnd();
            return TRUE;
        }
    }
    return FALSE;
}
