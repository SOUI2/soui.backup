#include "stdafx.h"
#include "RichEditOleCtrls.h"
#include "..\SImagePlayer.h"
#include "RichEditOleBase.h"
#include "SImRichedit.h"
#include <Gdiplus.h>
#include "..\SImageEx.h"
#include "utils.h"

// {9A7A5798-AB0E-4083-AE09-F21F4CC57486}
static const GUID IID_ImageOleCtrl = 
{ 0x9a7a5798, 0xab0e, 0x4083, { 0xae, 0x9, 0xf2, 0x1f, 0x4c, 0xc5, 0x74, 0x86 } };

RichEditImageOle::RichEditImageOle()
{
    m_bClosed       = FALSE;
    m_pPlayer       = NULL;
    m_guidOle       = IID_ImageOleCtrl;
    m_sizeMax.cx    = m_sizeMax.cy = 150;
    m_strXmlLayout  = L"LAYOUT:ImageOle";
}

double RichEditImageOle::GetZoomRatio(SIZE sizeImage, SIZE sizeMax)
{
    double fXRatio = 1.0f;
    double fYRatio = 1.0f;
    if (sizeImage.cx > sizeMax.cx)
    {
        fXRatio = (double)sizeMax.cx / (double)sizeImage.cx;
    }

    if (sizeImage.cy > sizeMax.cy)
    {
        fYRatio = (double)sizeMax.cy / (double)sizeImage.cy;
    }

    return fXRatio > fYRatio ? fYRatio : fXRatio;
}

BOOL RichEditImageOle::SetImage(LPCWSTR lpszImagePath)
{
    SStringW strTip;

    double fRatio = 1.0f;
    m_strImagePath = lpszImagePath;

    if (!m_pPlayer)
    {
        m_pPlayer = static_cast<SImagePlayer*>(m_oleWindow.FindChildByName(L"player"));
    }
    
    if (m_pPlayer)
    {
        m_pPlayer->ShowImageFile(lpszImagePath);
        m_sizeNatural = m_pPlayer->GetImageSize();
        
        SIZE szShow;
        fRatio = GetZoomRatio(m_sizeNatural, m_sizeMax);
        szShow.cx = LONG((double)m_sizeNatural.cx * fRatio); 
        szShow.cy = LONG((double)m_sizeNatural.cy * fRatio); 

        CalculateExtentSize(szShow);
        if (fRatio < 1.0f)
        {
            strTip.Format(_T("显示比例: %%%d,双击查看原图"), (int)(fRatio * 100));
            m_pPlayer->SetAttribute(L"tip", strTip);
        }
        else
        {
            m_pPlayer->SetAttribute(L"tip", L"");
        }

        m_oleWindow.SetDelayDraw(m_pPlayer->GetFrameCount() > 1); // 大于1帧的图片需要延迟刷新
        m_oleWindow.Move(0, 0, szShow.cx, szShow.cy);
    }

    SImageButton * pBtn = static_cast<SImageButton*>(m_oleWindow.FindChildByName(L"btn_show_img"));
    if (pBtn)
    {
        if (fRatio < 1.0f)
        {
            strTip.Format(_T("显示比例: %%%d,单击查看原图"), (int)(fRatio * 100));
            pBtn->SetAttribute(L"tip", strTip);
        }
        else
        {
            pBtn->SetAttribute(L"tip", L"");
        }

        BOOL bVisible = fRatio < 1.0f;
        pBtn->SetVisible(bVisible);
    }

    if (m_spAdviseSink)
    {
        m_spAdviseSink->OnViewChange(DVASPECT_CONTENT,-1);  
    }

    return TRUE;
}

BOOL RichEditImageOle::InitOleWindow(IRichEditObjHost * pHost)
{
    BOOL ret = RichEditOleBase::InitOleWindow(pHost);
    if (m_strImagePath.GetLength() > 0)
    {
        SetImage(m_strImagePath);
    }

    return ret;
}

HRESULT RichEditImageOle::Close(DWORD dwSaveOption)
{   
    if (m_pPlayer)
    {
        m_pPlayer->Pause();
    }
    m_bClosed = TRUE;
    return S_OK;
}

HRESULT RichEditImageOle::Draw(
    DWORD dwDrawAspect, LONG lindex,  
    void *pvAspect, 
    DVTARGETDEVICE *ptd, 
    HDC hdcTargetDev,
    HDC hdcDraw, 
    LPCRECTL lprcBounds,
    LPCRECTL lprcWBounds,
    BOOL ( STDMETHODCALLTYPE *pfnContinue )(ULONG_PTR dwContinue), 
    ULONG_PTR dwContinue)
{
    HRESULT hr;
    hr = RichEditOleBase::Draw( dwDrawAspect,
                                lindex,  
                                pvAspect, 
                                ptd, 
                                hdcTargetDev,
                                hdcDraw, 
                                lprcBounds,
                                lprcWBounds,
                                pfnContinue, 
                                dwContinue);
    if (m_bClosed)
    {
        m_bClosed = FALSE;
        if (m_pPlayer) m_pPlayer->Resume();
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////
//
// {E0ED3FC5-1645-4b7f-A0E2-86F5288F407B}
static const GUID IID_FileOleCtrl = 
{ 0xe0ed3fc5, 0x1645, 0x4b7f, { 0xa0, 0xe2, 0x86, 0xf5, 0x28, 0x8f, 0x40, 0x7b } };

RichEditFileOle::RichEditFileOle()
{
    m_guidOle = IID_FileOleCtrl;
    m_sizeNatural.cx = 295;
    m_sizeNatural.cy = 95;
    m_strXmlLayout  = L"LAYOUT:FileOle";
}

LRESULT RichEditFileOle::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RichEditOleBase::ProcessMessage(msg, wParam, lParam, bHandled);
    if (msg == WM_LBUTTONDOWN)
    {
        bHandled = TRUE; // 文件OLE不让RichEdit继续左击，否则会画出一个黑框
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//
// {C0402A65-5BBF-4bef-9861-C55CD3A07201}
static const GUID IID_FetchMoreOleCtrl = 
{ 0xc0402a65, 0x5bbf, 0x4bef, { 0x98, 0x61, 0xc5, 0x5c, 0xd3, 0xa0, 0x72, 0x1 } };

RichEditFetchMoreOle::RichEditFetchMoreOle()
{
    m_guidOle = IID_FetchMoreOleCtrl;
    m_sizeNatural.cx = 95;
    m_sizeNatural.cy = 25;
    m_strXmlLayout  = L"LAYOUT:FetchMoreOle";
}

BOOL RichEditFetchMoreOle::InitOleWindow(IRichEditObjHost * pHost)
{
    BOOL    ret   = RichEditOleBase::InitOleWindow(pHost);
    SLink * pLink = static_cast<SLink*>(m_oleWindow.FindChildByName(L"fetchmore"));
    if (pLink)
    {
        pLink->GetEventSet()->subscribeEvent(EventCmd::EventID, Subscriber(&RichEditFetchMoreOle::OnFetchMore,this));
    }

    return ret;
}

bool RichEditFetchMoreOle::OnFetchMore(EventArgs *pEvt)
{
    SWindow * pWnd = static_cast<SWindow*>(m_oleWindow.FindChildByName(L"icon"));
    if (pWnd)
    {
        pWnd->SetVisible(FALSE);
    }

    pWnd = static_cast<SWindow*>(m_oleWindow.FindChildByName(L"fetchmore"));
    if (pWnd)
    {
        pWnd->SetVisible(FALSE);
    }

    pWnd = static_cast<SWindow*>(m_oleWindow.FindChildByName(L"loading"));
    if (pWnd)
    {
        pWnd->SetVisible(TRUE);
    }
    return true;
}

LRESULT RichEditFetchMoreOle::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RichEditOleBase::ProcessMessage(msg, wParam, lParam, bHandled);
    if (msg == WM_LBUTTONDOWN)
    {
        bHandled = TRUE; // 查看更多OLE不让RichEdit继续左击，否则会画出一个黑框
    }
    return 0;
}

void RichEditFetchMoreOle::UpdatePosition()
{
    if (m_spAdviseSink)
    {    
        CRect rcHost = m_pObjectHost->GetHostRect();
        m_sizeNatural.cx = rcHost.Width();

        CalculateExtentSize(m_sizeNatural);
        m_oleWindow.SetOleWindowRect(CRect(0,0,0,0)); // 已经失效
        m_oleWindow.Move(0, 0, m_sizeNatural.cx, m_sizeNatural.cy);
        m_spAdviseSink->OnViewChange(DVASPECT_CONTENT,-1);  
    }
}

//////////////////////////////////////////////////////////////////////////
//RichEditSplitLineOle
// {8D5E6EF1-2A36-4930-91E7-1149303994BF}
static const GUID IID_SplitLineOleCtrl = 
{ 0x8d5e6ef1, 0x2a36, 0x4930, { 0x91, 0xe7, 0x11, 0x49, 0x30, 0x39, 0x94, 0xbf } };

RichEditSplitLineOle::RichEditSplitLineOle()
{
    m_guidOle = IID_SplitLineOleCtrl;
    m_sizeNatural.cx = 95;
    m_sizeNatural.cy = 25;
    m_strXmlLayout  = L"LAYOUT:SplitLineOle";
}

void RichEditSplitLineOle::UpdatePosition()
{
    if (m_spAdviseSink)
    {
        m_sizeNatural.cx = m_pObjectHost->GetHostRect().Width();
        m_oleWindow.SetOleWindowRect(CRect(0,0,0,0)); // 已经失效
        m_oleWindow.Move(0, 0, m_sizeNatural.cx, m_sizeNatural.cy);
        CalculateExtentSize(m_sizeNatural);
        m_spAdviseSink->OnViewChange(DVASPECT_CONTENT,-1);  
    }
}

LRESULT RichEditSplitLineOle::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (msg == WM_LBUTTONDOWN)
    {
        bHandled = TRUE; // 分隔栏OLE不让RichEdit继续左击，否则会画出一个黑框
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//
// {A5D897A6-4533-4e15-9A33-A5814F927768}
static const GUID IID_RemainderOleCtrl = 
{ 0xa5d897a6, 0x4533, 0x4e15, { 0x9a, 0x33, 0xa5, 0x81, 0x4f, 0x92, 0x77, 0x68 } };

RichEditReminderOle::RichEditReminderOle():
    m_crBorder(CR_INVALID)
    ,m_crBk(CR_INVALID)
    ,m_crText(CR_INVALID)
    ,m_nBorderWidth(0)
    ,m_strFont(L"size:12")
{
    m_guidOle = IID_RemainderOleCtrl;
    m_sizeMax.cx = m_sizeMax.cy = 150;
    m_strXmlLayout = L"LAYOUT:RemainderOle";
}

BOOL RichEditReminderOle::InitFromXml(pugi::xml_node xmlNode)
{
    m_strText = xmlNode.text().get();
    return __super::InitFromXml(xmlNode);
}

void RichEditReminderOle::CalculateNatureSize()
{
    CAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
    pRT->SelectObject(SFontPool::getSingleton().GetFont(m_strFont));
    pRT->MeasureText(m_strText, m_strText.GetLength(), &m_sizeNatural);

    m_sizeNatural.cx += m_nBorderWidth * 2;
    m_sizeNatural.cy += m_nBorderWidth * 2;

    if (m_sizeNatural.cx > m_sizeMax.cx)
    {
        m_sizeNatural.cx = m_sizeMax.cx;
    }
    if (m_sizeNatural.cy > m_sizeMax.cy)
    {
        m_sizeNatural.cy = m_sizeMax.cy;
    }
}

void RichEditReminderOle::InitAttributes()
{
    SWindow * pText = m_oleWindow.FindChildByName2<SStatic>(L"txt");
    if (pText)
    {
        pText->SetWindowText(m_strText);
        pText->SetAttribute(L"font", m_strFont);
        pText->GetStyle().SetTextColor(0, m_crText);
        pText->GetStyle().m_crBorder = m_crBorder;
        pText->GetStyle().m_crBg     = m_crBk;
        pText->GetStyle().m_rcMargin.SetRect(m_nBorderWidth,m_nBorderWidth,m_nBorderWidth,m_nBorderWidth);
    }
}

BOOL RichEditReminderOle::InitOleWindow(IRichEditObjHost * pHost)
{
    CalculateNatureSize();
    BOOL ret = RichEditOleBase::InitOleWindow(pHost);
    InitAttributes();
    return ret;
}

//////////////////////////////////////////////////////////////////////////
// RichEditNewsOle
// {CC5C0B6C-80BC-4d08-BE85-DC3574DD9162}
static const GUID IID_NewsOleCtrl = 
{ 0xcc5c0b6c, 0x80bc, 0x4d08, { 0xbe, 0x85, 0xdc, 0x35, 0x74, 0xdd, 0x91, 0x62 } };

RichEditNewsOle::RichEditNewsOle()
{
    m_guidOle = IID_NewsOleCtrl;
}

RichEditNewsOle::~RichEditNewsOle()
{
}

void RichEditNewsOle::AddNews(LPCWSTR pszTitle, LPCWSTR pszDesc, LPCWSTR pszURL, LPCWSTR pszImage)
{
    NewsItem item;
    item.strTitle = pszTitle;
    item.strDesc = pszDesc;
    item.strURL = pszURL;
    item.strImagePath = pszImage;

    //m_vecNewsItem.push_back(item);
    m_arrNews.Add(item);
}

void RichEditNewsOle::LayoutForSingleNews()
{
    if (m_arrNews.GetCount() == 0)
    {
        return;
    }

    NewsItem &item = m_arrNews.GetAt(0);
    CRect rcHost = m_pObjectHost->GetHostRect();
    m_sizeNatural.cx = rcHost.Width();
    m_sizeNatural.cy = item.strImagePath.IsEmpty()? 120: 360;

    if (m_oleWindow.GetWindow(GSW_FIRSTCHILD))
    {
        m_oleWindow.DestroyChild(m_oleWindow.GetWindow(GSW_FIRSTCHILD));
    }
    m_strXmlLayout = L"LAYOUT:SingleNewsOle";
    RichEditOleBase::InitOleWindow(m_pObjectHost);

    SImageEx * pImage = m_oleWindow.FindChildByName2<SImageEx>(L"newsImg");
    if (pImage != NULL)
    {
        if (item.strImagePath.IsEmpty())
        {
            SWindow * pParent = pImage->GetParent();
            pParent->RemoveChild(pImage);
            pImage->Release();
            pParent->RequestRelayout();
            pParent->UpdateLayout();
        }
        else
        {
            pImage->SetAttribute(L"src", item.strImagePath);
        }
    }
}

void RichEditNewsOle::LayoutForMultiNews()
{
    if (m_arrNews.GetCount() < 2)
    {
        return;
    }

    CRect rcHost = m_pObjectHost->GetHostRect();
    m_sizeNatural.cx = rcHost.Width();
    m_sizeNatural.cy = 262 + (m_arrNews.GetCount() -1)* 50;
    
    int nNewsIndex = 0;
    NewsItem& item = m_arrNews.GetAt(0);

    if (m_oleWindow.GetWindow(GSW_FIRSTCHILD))
    {
        m_oleWindow.DestroyChild(m_oleWindow.GetWindow(GSW_FIRSTCHILD));
    }

    m_strXmlLayout = L"LAYOUT:MultiNewsOle";
    BOOL bRet = RichEditOleBase::InitOleWindow(m_pObjectHost);

    pugi::xml_document  xmlDoc;
    LOADXML(xmlDoc, L"MultiNewsOle" ,RT_LAYOUT);

    SWindow * pParent = m_oleWindow.FindChildByName(L"parent");
    SWindow * pFirstArticle = pParent->FindChildByName(L"headlines");
    pFirstArticle->FindChildByName(L"title")->SetWindowText(item.strTitle);
    pFirstArticle->FindChildByName(L"illustration")->SetAttribute(L"src", item.strImagePath);
    pFirstArticle->SetUserData(nNewsIndex++);
    pFirstArticle->GetEventSet()->subscribeEvent(EventCmd::EventID, Subscriber(&RichEditNewsOle::OnClickNotifyWnd,this));

    pugi::xml_node xmlTempl=xmlDoc.child(L"root").child(L"window").child(L"news_template");
    for (size_t i = 1; i < m_arrNews.GetCount(); ++i)
    {
        item = m_arrNews.GetAt(i);
        SWindow * pWnd = new SWindow();
        pParent->InsertChild(pWnd);
        pWnd->InitFromXml(xmlTempl.child(L"window"));

        pWnd->FindChildByName(L"title")->SetWindowText(item.strTitle);
        pWnd->FindChildByName(L"illustration")->SetAttribute(L"src", item.strImagePath);
        pWnd->SetUserData(nNewsIndex++);
        pWnd->GetEventSet()->subscribeEvent(EventCmd::EventID, Subscriber(&RichEditNewsOle::OnClickNotifyWnd,this));
    }

    pParent->RequestRelayout();
    pParent->UpdateLayout();
}

void RichEditNewsOle::ReLayout()
{
    if (m_arrNews.GetCount() == 1)
    {
        LayoutForSingleNews();
    }
    else if (m_arrNews.GetCount() > 1)
    {
        LayoutForMultiNews();
    }

    UpdatePosition();
}

bool RichEditNewsOle::OnClickNotifyWnd(EventArgs *pEvt)
{
    SWindow * pWnd = static_cast<SWindow*>(pEvt->sender);
    if (pWnd)
    {
        ULONG uIndex = (ULONG)pWnd->GetUserData();
        if (uIndex < m_arrNews.GetCount())
        {
            ::MessageBox(NULL, m_arrNews[uIndex].strURL, L"跳转到", MB_OK);
        }
    }

    return true;
}

void RichEditNewsOle::UpdatePosition()
{
    if (m_spAdviseSink)
    {
        m_sizeNatural.cx = m_pObjectHost->GetHostRect().Width();
        CalculateExtentSize(m_sizeNatural);
        m_oleWindow.SetOleWindowRect(CRect(0,0,0,0)); // 已经失效
        m_oleWindow.Move(0, 0, m_sizeNatural.cx, m_sizeNatural.cy);
        m_spAdviseSink->OnViewChange(DVASPECT_CONTENT,-1);  
    }
}

LRESULT RichEditNewsOle::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RichEditOleBase::ProcessMessage(msg, wParam, lParam, bHandled);
    if (msg == WM_LBUTTONDOWN)
    {
        bHandled = TRUE; // 新闻OLE不让RichEdit继续左击，否则会画出一个黑框
    }
    return 0;
}
