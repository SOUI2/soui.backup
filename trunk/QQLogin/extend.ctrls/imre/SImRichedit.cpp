#include "stdafx.h"
#include "SImRichedit.h"
#include "TOM.h"
#include <atlcomcli.h>
#include "interface\render-i.h"
#include "RichEditOleCallback.h"
#include "atl.mini\SComHelper.h"
#include <time.h>

SImRichEdit::SImRichEdit():m_pTxtDoc(NULL)
    ,m_pLastOverOle(NULL)
    ,m_bBkgndDirty(TRUE)
{

}

SImRichEdit::~SImRichEdit()
{

}

void SImRichEdit::GetSel(long* pStartCp, long* pEndCp)
{
    CHARRANGE cr ={0, 0};
    SSendMessage(EM_EXGETSEL, NULL, (LPARAM)&cr);

    *pStartCp = cr.cpMin;
    *pEndCp   = cr.cpMax;
}

void SImRichEdit::SetSel(int nStart, int nEnd)
{
    CHARRANGE cr = { nStart, nEnd };
    SSendMessage(EM_EXSETSEL, NULL, (LPARAM)&cr);
}

void SImRichEdit::ScrollEnd()
{
    SSendMessage(WM_VSCROLL, (WPARAM)SB_BOTTOM, NULL);
}

int SImRichEdit::GetContentLength()
{
    SComPtr<ITextRange> range2;
    m_pTxtDoc->Range((~(ULONG)0)/2,(~(ULONG)0)/2,&range2);
    long end;
    range2->GetEnd(&end);
    return end;
}

ITextRange* SImRichEdit::GetTextRange(const CHARRANGE& chr)
{
    ITextRange* pRange;
    m_pTxtDoc->Range(chr.cpMin, chr.cpMax, &pRange);
    return pRange;
}

RichEditContent * SImRichEdit::GetContent(UINT uIndex)
{
    if (uIndex >= m_arrContent.GetCount())
        return NULL;

    return m_arrContent.GetAt(uIndex);
}

void SImRichEdit::DeleteContent(UINT uIndex)
{
    if (uIndex >= m_arrContent.GetCount())
        return;

    delete m_arrContent.GetAt(uIndex);
    m_arrContent.RemoveAt(uIndex);

    // TODO:还要删除richedit上对应的内容
}   

void SImRichEdit::Clear()
{
    SSendMessage(EM_EMPTYUNDOBUFFER);
    for (size_t npos = 0; npos < m_arrContent.GetCount(); ++npos)
    {
        RichEditContent * p = m_arrContent.GetAt(npos);
        p->Release();
    }
    m_arrContent.RemoveAll();
}

RichEditContent* SImRichEdit::CreateRichEditConent(LPCWSTR lpszContent)
{
    if (lpszContent == NULL)
        return NULL;

    RichEditContent * pRet = NULL;
    pugi::xml_document  doc;
    if(doc.load_buffer(lpszContent, wcslen(lpszContent)*sizeof(WCHAR) ))
    {
        pRet = new RichEditContent();
        if (!pRet->InitFromXml(doc.child(RichEditContent::GetClassName())))
        {
            delete pRet;
            return NULL;
        }
    }

    return pRet;
}

void SImRichEdit::SortContents(UINT npos)
{
    if (npos >= m_arrContent.GetCount())
        return;

    CHARRANGE chr = m_arrContent.GetAt(npos)->GetCharRange();
    int nChrOffset = chr.cpMax - chr.cpMin;

    for (npos += 1; npos < m_arrContent.GetCount(); ++npos)
    {
        RichEditContent * pCurr = m_arrContent.GetAt(npos);
        pCurr->OffsetCharRange(nChrOffset);
    }
}

UINT SImRichEdit::InsertContent(LPCWSTR lpszContent, UINT uInsertAt/*=RECONTENT_LAST*/)
{
    RichEditContent * p = CreateRichEditConent(lpszContent);
    if (p == NULL)
        return -1;

    CHARRANGE chr = {-1, -1};

    if (uInsertAt == RECONTENT_CARET)
    {
        m_arrContent.Add(p);
    }
    else if (uInsertAt >= m_arrContent.GetCount())
    {
        uInsertAt = m_arrContent.GetCount();
        if (uInsertAt > 0)
        {
            chr = m_arrContent.GetAt(uInsertAt - 1)->GetCharRange();
            SetSel(chr.cpMax, chr.cpMax);
        }

        m_arrContent.Add(p);
    }
    else
    {
        chr = m_arrContent.GetAt(uInsertAt)->GetCharRange();
        SetSel(chr.cpMin, chr.cpMin);
        m_arrContent.InsertAt(uInsertAt, p);
    }

    UpdateContext ctx;
    PreUpdate(ctx);

    p->InsertIntoHost(this);
    p->UpdatePosition();
    SortContents(uInsertAt);
    InvalidateRect(p->GetRect());

    DoneUpdate(ctx);
    return uInsertAt;
}

RichEditObj * SImRichEdit::GetElementById(LPCWSTR lpszId)
{
    for (size_t npos = 0; npos < m_arrContent.GetCount(); ++npos)
    {
        RichEditObj * pObjRet = m_arrContent.GetAt(npos)->GetById(lpszId);
        if (pObjRet != NULL)
            return pObjRet;
    }

    return NULL;
}

RichEditOleBase * SImRichEdit::GetOleById(LPCWSTR lpszId)
{
    RichEditObj     * pObj = GetElementById(lpszId);
    SStringW cn = pObj->GetClassName();
    //RichEditOleBase * pOle = dynamic_cast<RichEditOleBase*>(pObj);
    RichEditOleBase * pOle = static_cast<RichEditOleBase*>(pObj);
    return pOle;
}

RichEditObj* SImRichEdit::HitTest(RichEditObj * pObject, POINT pt)
{
    for (; pObject != NULL; pObject = pObject->GetNext())
    {
        if (pObject->NeedToProcessMessage() && 
            pObject->GetRect().PtInRect(pt))
            return pObject;

        RichEditObj * p = HitTest(pObject->GetFirstChild(), pt);
        if (p != NULL)
            return p;
    }

    return NULL;
}

RichEditObj* SImRichEdit::HitTest(POINT pt)
{
    for (size_t npos = 0 ;npos < m_arrContent.GetCount(); ++npos)
    {
        RichEditObj * pObjHitted = HitTest(m_arrContent.GetAt(npos), pt);
        if (pObjHitted != NULL)
            return pObjHitted;
    }

    return NULL;
}

BOOL SImRichEdit::GetDefCharFormat(CHARFORMAT& cf)
{
    cf.cbSize = sizeof(CHARFORMAT);
    SSendMessage(EM_GETCHARFORMAT, 0, (LPARAM)&cf);
    return TRUE;
}

BOOL SImRichEdit::SetDefCharFormat(CHARFORMAT& cf)
{
    cf.cbSize = sizeof(CHARFORMAT);
    SSendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
    return TRUE;
}

BOOL SImRichEdit::SetParaFormat(PARAFORMAT2& pf)
{
    pf.cbSize = sizeof(PARAFORMAT2);
    SSendMessage(EM_SETPARAFORMAT, 0, (LPARAM)&pf);
    return TRUE;
}

void SImRichEdit::SetFontSize(int size)
{
    CHARFORMAT cf;
    GetDefCharFormat(cf);

    cf.dwMask = cf.dwMask | CFM_SIZE;
    cf.yHeight = size * 20;

    SetDefCharFormat(cf);
}

void SImRichEdit::SetSelectionColor(COLORREF color)
{
    CHARFORMAT cf;
    GetDefCharFormat(cf);
    cf.dwMask |= CFM_COLOR;
    cf.crTextColor = color;

    SSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

int SImRichEdit::LineFromChar(int ncp)
{
    return (int)SSendMessage(EM_LINEFROMCHAR, ncp, 0);
}

int  SImRichEdit::LineIndex(int nLineNum)
{
    return SSendMessage(EM_LINEINDEX, nLineNum, 0);
}

int SImRichEdit::LineLength(int nLineCP /* = -1 */)
{
    return (int) SSendMessage(EM_LINELENGTH, nLineCP, 0);
}

void  SImRichEdit::PosFromChar(UINT ncp, POINT* pPos)
{
    POINTL pt = {0, 0};
    SSendMessage(EM_POSFROMCHAR, (WPARAM)&pt, ncp);
    pPos->x = pt.x;
    pPos->y = pt.y;
}

int  SImRichEdit::GetFirstVisibleLine()
{
    return (int)SSendMessage(EM_GETFIRSTVISIBLELINE, 0, 0);
}

int  SImRichEdit::CharFromPos(POINT pt)
{
    POINTL  ptl = {pt.x, pt.y};
    return (int) SSendMessage(EM_CHARFROMPOS, 0, (LPARAM)&ptl);
}

void  SImRichEdit::GetVisibleCharRange(CHARRANGE& chr)
{
    int nFirstVisibleLine = GetFirstVisibleLine();
    chr.cpMin = LineIndex(nFirstVisibleLine);

    CRect  rcClient;
    GetClientRect(&rcClient);

    POINT  ptRightBottom = {rcClient.right, rcClient.bottom};
    chr.cpMax = CharFromPos(ptRightBottom);
}

UINT SImRichEdit::InserSpaceLine(UINT uInsertAt/*=RECONTENT_LAST*/)
{
    const TCHAR * pFormatedSpaceLine =
        L"<RichEditContent type=\"spaceline\" >"
            L"<para break=\"1\" align=\"left\" />"
        L"</RichEditContent>";

    return InsertContent(pFormatedSpaceLine, uInsertAt);
}

LRESULT SImRichEdit::OnCreate( LPVOID )
{
    if(0 != __super::OnCreate(NULL)) return 1;


    HRESULT hr;
    IRichEditOle* pOle = NULL;
    SSendMessage(EM_GETOLEINTERFACE,0, (LPARAM)&pOle);
    hr = pOle->QueryInterface(__uuidof(ITextDocument), (void**)&m_pTxtDoc);
    SASSERT(SUCCEEDED(hr));
    pOle->Release();

    // set IME
    DWORD dw = SSendMessage(EM_GETLANGOPTIONS);
    dw |= IMF_AUTOKEYBOARD | IMF_DUALFONT;
    dw &= ~IMF_AUTOFONT;
    SSendMessage(EM_SETLANGOPTIONS, 0, IMF_UIFONTS);

    dw = SSendMessage(EM_GETEDITSTYLE);
    dw |= SES_USECTF;
    SSendMessage(EM_SETEDITSTYLE, dw, dw);

    SetOleCallback();

    return 0;
}

void SImRichEdit::OnDestroy()
{
    if (m_pTxtDoc)
    {
        m_pTxtDoc->Release();
    }
    Clear();

    SRichEdit::OnDestroy();
}

void SImRichEdit::UpdateBkgndRenderTarget()
{
    // 更新背景RenderTarget
    CRect rcWnd = GetClientRect();
    if(!m_prtBackground)
    {
        GETRENDERFACTORY->CreateRenderTarget(&m_prtBackground,rcWnd.Width(), rcWnd.Height());
    }else
    {
        m_prtBackground->Resize(rcWnd.Size());
    }
    m_prtBackground->SetViewportOrg(-rcWnd.TopLeft());
    m_bBkgndDirty = TRUE;
}

void SImRichEdit::OnSize(UINT nType, CSize size)
{
    __super::OnSize(nType, size);

    UpdateBkgndRenderTarget();

    UpdateContext ctx;
    PreUpdate(ctx);

    for (size_t npos = 0; npos < m_arrContent.GetCount(); ++npos)
    {
        m_arrContent.GetAt(npos)->UpdatePosition();
    }

    ctx.bHasScrollBar = HasScrollBar(TRUE); //强制不刷滚动条
    DoneUpdate(ctx);
}

void SImRichEdit::OnPaint(IRenderTarget * pRT)
{
    if (m_bBkgndDirty)
    {
        m_bBkgndDirty = FALSE;
        CRect rcClient = GetClientRect();
        m_prtBackground->BitBlt(rcClient, pRT, rcClient.left, rcClient.top);
    }

    CHARRANGE chrVisible;
    GetVisibleCharRange(chrVisible);

    for (size_t npos = 0; npos < m_arrContent.GetCount(); ++npos)
    {
        RichEditContent * pContent = m_arrContent.GetAt(npos);
        CHARRANGE chr = pContent->GetCharRange();

        pContent->SetDirty(TRUE);

        if (chr.cpMax < chrVisible.cpMin || chr.cpMin > chrVisible.cpMax)
        {
            continue;
        }

        pContent->DrawObject(pRT);
    }

    SRichEdit::OnPaint(pRT);
}

void SImRichEdit::DirectDraw(const CRect& rcDraw)
{
    if (GetState()&WndState_Invisible) // 不用两个&&,不用能IsVisible做判断
    {
        return;
    }

    CRect rcClient = GetClientRect();
    rcClient.IntersectRect(rcClient, rcDraw);

    CAutoRefPtr<IRegion> rgn;
    GETRENDERFACTORY->CreateRegion(&rgn);
    rgn->CombineRect(rcClient,RGN_OR);

    /*
     * 这里有两种方法去画背景
     * - 1.调用窗口的GetRenderTarget，比较正宗，但是如果richedit被嵌套的
     *     层次较深，效率就比较慢
     * 
     * - 2.自己维护缓存背景，在onsize的时候去更新背景。好处是速度较快，
     *     但是背景有可能是不准确的。
     *
     * 用哪种画背景就看情况了，如果背景是纯色的，那就用第2种咯。
     *  
     * =============================================================================
     * 对于触发gif的刷新，也有两种方法
     * - 1.触发richedit的Draw事件，让richedit去触发ole的绘制事件。
     *     这种方法比较方便，因为是richedit去触发的，不需要担心什么位置、
     *     选中状态方面的问题，但是效率较低。
     * 
     * - 2.直接传一个dc或者renderTarget给ole对象，让它去画，最后
     *     贴到窗口上。这种方法效率较高，但是需要自己处理是否被选中
     *     等状态，比较麻烦。SoSmiley就是用这种方式。
     *
     * 这里用的是第1种去触发gif刷新,效率勉强还可以接受，如果有需要再改成第2种。
    */

    /*
     * 方法1画背景
    */
    //IRenderTarget *pRT = GetRenderTarget(OLEDC_PAINTBKGND,rgn);
    //SSendMessage(WM_ERASEBKGND,(WPARAM)pRT);
    //RedrawRegion(pRT, rgn);
    //ReleaseRenderTarget(pRT);

    /*
     * 方法2画背景
    */
    CAutoRefPtr<IRenderTarget> pRT = GetContainer()->OnGetRenderTarget(rcClient, 0);
    pRT->BitBlt(rcClient, m_prtBackground, rcClient.left, rcClient.top);
    GetParent()->RedrawRegion(pRT, rgn);
    GetContainer()->OnReleaseRenderTarget(pRT, rcClient, 0);

    //STRACE(_T("%s dd time n1:%dms, n2:%dms"), m_strName, n, n2);
}

void SImRichEdit::DelayDraw(const CRect& rc)
{
    CRect rcTemp = m_rcDelayDraw;
    m_rcDelayDraw.UnionRect(rcTemp, rc);

    time_t nTicksNow = GetTickCount();
    if (nTicksNow - m_nTicksLastDraw >60) // 60ms才刷新此
    {
        DirectDraw(m_rcDelayDraw);
        m_nTicksLastDraw = nTicksNow;
        m_rcDelayDraw.SetRectEmpty();
    }
}

void SImRichEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
    BOOL bHandled = FALSE;
    POINT pt  ={point.x, point.y};
    RichEditObj * pContent = HitTest(pt);
    if (pContent != NULL)
    {
        LPARAM lParam = MAKELPARAM(pt.x, pt.y);
        pContent->ProcessMessage(WM_LBUTTONDOWN, 0, lParam, bHandled);
        if (bHandled)
        {
            SetFocus();
        }
    }

    SetMsgHandled(bHandled);
}

void SImRichEdit::OnLButtonUp(UINT nFlags, CPoint point)
{
    BOOL bHandled = FALSE;
    POINT pt  ={point.x, point.y};
    RichEditObj * pContent = HitTest(pt);
    if (pContent != NULL)
    {
        LPARAM lParam = MAKELPARAM(pt.x, pt.y);
        pContent->ProcessMessage(WM_LBUTTONUP, 0, lParam, bHandled);
    }

    SetMsgHandled(bHandled);
}

void SImRichEdit::OnRButtonDown(UINT nFlags, CPoint point)
{
    __super::OnRButtonDown(nFlags, point);
}

void SImRichEdit::OnMouseMove(UINT nFlags, CPoint point)
{
    LPARAM lParam = 0;
    POINT pt  ={point.x, point.y};

    BOOL bHandled = FALSE;
    RichEditObj * pContent = HitTest(pt);
    if (pContent != NULL)
    {
        LPARAM lParam = MAKELPARAM(pt.x, pt.y);
        pContent->ProcessMessage(WM_MOUSEMOVE, 0, lParam, bHandled);
    }

    if (pContent != m_pLastOverOle)
    {
        if (m_pLastOverOle != NULL)
        {
            m_pLastOverOle->ProcessMessage(WM_MOUSELEAVE, 0, lParam, bHandled);
        }
    }
    m_pLastOverOle = pContent;

    __super::OnMouseMove(nFlags, point);
}

BOOL SImRichEdit::OnUpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
{
    POINT point  ={pt.x, pt.y};
    RichEditObj * pContent = HitTest(pt);
    if (pContent != NULL)
    {
        CRect rcOle = pContent->GetRect();
        point.x = point.x - rcOle.left;
        point.y = point.y - rcOle.top;
        if (pContent->OnUpdateToolTip(point, tipInfo))
        {
            SIZE sizeTip = {tipInfo.rcTarget.Width(), tipInfo.rcTarget.Height()};
            tipInfo.swnd = GetSwnd();
            tipInfo.rcTarget.left += rcOle.left;
            tipInfo.rcTarget.top += rcOle.top;
            tipInfo.rcTarget.right += tipInfo.rcTarget.left + sizeTip.cx;
            tipInfo.rcTarget.bottom += tipInfo.rcTarget.top + sizeTip.cy;

            return TRUE;
        }
    }

    return __super::OnUpdateToolTip(pt, tipInfo);
}

BOOL SImRichEdit::OnSetCursor(const CPoint &point)
{
    LPARAM lParam = 0;
    POINT pt  ={point.x, point.y};
    RichEditObj * pContent = HitTest(pt);
    if (pContent != NULL)
    {
        BOOL bHandled = FALSE;
        LPARAM lParam = MAKELPARAM(pt.x, pt.y);
        pContent->ProcessMessage(WM_SETCURSOR, 0, lParam, bHandled);

        return TRUE;
    }

    if (!m_style.m_strCursor.IsEmpty())
    {
        return SWindow::OnSetCursor(point);
    }

    return SRichEdit::OnSetCursor(point);
}

BOOL SImRichEdit::OnScroll( BOOL bVertical,UINT uCode,int nPos )
{
    if(m_fScrollPending) return FALSE;
    LRESULT lresult=-1;
    m_fScrollPending=TRUE;

    SPanel::OnScroll(bVertical,uCode,nPos);
    m_pTxtHost->GetTextService()->TxSendMessage(bVertical?WM_VSCROLL:WM_HSCROLL,MAKEWPARAM(uCode,nPos),0,&lresult);
    LONG lPos=0;
    m_pTxtHost->GetTextService()->TxGetVScroll(NULL,NULL,&lPos,NULL,NULL);
    if(lPos != GetScrollPos(bVertical))
    {
        //SetScrollPos(bVertical,lPos,TRUE);
    }

    m_fScrollPending=FALSE;
    DirectDraw(m_rcClient);
    return lresult==0;
}

BOOL SImRichEdit::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    int nPos = GetScrollPos(TRUE);
    zDelta/=WHEEL_DELTA;

    int nFinalPos = nPos + zDelta * m_nScrollSpeed * -1;
    if (nFinalPos < 0)
    {
        nFinalPos = 0;
    }
    
    INT nmin, nmax;
    GetScrollRange(TRUE, &nmin, &nmax);
    if (nFinalPos == nPos)
    {
        return FALSE;
    }

    OnScroll(TRUE,SB_THUMBPOSITION, nFinalPos);

    return FALSE;
}

BOOL SImRichEdit::InsertImage(LPCWSTR lpImagePath)
{
    //ISmileySource* pSource = new CSmileySource;
    //HRESULT hr=pSource->LoadFromFile(lpImagePath);
    //if(SUCCEEDED(hr))
    //{
    //    ImageOleCtrl* pSmiley = new ImageOleCtrl();
    //    pSmiley->SetSource(pSource);
    //    pSmiley->SetSmileyHost(m_pSmileHost);
    //    pSmiley->SetHost(this);
    //    pSmiley->Init();

    //    SComPtr<IRichEditOle> ole;
    //    SSendMessage(EM_GETOLEINTERFACE,0,(LPARAM)&ole);

    //    pSmiley->Insert2Richedit((DWORD_PTR)(void*)ole);

    //    m_listOle.push_back(pSmiley);
    //}

    //pSource->Release();
    //return SUCCEEDED(hr);

    return FALSE;
}

void SImRichEdit::SetOleCallback()
{
    RichEditOleCallback * pcb = new RichEditOleCallback();
    SSendMessage(EM_SETOLECALLBACK, 0, (LPARAM)pcb);
    pcb->Release();
}

void SImRichEdit::PreUpdate(UpdateContext& context)
{
    context.bReadOnly = GetReadOnly();
    if (context.bReadOnly)
    {
        SetReadOnly(FALSE);
    }

    context.bHasScrollBar = HasScrollBar(TRUE);
}

void SImRichEdit::DoneUpdate(const UpdateContext& context)
{
    if (context.bHasScrollBar != HasScrollBar(TRUE))
    {
        //滚动条发生了变化,全部内容要重新布局
        for (size_t npos = 0; npos < m_arrContent.GetCount(); ++npos)
        {
            m_arrContent.GetAt(npos)->OffsetCharRange(0);
        }
    }

    if (context.bReadOnly)
    {
        SetReadOnly(TRUE);
    }
}

HRESULT SImRichEdit::SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pRet)
{
    LRESULT lRet = SSendMessage(uMsg, wParam, lParam);
    if(pRet) *pRet = lRet;
    return S_OK;
}

CRect SImRichEdit::GetAdjustedRect()
{
    int nContentMarginRight = 16;    // 整体内容的靠右边距

    CRect rc  = GetWindowRect();
    rc.right -= nContentMarginRight;

    return rc;
}
