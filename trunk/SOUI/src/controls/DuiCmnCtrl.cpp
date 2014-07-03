//////////////////////////////////////////////////////////////////////////
//   File Name: duicmnctrl.h
//////////////////////////////////////////////////////////////////////////

#include "duistd.h"
#include "control/DuiCmnCtrl.h"

#include <vsstyle.h>

namespace SOUI
{


//////////////////////////////////////////////////////////////////////////
// Static Control
//
void SStatic::DrawText(IRenderTarget *pRT,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat)
{
    if(!m_bMultiLines)
    {
        __super::DrawText(pRT,pszBuf,cchText,pRect,uFormat);
    }
    else
    {
        SIZE szChar;
        int i=0, nLine=1;
        if(cchText==-1) cchText=_tcslen(pszBuf);
        LPCTSTR p1=pszBuf;
        POINT pt= {pRect->left,pRect->top};
        pRT->MeasureText(_T("A"),1,&szChar);
        int nLineHei=szChar.cy;
        int nRight=pRect->right;
        pRect->right=pRect->left;
        while(i<cchText)
        {
            LPTSTR p2=CharNext(p1);
            if(*p1==_T('\\') && p2 && *p2==_T('n'))
            {
                pt.y+=nLineHei+m_nLineInter;
                pt.x=pRect->left;
                nLine++;
                i+=p2-p1;
                p1=CharNext(p2);
                i+=p1-p2;
                continue;
            }
            pRT->MeasureText(p1,p2-p1,&szChar);
            if(pt.x+szChar.cx > nRight)
            {
                pt.y+=nLineHei+m_nLineInter;
                pt.x=pRect->left;
                nLine++;
                continue;
            }
            if(!(uFormat & DT_CALCRECT))
            {
                pRT->TextOut(pt.x,pt.y,p1,p2-p1);
            }
            pt.x+=szChar.cx;
            if(pt.x>pRect->right && uFormat & DT_CALCRECT) pRect->right=pt.x;
            i+=p2-p1;
            p1=p2;
        }
        if(uFormat & DT_CALCRECT)
        {
            pRect->bottom=pt.y+nLineHei;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
// Link Control
// Only For Header Drag Test
// Usage: <link>inner text example</link>
//
void SLink::DrawText(IRenderTarget *pRT,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat)
{
    if(!(uFormat&DT_CALCRECT))
    {
        CRect rc;        
        pRT->DrawText(pszBuf,cchText,&rc,DT_LEFT|DT_CALCRECT);

        if (m_style.GetTextAlign()&DT_CENTER)
        {
            m_rcText.left = pRect->left + (pRect->right-pRect->left - rc.Width())/2;
            m_rcText.right = m_rcText.left + rc.Width();
        }
        else if (m_style.GetTextAlign()&DT_RIGHT)
        {
            m_rcText.left = pRect->right - rc.Width();
            m_rcText.right = pRect->right;
        }
        else
        {
            m_rcText.left = pRect->left;
            m_rcText.right = pRect->left + rc.Width();
        }

        if(m_style.GetTextAlign()&DT_VCENTER)
        {
            m_rcText.top=pRect->top+ (pRect->bottom-pRect->top-rc.Height())/2;
            m_rcText.bottom=m_rcText.top+rc.Height();
        }else if(m_style.GetTextAlign()&DT_BOTTOM)
        {
            m_rcText.bottom=m_rcText.bottom;
            m_rcText.top=m_rcText.bottom-rc.Height();
        }else
        {
            m_rcText.top=m_rcText.top;
            m_rcText.bottom=m_rcText.top+rc.Height();
        }
    }
    __super::DrawText(pRT,pszBuf,cchText,pRect,uFormat);
}


void SLink::OnInitFinished( pugi::xml_node xmlNode)
{
    __super::OnInitFinished(xmlNode);
    if(m_strToolTipText.IsEmpty()) m_strToolTipText=m_strLinkUrl;
}

BOOL SLink::OnSetCursor(const CPoint &pt)
{
    if(!m_rcText.PtInRect(pt)) return FALSE;
    HCURSOR hCur = ::LoadCursor(NULL, m_style.m_lpCursorName);
    ::SetCursor(hCur);
    return TRUE;
}

void SLink::OnLButtonDown( UINT nFlags,CPoint pt )
{
    if(!m_rcText.PtInRect(pt)) return;
    __super::OnLButtonDown(nFlags,pt);
}

void SLink::OnLButtonUp( UINT nFlags,CPoint pt )
{
    if(!m_rcText.PtInRect(pt))
    {
        ReleaseCapture();
        return;
    }
    __super::OnLButtonUp(nFlags,pt);
}

void SLink::OnMouseMove( UINT nFlags,CPoint pt )
{
    if(!m_rcText.PtInRect(pt))
    {
        if(m_dwState&DuiWndState_Hover) OnMouseLeave();
    }
    else
    {
        if(!(m_dwState&DuiWndState_Hover)) OnMouseHover(nFlags,pt);
    }
}

void SLink::OnMouseHover( WPARAM wParam, CPoint pt )
{
    if(!m_rcText.PtInRect(pt)) return;
    __super::OnMouseHover(wParam,pt);
}
//////////////////////////////////////////////////////////////////////////
// Button Control
// Use id attribute to process click event
//
// Usage: <button name=xx skin=xx>inner text example</button>
//

SButton::SButton() :m_accel(0),m_bAnimate(FALSE),m_byAlphaAni(0xFF)
{
    m_bTabStop=TRUE;
}

void SButton::OnPaint(IRenderTarget *pRT)
{
    if (!m_pBgSkin) return;
    CRect rcClient;
    GetClient(&rcClient);

    if(m_byAlphaAni==0xFF)
    {//不在动画过程中
        m_pBgSkin->Draw(
            pRT, rcClient,
            IIF_STATE4(GetState(), 0, 1, 2, 3),m_byAlpha
            );
    }
    else
    {//在动画过程中
        BYTE byNewAlpha=(BYTE)(((UINT)m_byAlphaAni*m_byAlpha)>>8);
        if(GetState()&DuiWndState_Hover)
        {
            //get hover
            m_pBgSkin->Draw(pRT, rcClient, 0, m_byAlpha);
            m_pBgSkin->Draw(pRT, rcClient, 1, byNewAlpha);
        }
        else
        {
            //lose hover
            m_pBgSkin->Draw(pRT, rcClient,0, m_byAlpha);
            m_pBgSkin->Draw(pRT, rcClient, 1, m_byAlpha-byNewAlpha);
        }
    }

    __super::OnPaint(pRT);
}

void SButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if(nChar==VK_SPACE || nChar==VK_RETURN)
    {
        ModifyState(DuiWndState_PushDown,0,TRUE);
    }else
    {
        SetMsgHandled(FALSE);
    }
}

void SButton::OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    if(nChar==VK_SPACE || nChar==VK_RETURN)
    {
        ModifyState(0,DuiWndState_PushDown,TRUE);
        FireCommand();
    }else
    {
        SetMsgHandled(FALSE);
    }
}

bool SButton::OnAcceleratorPressed( const CAccelerator& accelerator )
{
    if(IsDisabled(TRUE)) return false;
    FireCommand();
    return true;
}

void SButton::OnDestroy()
{
    if(m_accel)
    {
        CAccelerator acc(m_accel);
        GetContainer()->GetAcceleratorMgr()->UnregisterAccelerator(acc,this);
    }
    StopCurAnimate();
    __super::OnDestroy();
}

HRESULT SButton::OnAttrAccel( SStringW strAccel,BOOL bLoading )
{
    SStringT strAccelT=DUI_CW2T(strAccel);
    m_accel=CAccelerator::TranslateAccelKey(strAccelT);
    if(m_accel)
    {
        CAccelerator acc(m_accel);
        GetContainer()->GetAcceleratorMgr()->RegisterAccelerator(acc,this);
        return S_OK;
    }
    return S_FALSE;
}

CSize SButton::GetDesiredSize( LPRECT pRcContainer )
{
    ASSERT(m_pBgSkin);
    CSize szRet=m_pBgSkin->GetSkinSize();
    if(szRet.cx==0 || szRet.cy==0)
        szRet=__super::GetDesiredSize(pRcContainer);
    return szRet;
}

void SButton::OnStateChanged( DWORD dwOldState,DWORD dwNewState )
{
    StopCurAnimate();

    if(GetCapture()==m_hSWnd)    //点击中
        return;

    if(m_bAnimate &&
        ((dwOldState==DuiWndState_Normal && dwNewState==DuiWndState_Hover)
        ||(dwOldState==DuiWndState_Hover && dwNewState==DuiWndState_Normal)))
    {//启动动画
        m_byAlphaAni=5;
        GetContainer()->RegisterTimelineHandler(this);
    }
}

void SButton::OnSize( UINT nType, CSize size )
{
    StopCurAnimate();
}

//中止原来的动画
void SButton::StopCurAnimate()
{
    GetContainer()->UnregisterTimelineHandler(this);
    m_byAlphaAni=0xFF;
}

void SButton::OnNextFrame()
{
    m_byAlphaAni+=25;
    if(m_byAlphaAni==0xFF) StopCurAnimate();
    Invalidate();
}

//////////////////////////////////////////////////////////////////////////
// Image Control
// Use src attribute specify a resource id
//
// Usage: <img skin="skin" sub="0"/>
//
SImageWnd::SImageWnd()
    : m_iFrame(0)
    , m_pSkin(NULL)
    , m_bManaged(FALSE)
{
    m_bMsgTransparent=TRUE;
}

SImageWnd::~SImageWnd()
{
    if(m_bManaged && m_pSkin)
    {
        m_pSkin->Release();
    }
    m_pSkin=NULL;
}

void SImageWnd::OnPaint(IRenderTarget *pRT)
{
    if (m_pSkin)
        m_pSkin->Draw(pRT, m_rcWindow, m_iFrame,m_byAlpha);
}

BOOL SImageWnd::SetSkin(ISkinObj *pSkin,int iFrame/*=0*/,BOOL bAutoFree/*=TRUE*/)
{
    if(IsVisible(TRUE)) Invalidate();
    if(m_bManaged && m_pSkin)
    {
        m_pSkin->Release();
        m_bManaged=FALSE;
    }
    if(!pSkin) return FALSE;
    m_pSkin=pSkin;
    m_iFrame=iFrame;
    
    if(bAutoFree)
    {
        m_pSkin->AddRef();
        m_bManaged=TRUE;
    }else
    {
        m_bManaged=FALSE;
    }

    ASSERT(GetParent());

    if(m_dlgpos.nCount==2)
    {
        //重新计算坐标
        SendMessage(WM_WINDOWPOSCHANGED);
    }
    if(IsVisible(TRUE)) Invalidate();
    return TRUE;
}

BOOL SImageWnd::SetIcon( int nSubID )
{
    if(!m_pSkin) return FALSE;
    if(nSubID<0 || nSubID>m_pSkin->GetStates()-1) return FALSE;
    m_iFrame=nSubID;
    return TRUE;
}

CSize SImageWnd::GetDesiredSize(LPRECT pRcContainer)
{
    CSize szRet;
    if(m_pSkin) szRet=m_pSkin->GetSkinSize();
    return szRet;
}

SAnimateImgWnd::SAnimateImgWnd()
:m_pSkin(NULL)
,m_iCurFrame(0)
,m_nSpeed(50)
,m_bAutoStart(TRUE)
,m_bPlaying(FALSE)
{
    m_bMsgTransparent=TRUE;
}

void SAnimateImgWnd::OnPaint(IRenderTarget *pRT)
{
    if (m_pSkin)
        m_pSkin->Draw(pRT, m_rcWindow, m_iCurFrame,m_byAlpha);
}


void SAnimateImgWnd::Start()
{
    if(!m_bPlaying)
    {
        if(IsVisible(TRUE)) GetContainer()->RegisterTimelineHandler(this);
        m_bPlaying=TRUE;
    }
}

void SAnimateImgWnd::Stop()
{
    if(m_bPlaying)
    {
        if(IsVisible(TRUE)) GetContainer()->UnregisterTimelineHandler(this);
        m_bPlaying=FALSE;
    }
}

void SAnimateImgWnd::OnDestroy()
{
    Stop();
}

CSize SAnimateImgWnd::GetDesiredSize(LPRECT pRcContainer)
{
    CSize szRet;
    if(m_pSkin) szRet=m_pSkin->GetSkinSize();
    return szRet;
}

void SAnimateImgWnd::OnShowWindow( BOOL bShow, UINT nStatus )
{
    __super::OnShowWindow(bShow,nStatus);
    if(!bShow)
    {
        if(IsPlaying()) GetContainer()->UnregisterTimelineHandler(this);
    }else
    {
        if(IsPlaying()) GetContainer()->RegisterTimelineHandler(this);
        else if(m_bAutoStart) Start();
    }
}

void SAnimateImgWnd::OnNextFrame()
{
    if(!m_pSkin) GetContainer()->UnregisterTimelineHandler(this);
    else
    {
        static int nFrame=0;
        if(nFrame > (m_nSpeed/10)) nFrame=0;
        if(nFrame==0)
        {
            int nStates=m_pSkin->GetStates();
            m_iCurFrame++;
            m_iCurFrame%=nStates;
            Invalidate();
        }
        nFrame++;
    }
}
//////////////////////////////////////////////////////////////////////////
// Progress Control
// Use id attribute to process click event
//
// Usage: <progress bgskin=xx posskin=xx min=0 max=100 value=10,showpercent=0/>
//

SProgress::SProgress()
    : m_nMinValue(0)
    , m_nMaxValue(100)
    , m_nValue(0)
    , m_bShowPercent(FALSE)
    , m_pSkinBg(NULL)
    , m_pSkinPos(NULL)
    , m_bVertical(FALSE)
{

}


CSize SProgress::GetDesiredSize(LPRECT pRcContainer)
{
    CSize szRet;
    SIZE sizeBg = m_pSkinBg->GetSkinSize();
    if(IsVertical())
    {
        szRet.cx = sizeBg.cx + 2 * m_style.m_nMarginX;
        if(m_dlgpos.uPositionType & SizeY_Specify)
            szRet.cy=m_dlgpos.uSpecifyHeight;
        else
            szRet.cy = sizeBg.cy +2 * m_style.m_nMarginY;
    }else
    {
        szRet.cy = sizeBg.cy + 2 * m_style.m_nMarginY;
        if(m_dlgpos.uPositionType & SizeX_Specify)
            szRet.cx=m_dlgpos.uSpecifyWidth;
        else
            szRet.cx = sizeBg.cx +2 * m_style.m_nMarginX;
    }
    return szRet;
}

void SProgress::OnPaint(IRenderTarget *pRT)
{
    SPainter DuiDC;

    BeforePaint(pRT, DuiDC);

    ASSERT(m_pSkinBg && m_pSkinPos);
    
    CRect rcClient;
    GetClient(&rcClient);
    m_pSkinBg->Draw(pRT, rcClient, DuiWndState_Normal,m_byAlpha);
    CRect rcValue=rcClient;

    if(IsVertical())
    {
        rcValue.bottom=rcValue.top+rcValue.Height()*(m_nValue-m_nMinValue)/(m_nMaxValue-m_nMinValue);
    }
    else
    {
        rcValue.right=rcValue.left+rcValue.Width()*(m_nValue-m_nMinValue)/(m_nMaxValue-m_nMinValue);
    }
    if(m_nValue>m_nMinValue)
    {
        m_pSkinPos->Draw(pRT, rcValue, DuiWndState_Normal,m_byAlpha);
    }


    if (m_bShowPercent && !IsVertical())
    {
        SStringT strPercent;
        strPercent.Format(_T("%d%%"), (int)((m_nValue-m_nMinValue) * 100/(m_nMaxValue-m_nMinValue)));
        pRT->DrawText(strPercent, strPercent.GetLength(), m_rcWindow, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    }
    AfterPaint(pRT, DuiDC);
}

BOOL SProgress::SetValue(int dwValue)
{
    if(dwValue<m_nMinValue || dwValue>m_nMaxValue) return FALSE;
    m_nValue=dwValue;
    
    Invalidate();
    return TRUE;
}

void SProgress::SetRange( int nMin,int nMax )
{
    ASSERT(nMax>nMin);
    m_nMaxValue=nMax;
    m_nMinValue=nMin;
    if(m_nValue>m_nMaxValue) m_nValue=m_nMaxValue;
    if(m_nValue<nMin) m_nValue=m_nMinValue;
    Invalidate();
}

void SProgress::GetRange( int *pMin,int *pMax )
{
    if(pMin) *pMin=m_nMinValue;
    if(pMax) *pMax=m_nMaxValue;
}

//////////////////////////////////////////////////////////////////////////
// Line Control
// Simple HTML "HR" tag
//
// Usage: <hr style=solid size=1 crbg=.../>
//

SLine::SLine()
    : m_nLineStyle(PS_SOLID)
    , m_nLineSize(1)
    , m_mode(HR_HORZ)
{
}

void SLine::OnPaint(IRenderTarget *pRT)
{
    CPoint pts[2];
    pts[0]=m_rcWindow.TopLeft();
    switch(m_mode)
    {
    case HR_HORZ:pts[1].x=m_rcWindow.right,pts[1].y=m_rcWindow.top;break;
    case HR_VERT:pts[1].x=m_rcWindow.left,pts[1].y=m_rcWindow.bottom;break;
    case HR_TILT:pts[1]=m_rcWindow.BottomRight();break;
    }
    CAutoRefPtr<IPen> curPen,oldPen;
    pRT->CreatePen(m_nLineStyle,m_style.m_crBg,m_nLineSize,&curPen);
    pRT->SelectObject(curPen,(IRenderObj**)&oldPen);
    pRT->DrawLines(pts,2);
    pRT->SelectObject(oldPen);
}

//////////////////////////////////////////////////////////////////////////
// Check Box
//
// Usage: <check state=1>This is a check-box</check>
//

SCheckBox::SCheckBox()
    : m_pSkin(GETSKIN(L"btncheckbox"))
    , m_pFocusSkin(GETSKIN(L"focuscheckbox"))
{
    m_bTabStop=TRUE;
}


CRect SCheckBox::GetCheckRect()
{
    CRect rcClient;
    GetClient(rcClient);
    ASSERT(m_pSkin);
    CSize szCheck=m_pSkin->GetSkinSize();
    CRect rcCheckBox(rcClient.TopLeft(),szCheck);
    rcCheckBox.OffsetRect(0,(rcClient.Height()-szCheck.cy)/2);
    return rcCheckBox;
}

void SCheckBox::GetTextRect( LPRECT pRect )
{
    GetClient(pRect);
    ASSERT(m_pSkin);
    CSize szCheck=m_pSkin->GetSkinSize();
    pRect->left+=szCheck.cx+CheckBoxSpacing;    
}

void SCheckBox::OnPaint(IRenderTarget *pRT)
{
    CRect rcCheckBox=GetCheckRect();
    m_pSkin->Draw(pRT, rcCheckBox, _GetDrawState());
    __super::OnPaint(pRT);
}

void SCheckBox::DrawFocus(IRenderTarget *pRT)
{
    if(m_pFocusSkin)
    {
        CRect rcCheckBox=GetCheckRect();
        m_pFocusSkin->Draw(pRT,rcCheckBox,0);
    }else
    {
        __super::DrawFocus(pRT);
    }
}

CSize SCheckBox::GetDesiredSize(LPRECT pRcContainer)
{
    ASSERT(m_pSkin);
    CSize szCheck=m_pSkin->GetSkinSize();
    CSize szRet=__super::GetDesiredSize(pRcContainer);
    szRet.cx+=szCheck.cx + CheckBoxSpacing;
    szRet.cy=max(szRet.cy, szCheck.cy);
    return szRet;
}


UINT SCheckBox::_GetDrawState()
{
    DWORD dwState = GetState();
    UINT uState = 0;

    if (m_pSkin)
    {
        if (dwState & DuiWndState_Check)
        {
            if (dwState & DuiWndState_Disable)
                uState = CBS_CHECKEDDISABLED;
            else if (dwState & DuiWndState_PushDown)
                uState = CBS_CHECKEDPRESSED;
            else if (dwState & DuiWndState_Hover)
                uState = CBS_CHECKEDHOT;
            else
                uState = CBS_CHECKEDNORMAL;
        }
        else
        {
            if (dwState & DuiWndState_Disable)
                uState = CBS_UNCHECKEDDISABLED;
            else if (dwState & DuiWndState_PushDown)
                uState = CBS_UNCHECKEDPRESSED;
            else if (dwState & DuiWndState_Hover)
                uState = CBS_UNCHECKEDHOT;
            else
                uState = CBS_UNCHECKEDNORMAL;
        }
    }

    --uState;    // 减1

    return uState;
}

void SCheckBox::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetFocus();
    __super::OnLButtonDown(nFlags,point);
}

void SCheckBox::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (IsChecked())
        ModifyState(0, DuiWndState_Check,TRUE);
    else
        ModifyState(DuiWndState_Check, 0,TRUE);
    __super::OnLButtonUp(nFlags,point);
}

void SCheckBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if(nChar==VK_SPACE)
    {
        if (IsChecked())
            ModifyState(0, DuiWndState_Check,TRUE);
        else
            ModifyState(DuiWndState_Check, 0,TRUE);

        FireCommand();
    }
}

//////////////////////////////////////////////////////////////////////////
// Icon Control

SIconWnd::SIconWnd():m_theIcon(0)
{

}

SIconWnd::~SIconWnd()
{
    if(m_theIcon) DeleteObject(m_theIcon);
}

void SIconWnd::OnPaint(IRenderTarget *pRT)
{
    CRect rcClient;
    GetClient(&rcClient);
    pRT->DrawIconEx(rcClient.left,rcClient.top,m_theIcon,rcClient.Width(),rcClient.Height(),DI_NORMAL);
}


CSize SIconWnd::GetDesiredSize(LPRECT pRcContainer)
{
    if(!m_theIcon) return CSize();
    ICONINFO iconInfo={0};
    GetIconInfo(m_theIcon,&iconInfo);
    BITMAP bm={0};
    GetObject(&iconInfo.hbmColor,sizeof(bm),&bm);
    
    return CSize(bm.bmWidth,bm.bmHeight);
}

void SIconWnd::SetIcon(HICON hIcon)
{
    if(m_theIcon) DeleteObject(m_theIcon);
    m_theIcon = hIcon;
    Invalidate();
}

//////////////////////////////////////////////////////////////////////////
// Radio Box
//
// Usage: <radio state=1>This is a check-box</radio>
//

SRadioBox::SRadioBox()
    : m_pSkin(GETSKIN(L"btnRadio"))
    , m_pFocusSkin(GETSKIN(L"focusRadio"))
{
    m_bTabStop=TRUE;
}


CRect SRadioBox::GetRadioRect()
{
    CRect rcClient;
    GetClient(rcClient);
    ASSERT(m_pSkin);
    CSize szRadioBox=m_pSkin->GetSkinSize();
    CRect rcRadioBox(rcClient.TopLeft(),szRadioBox);
    rcRadioBox.OffsetRect(0,(rcClient.Height()-szRadioBox.cy)/2);
    return rcRadioBox;
}


void SRadioBox::GetTextRect( LPRECT pRect )
{
    GetClient(pRect);
    ASSERT(m_pSkin);
    CSize szRadioBox=m_pSkin->GetSkinSize();
    pRect->left+=szRadioBox.cx+RadioBoxSpacing;
}

void SRadioBox::OnPaint(IRenderTarget *pRT)
{
    ASSERT(m_pSkin);
    CRect rcRadioBox=GetRadioRect();
    m_pSkin->Draw(pRT, rcRadioBox, _GetDrawState());
    __super::OnPaint(pRT);
}

void SRadioBox::DrawFocus(IRenderTarget *pRT)
{
    if(m_pFocusSkin)
    {
        CRect rcCheckBox=GetRadioRect();
        m_pFocusSkin->Draw(pRT,rcCheckBox,0);
    }else
    {
        __super::DrawFocus(pRT);
    }
}


CSize SRadioBox::GetDesiredSize(LPRECT pRcContainer)
{
    CSize szRet=__super::GetDesiredSize(pRcContainer);
    CSize szRaio=m_pSkin->GetSkinSize();
    szRet.cx+=szRaio.cx + RadioBoxSpacing;
    szRet.cy=max(szRet.cy,szRaio.cy);
    return szRet;
}


UINT SRadioBox::_GetDrawState()
{
    DWORD dwState = GetState();
    UINT uState = 0;

    if (dwState & DuiWndState_Check)
    {
        if (dwState & DuiWndState_Disable)
            uState = RBS_CHECKEDDISABLED;
        else if (dwState & DuiWndState_PushDown)
            uState = RBS_CHECKEDPRESSED;
        else if (dwState & DuiWndState_Hover)
            uState = RBS_CHECKEDHOT;
        else
            uState = RBS_CHECKEDNORMAL;
    }
    else
    {
        if (dwState & DuiWndState_Disable)
            uState = RBS_UNCHECKEDDISABLED;
        else if (dwState & DuiWndState_PushDown)
            uState = RBS_UNCHECKEDPRESSED;
        else if (dwState & DuiWndState_Hover)
            uState = RBS_UNCHECKEDHOT;
        else
            uState = RBS_UNCHECKEDNORMAL;
    }

    --uState;

    return uState;
}

BOOL SRadioBox::NeedRedrawWhenStateChange()
{
    return TRUE;
}

void SRadioBox::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetFocus();
    __super::OnLButtonDown(nFlags,point);
}

void SRadioBox::OnSetFocus()
{
    SWindow *pParent=GetParent();
    pParent->CheckRadioButton(this);
    Invalidate();
}

SWindow * SRadioBox::GetSelectedSiblingInGroup()
{
    SWindow *pParent=GetParent();
    ASSERT(pParent);
    SWindow *pSibling=pParent->GetWindow(GDUI_FIRSTCHILD);
    while(pSibling)
    {
        if(pSibling->IsClass(GetClassName()))
        {
            SRadioBox * pRadio=(SRadioBox*)pSibling;
            if(pRadio->IsChecked()) return pRadio;
        }
        pSibling=pSibling->GetWindow(GDUI_NEXTSIBLING);
    }
    return this;
}

//////////////////////////////////////////////////////////////////////////
// CDuiToggle
SToggle::SToggle():m_bToggled(FALSE),m_pSkin(NULL)
{

}

void SToggle::SetToggle(BOOL bToggle,BOOL bUpdate/*=TRUE*/)
{
    m_bToggled=bToggle;
    if(bUpdate) Invalidate();
}

BOOL SToggle::GetToggle()
{
    return m_bToggled;
}

void SToggle::OnPaint(IRenderTarget *pRT)
{
    ASSERT(m_pSkin);
    DWORD nState=0;
    if(GetState()&DuiWndState_Hover) nState=2;
    else if(GetState()&DuiWndState_Check) nState=3;
    if(m_bToggled) nState+=3;
    m_pSkin->Draw(pRT,m_rcWindow,nState);
}

void SToggle::OnLButtonUp(UINT nFlags,CPoint pt)
{
    m_bToggled=!m_bToggled;
    __super::OnLButtonUp(nFlags,pt);
}

CSize SToggle::GetDesiredSize(LPRECT pRcContainer)
{
    CSize sz;
    if(m_pSkin) sz=m_pSkin->GetSkinSize();
    return sz;
}

#define GROUP_HEADER        20
#define GROUP_ROUNDCORNOR    4

SGroup::SGroup():m_nRound(GROUP_ROUNDCORNOR),m_crLine1(RGB(0xF0,0xF0,0xF0)),m_crLine2(RGB(0xA0,0xA0,0xA0))
{
}

void SGroup::OnPaint(IRenderTarget *pRT)
{

    SPainter DuiDC;

    BeforePaint(pRT, DuiDC);

    CSize szFnt;
    pRT->MeasureText(m_strText, m_strText.GetLength(),&szFnt);

    CRect rcText=m_rcWindow;
    rcText.left+=GROUP_HEADER,rcText.right-=GROUP_HEADER;
    rcText.bottom=rcText.top+szFnt.cy+2;
    if(GetTextAlign() & DT_CENTER)
    {
        rcText.left+=(rcText.Width()-szFnt.cx)/2;
        rcText.right=rcText.left+szFnt.cx;
    }
    else if(GetTextAlign() & DT_RIGHT)
    {
        rcText.left=rcText.right-szFnt.cx;
    }
    else
    {
        rcText.right=rcText.left+szFnt.cx;
    }

    
    if(!m_strText.IsEmpty())
    {
        CRect rcClip=rcText;
        rcClip.InflateRect(5,5,5,5);
        pRT->PushClipRect(&rcClip,RGN_DIFF);
    }

    {
        CRect rcGroupBox = m_rcWindow;

        if(!m_strText.IsEmpty()) rcGroupBox.top+=szFnt.cy/2;
        rcGroupBox.DeflateRect(1,1,1,0);
        
        CAutoRefPtr<IPen> pen1,pen2,oldPen;
        pRT->CreatePen(PS_SOLID,m_crLine1,1,&pen1);
        pRT->CreatePen(PS_SOLID,m_crLine2,1,&pen2);
        pRT->SelectObject(pen1,(IRenderObj**)&oldPen);
        pRT->DrawRoundRect(&rcGroupBox,CPoint(m_nRound,m_nRound));
              
        pRT->SelectObject(pen2);
        rcGroupBox.InflateRect(1,1,1,-1);
        pRT->DrawRoundRect(&rcGroupBox,CPoint(m_nRound,m_nRound));

        pRT->SelectObject(oldPen);
    }

    if(!m_strText.IsEmpty())
    {
        pRT->DrawText(m_strText, m_strText.GetLength(), rcText, DT_SINGLELINE|DT_VCENTER);
        pRT->PopClip();
    }

    AfterPaint(pRT, DuiDC);
}

}//namespace SOUI