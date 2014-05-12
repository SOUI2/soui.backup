//////////////////////////////////////////////////////////////////////////
//   File Name: duicmnctrl.h
//////////////////////////////////////////////////////////////////////////

#include "duistd.h"
#include "DuiCmnCtrl.h"

#include "duiwndnotify.h"
#include "duisystem.h"
#include <vsstyle.h>

namespace SOUI
{


//////////////////////////////////////////////////////////////////////////
// Static Control
//
void CDuiStatic::DuiDrawText(HDC hdc,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat)
{
    if(!m_bMultiLines)
    {
        __super::DuiDrawText(hdc,pszBuf,cchText,pRect,uFormat);
    }
    else
    {
        SIZE szChar;
        int i=0, nLine=1;
        if(cchText==-1) cchText=_tcslen(pszBuf);
        LPCTSTR p1=pszBuf;
        POINT pt= {pRect->left,pRect->top};
        GetTextExtentPoint(hdc,_T("A"),1,&szChar);
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
            GetTextExtentPoint(hdc,p1,p2-p1,&szChar);
            if(pt.x+szChar.cx > nRight)
            {
                pt.y+=nLineHei+m_nLineInter;
                pt.x=pRect->left;
                nLine++;
                continue;
            }
            if(!(uFormat & DT_CALCRECT))
            {
                CGdiAlpha::TextOut(hdc,pt.x,pt.y,p1,p2-p1);
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
void CDuiLink::DuiDrawText(HDC hdc,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat)
{
	if(!(uFormat&DT_CALCRECT))
	{
		CRect rc;		
		DrawText(hdc,pszBuf,cchText,&rc,DT_LEFT|DT_CALCRECT);

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
	__super::DuiDrawText(hdc,pszBuf,cchText,pRect,uFormat);
}


void CDuiLink::OnAttributeFinish( pugi::xml_node xmlNode)
{
	__super::OnAttributeFinish(xmlNode);
    if(m_strToolTipText.IsEmpty()) m_strToolTipText=m_strLinkUrl;
}

BOOL CDuiLink::OnDuiSetCursor(const CPoint &pt)
{
    if(!m_rcText.PtInRect(pt)) return FALSE;
    HCURSOR hCur = ::LoadCursor(NULL, m_style.m_lpCursorName);
    ::SetCursor(hCur);
    return TRUE;
}

void CDuiLink::OnLButtonDown( UINT nFlags,CPoint pt )
{
    if(!m_rcText.PtInRect(pt)) return;
    __super::OnLButtonDown(nFlags,pt);
}

void CDuiLink::OnLButtonUp( UINT nFlags,CPoint pt )
{
    if(!m_rcText.PtInRect(pt))
    {
        ReleaseDuiCapture();
        return;
    }
    __super::OnLButtonUp(nFlags,pt);
}

void CDuiLink::OnMouseMove( UINT nFlags,CPoint pt )
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

void CDuiLink::OnMouseHover( WPARAM wParam, CPoint pt )
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

CDuiButton::CDuiButton() :m_accel(0),m_bAnimate(FALSE),m_byAlphaAni(0xFF)
{
	m_bTabStop=TRUE;
}

void CDuiButton::OnPaint(CDCHandle dc)
{
	if (!m_pBgSkin) return;
	CRect rcClient;
	GetClient(&rcClient);

	if(m_byAlphaAni==0xFF)
	{//不在动画过程中
		m_pBgSkin->Draw(
			dc, rcClient,
			IIF_STATE4(GetState(), 0, 1, 2, 3),m_byAlpha
			);
	}
	else
	{//在动画过程中
		BYTE byNewAlpha=(BYTE)(((UINT)m_byAlphaAni*m_byAlpha)>>8);
		if(GetState()&DuiWndState_Hover)
		{
			//get hover
			m_pBgSkin->Draw(dc, rcClient, 0, m_byAlpha);
			m_pBgSkin->Draw(dc, rcClient, 1, byNewAlpha);
		}
		else
		{
			//lose hover
			m_pBgSkin->Draw(dc, rcClient,0, m_byAlpha);
			m_pBgSkin->Draw(dc, rcClient, 1, m_byAlpha-byNewAlpha);
		}
	}

    __super::OnPaint(dc);
}

void CDuiButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if(nChar==VK_SPACE || nChar==VK_RETURN)
    {
		ModifyState(DuiWndState_PushDown,0,TRUE);
    }else
	{
		SetMsgHandled(FALSE);
	}
}

void CDuiButton::OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	if(nChar==VK_SPACE || nChar==VK_RETURN)
	{
		ModifyState(0,DuiWndState_PushDown,TRUE);
		NotifyCommand();
	}else
	{
		SetMsgHandled(FALSE);
	}
}

bool CDuiButton::OnAcceleratorPressed( const CAccelerator& accelerator )
{
	if(IsDisabled(TRUE)) return false;
	NotifyCommand();
	return true;
}

void CDuiButton::OnDestroy()
{
	if(m_accel)
	{
		CAccelerator acc(m_accel);
		GetContainer()->GetAcceleratorMgr()->UnregisterAccelerator(acc,this);
	}
	StopCurAnimate();
	__super::OnDestroy();
}

HRESULT CDuiButton::OnAttrAccel( CDuiStringA strAccel,BOOL bLoading )
{
	CDuiStringT strAccelT=DUI_CA2T(strAccel,CP_UTF8);
	m_accel=CAccelerator::TranslateAccelKey(strAccelT);
	if(m_accel)
	{
		CAccelerator acc(m_accel);
		GetContainer()->GetAcceleratorMgr()->RegisterAccelerator(acc,this);
		return S_OK;
	}
	return S_FALSE;
}

CSize CDuiButton::GetDesiredSize( LPRECT pRcContainer )
{
	DUIASSERT(m_pBgSkin);
	CSize szRet=m_pBgSkin->GetSkinSize();
	if(szRet.cx==0 || szRet.cy==0)
		szRet=__super::GetDesiredSize(pRcContainer);
	return szRet;
}

void CDuiButton::OnStateChanged( DWORD dwOldState,DWORD dwNewState )
{
	StopCurAnimate();

	if(GetDuiCapture()==m_hDuiWnd)	//点击中
		return;

	if(m_bAnimate &&
		((dwOldState==DuiWndState_Normal && dwNewState==DuiWndState_Hover)
		||(dwOldState==DuiWndState_Hover && dwNewState==DuiWndState_Normal)))
	{//启动动画
		m_byAlphaAni=5;
		GetContainer()->RegisterTimelineHandler(this);
	}
}

void CDuiButton::OnSize( UINT nType, CSize size )
{
	StopCurAnimate();
}

//中止原来的动画
void CDuiButton::StopCurAnimate()
{
	GetContainer()->UnregisterTimelineHandler(this);
	m_byAlphaAni=0xFF;
}

void CDuiButton::OnNextFrame()
{
	m_byAlphaAni+=25;
	if(m_byAlphaAni==0xFF) StopCurAnimate();
	NotifyInvalidate();
}

//////////////////////////////////////////////////////////////////////////
// Image Control
// Use src attribute specify a resource id
//
// Usage: <img skin="skin" sub="0"/>
//
CDuiImageWnd::CDuiImageWnd()
    : m_nSubImageID(0)
    , m_pSkin(NULL)
    , m_bManaged(FALSE)
{
    m_bMsgTransparent=TRUE;
}

CDuiImageWnd::~CDuiImageWnd()
{
    if(m_bManaged && m_pSkin)
    {
        m_pSkin->Release();
    }
    m_pSkin=NULL;
}

void CDuiImageWnd::OnPaint(CDCHandle dc)
{
    if (m_pSkin)
        m_pSkin->Draw(dc, m_rcWindow, m_nSubImageID,m_byAlpha);
}

BOOL CDuiImageWnd::SetSkin(CDuiSkinBase *pSkin,int nSubID/*=0*/)
{
    if(IsVisible(TRUE)) NotifyInvalidate();
    if(m_bManaged && m_pSkin)
    {
        m_pSkin->Release();
        m_bManaged=FALSE;
    }
    if(!pSkin) return FALSE;
    m_pSkin=pSkin;
    m_pSkin->AddRef();
    m_bManaged=TRUE;
    m_nSubImageID=nSubID;

	DUIASSERT(GetParent());

    if(m_dlgpos.nCount==2)
    {
        //重新计算坐标
		DuiSendMessage(WM_WINDOWPOSCHANGED);
    }
    if(IsVisible(TRUE)) NotifyInvalidate();
    return TRUE;
}

BOOL CDuiImageWnd::SetIcon( int nSubID )
{
    if(!m_pSkin) return FALSE;
    if(nSubID<0 || nSubID>m_pSkin->GetStates()-1) return FALSE;
    m_nSubImageID=nSubID;
    return TRUE;
}

CSize CDuiImageWnd::GetDesiredSize(LPRECT pRcContainer)
{
	CSize szRet;
	if(m_pSkin) szRet=m_pSkin->GetSkinSize();
	return szRet;
}

CDuiAnimateImgWnd::CDuiAnimateImgWnd()
:m_pSkin(NULL)
,m_iCurFrame(0)
,m_nSpeed(50)
,m_bAutoStart(TRUE)
,m_bPlaying(FALSE)
{
    m_bMsgTransparent=TRUE;
}

void CDuiAnimateImgWnd::OnPaint(CDCHandle dc)
{
    if (m_pSkin)
        m_pSkin->Draw(dc, m_rcWindow, m_iCurFrame,m_byAlpha);
}


void CDuiAnimateImgWnd::Start()
{
    if(!m_bPlaying)
	{
		if(IsVisible(TRUE)) GetContainer()->RegisterTimelineHandler(this);
		m_bPlaying=TRUE;
	}
}

void CDuiAnimateImgWnd::Stop()
{
	if(m_bPlaying)
	{
		if(IsVisible(TRUE)) GetContainer()->UnregisterTimelineHandler(this);
		m_bPlaying=FALSE;
	}
}

void CDuiAnimateImgWnd::OnDestroy()
{
    Stop();
}

CSize CDuiAnimateImgWnd::GetDesiredSize(LPRECT pRcContainer)
{
	CSize szRet;
	if(m_pSkin) szRet=m_pSkin->GetSkinSize();
	return szRet;
}

void CDuiAnimateImgWnd::OnShowWindow( BOOL bShow, UINT nStatus )
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

void CDuiAnimateImgWnd::OnNextFrame()
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
			NotifyInvalidate();
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

CDuiProgress::CDuiProgress()
    : m_nMinValue(0)
    , m_nMaxValue(100)
    , m_nValue(0)
    , m_bShowPercent(FALSE)
    , m_pSkinBg(NULL)
    , m_pSkinPos(NULL)
	, m_bVertical(FALSE)
{

}


CSize CDuiProgress::GetDesiredSize(LPRECT pRcContainer)
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

void CDuiProgress::OnPaint(CDCHandle dc)
{
    DuiDCPaint DuiDC;

    BeforePaint(dc, DuiDC);

	DUIASSERT(m_pSkinBg && m_pSkinPos);
	
	m_pSkinBg->Draw(dc, DuiDC.rcClient, DuiWndState_Normal,m_byAlpha);
	CRect rcValue=DuiDC.rcClient;

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
		m_pSkinPos->Draw(dc, rcValue, DuiWndState_Normal,m_byAlpha);
	}


	if (m_bShowPercent && !IsVertical())
	{
		CDuiStringT strPercent;
		strPercent.Format(_T("%d%%"), (int)((m_nValue-m_nMinValue) * 100/(m_nMaxValue-m_nMinValue)));
		CGdiAlpha::DrawText(dc,strPercent, strPercent.GetLength(), m_rcWindow, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
    AfterPaint(dc, DuiDC);
}

BOOL CDuiProgress::SetValue(int dwValue)
{
    if(dwValue<m_nMinValue || dwValue>m_nMaxValue) return FALSE;
	m_nValue=dwValue;
	
	NotifyInvalidate();
	return TRUE;
}

void CDuiProgress::SetRange( int nMin,int nMax )
{
	DUIASSERT(nMax>nMin);
	m_nMaxValue=nMax;
	m_nMinValue=nMin;
	if(m_nValue>m_nMaxValue) m_nValue=m_nMaxValue;
	if(m_nValue<nMin) m_nValue=m_nMinValue;
	NotifyInvalidate();
}

void CDuiProgress::GetRange( int *pMin,int *pMax )
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

CDuiLine::CDuiLine()
    : m_nPenStyle(PS_SOLID)
    , m_nLineSize(1)
    , m_bLineShadow(FALSE)
    , m_crShadow(RGB(0xFF, 0xFF, 0xFF))
	, m_mode(HR_HORZ)
{
}

// Do nothing
void CDuiLine::OnPaint(CDCHandle dc)
{
	CPoint ptBegin,ptEnd;
	ptBegin=m_rcWindow.TopLeft();
	switch(m_mode)
	{
	case HR_HORZ:ptEnd.x=m_rcWindow.right,ptEnd.y=m_rcWindow.top;break;
	case HR_VERT:ptEnd.x=m_rcWindow.left,ptEnd.y=m_rcWindow.bottom;break;
	case HR_TILT:ptEnd=m_rcWindow.BottomRight();break;
	}
    CGdiAlpha::DrawLine(dc,ptBegin.x,ptBegin.y,ptEnd.x,ptEnd.y,m_style.m_crBg,m_nPenStyle, m_nLineSize);

    // 画阴影线
    if (m_bLineShadow)
    {
		switch(m_mode)
		{
		case HR_HORZ:
			ptBegin.y+=m_nLineSize,ptEnd.y+=m_nLineSize;
			break;
		case HR_VERT:
			ptBegin.x+=m_nLineSize,ptEnd.x+=m_nLineSize;
			break;
		case HR_TILT:
			ptBegin.Offset(m_nLineSize,m_nLineSize);
			ptEnd.Offset(m_nLineSize,m_nLineSize);
			break;
		}
		CGdiAlpha::DrawLine(dc,ptBegin.x,ptBegin.y,ptEnd.x,ptEnd.y,m_crShadow,m_nPenStyle, m_nLineSize);
    }
}

//////////////////////////////////////////////////////////////////////////
// Check Box
//
// Usage: <check state=1>This is a check-box</check>
//

CDuiCheckBox::CDuiCheckBox()
    : m_pSkin(GETSKIN("btncheckbox"))
    , m_pFocusSkin(GETSKIN("focuscheckbox"))
{
	m_bTabStop=TRUE;
}


CRect CDuiCheckBox::GetCheckRect()
{
	CRect rcClient;
	GetClient(rcClient);
	DUIASSERT(m_pSkin);
	CSize szCheck=m_pSkin->GetSkinSize();
	CRect rcCheckBox(rcClient.TopLeft(),szCheck);
	rcCheckBox.OffsetRect(0,(rcClient.Height()-szCheck.cy)/2);
	return rcCheckBox;
}

void CDuiCheckBox::GetTextRect( LPRECT pRect )
{
	GetClient(pRect);
	DUIASSERT(m_pSkin);
	CSize szCheck=m_pSkin->GetSkinSize();
	pRect->left+=szCheck.cx+CheckBoxSpacing;	
}

void CDuiCheckBox::OnPaint(CDCHandle dc)
{
	CRect rcCheckBox=GetCheckRect();
    m_pSkin->Draw(dc, rcCheckBox, _GetDrawState());
    __super::OnPaint(dc);
}

void CDuiCheckBox::DuiDrawFocus( HDC dc )
{
    if(m_pFocusSkin)
	{
		CRect rcCheckBox=GetCheckRect();
		m_pFocusSkin->Draw(dc,rcCheckBox,0);
	}else
	{
		__super::DuiDrawFocus(dc);
	}
}

CSize CDuiCheckBox::GetDesiredSize(LPRECT pRcContainer)
{
	DUIASSERT(m_pSkin);
	CSize szCheck=m_pSkin->GetSkinSize();
	CSize szRet=__super::GetDesiredSize(pRcContainer);
	szRet.cx+=szCheck.cx + CheckBoxSpacing;
	szRet.cy=max(szRet.cy, szCheck.cy);
	return szRet;
}


UINT CDuiCheckBox::_GetDrawState()
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

    --uState;	// 减1

    return uState;
}

void CDuiCheckBox::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetDuiFocus();
    __super::OnLButtonDown(nFlags,point);
}

void CDuiCheckBox::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (IsChecked())
        ModifyState(0, DuiWndState_Check,TRUE);
    else
        ModifyState(DuiWndState_Check, 0,TRUE);
    __super::OnLButtonUp(nFlags,point);
}

void CDuiCheckBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if(nChar==VK_SPACE)
    {
        if (IsChecked())
            ModifyState(0, DuiWndState_Check,TRUE);
        else
            ModifyState(DuiWndState_Check, 0,TRUE);

        if (GetCmdID())
        {
            DUINMCOMMAND nms;
			nms.hdr.hDuiWnd=m_hDuiWnd;
            nms.hdr.code = NM_COMMAND;
            nms.hdr.idFrom = GetCmdID();
			nms.hdr.pszNameFrom = GetName();
            nms.uItemData = GetUserData();
            DuiNotify((LPDUINMHDR)&nms);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Icon Control
// Use src attribute specify a resource id
//
// Usage: <icon src=xx oem="0" size="16"/>
//
CDuiIconWnd::CDuiIconWnd()
    : m_nSize(16)
{

}

BOOL CDuiIconWnd::Load(pugi::xml_node xmlNode)
{
    if (!CDuiWindow::Load(xmlNode))
        return FALSE;

    _ReloadIcon();

    return TRUE;
}

void CDuiIconWnd::OnPaint(CDCHandle dc)
{
    if (m_strCurIconName != m_strIconName)
        _ReloadIcon();

    CRect rcClient;
    GetClient(&rcClient);
    if(GetContainer()->IsTranslucent())
    {
        ALPHAINFO ai;
        CGdiAlpha::AlphaBackup(dc,&rcClient,ai);
        DrawIconEx(dc, rcClient.left,rcClient.top,m_theIcon,rcClient.Width(),rcClient.Height(),0,NULL,DI_NORMAL);
        CGdiAlpha::AlphaRestore(dc,ai);
    }
    else
    {
        DrawIconEx(dc, rcClient.left,rcClient.top,m_theIcon,rcClient.Width(),rcClient.Height(),0,NULL,DI_NORMAL);
    }
}


CSize CDuiIconWnd::GetDesiredSize(LPRECT pRcContainer)
{
	return CSize(m_nSize,m_nSize);
}

HICON CDuiIconWnd::AttachIcon(HICON hIcon)
{
    HICON tmp = m_theIcon;
    m_theIcon = hIcon;
    return tmp;
}

void CDuiIconWnd::LoadIconFile( LPCTSTR lpFIleName )
{
    if( m_theIcon ) DestroyIcon(m_theIcon);
    HICON hIcon = (HICON)LoadImage(NULL, lpFIleName, IMAGE_ICON, m_nSize, m_nSize, LR_LOADFROMFILE );
    m_theIcon = hIcon;
}

void CDuiIconWnd::_ReloadIcon()
{
    if (m_theIcon)		DestroyIcon(m_theIcon);
    m_theIcon=GETRESPROVIDER->LoadIcon(DUIRES_ICON_TYPE,m_strIconName,m_nSize,m_nSize);
    if(m_theIcon) m_strCurIconName = m_strIconName;
}

//////////////////////////////////////////////////////////////////////////
// Radio Box
//
// Usage: <radio state=1>This is a check-box</radio>
//

CDuiRadioBox::CDuiRadioBox()
    : m_pSkin(GETSKIN("btnRadio"))
    , m_pFocusSkin(GETSKIN("focusRadio"))
{
	m_bTabStop=TRUE;
}


CRect CDuiRadioBox::GetRadioRect()
{
	CRect rcClient;
	GetClient(rcClient);
	DUIASSERT(m_pSkin);
	CSize szRadioBox=m_pSkin->GetSkinSize();
	CRect rcRadioBox(rcClient.TopLeft(),szRadioBox);
	rcRadioBox.OffsetRect(0,(rcClient.Height()-szRadioBox.cy)/2);
	return rcRadioBox;
}


void CDuiRadioBox::GetTextRect( LPRECT pRect )
{
	GetClient(pRect);
	DUIASSERT(m_pSkin);
	CSize szRadioBox=m_pSkin->GetSkinSize();
	pRect->left+=szRadioBox.cx+RadioBoxSpacing;
}

void CDuiRadioBox::OnPaint(CDCHandle dc)
{
	DUIASSERT(m_pSkin);
    CRect rcRadioBox=GetRadioRect();
    m_pSkin->Draw(dc, rcRadioBox, _GetDrawState());
    __super::OnPaint(dc);
}

void CDuiRadioBox::DuiDrawFocus(HDC dc)
{
    if(m_pFocusSkin)
	{
		CRect rcCheckBox=GetRadioRect();
		m_pFocusSkin->Draw(dc,rcCheckBox,0);
	}else
	{
		__super::DuiDrawFocus(dc);
	}
}


CSize CDuiRadioBox::GetDesiredSize(LPRECT pRcContainer)
{
	CSize szRet=__super::GetDesiredSize(pRcContainer);
	CSize szRaio=m_pSkin->GetSkinSize();
	szRet.cx+=szRaio.cx + RadioBoxSpacing;
	szRet.cy=max(szRet.cy,szRaio.cy);
	return szRet;
}


UINT CDuiRadioBox::_GetDrawState()
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

BOOL CDuiRadioBox::NeedRedrawWhenStateChange()
{
    return TRUE;
}

void CDuiRadioBox::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetDuiFocus();
    __super::OnLButtonDown(nFlags,point);
}

void CDuiRadioBox::OnSetDuiFocus()
{
	CDuiWindow *pParent=GetParent();
	pParent->CheckRadioButton(this);
	NotifyInvalidate();
}

//////////////////////////////////////////////////////////////////////////
// CDuiToggle
CDuiToggle::CDuiToggle():m_bToggled(FALSE),m_pSkin(NULL)
{

}

void CDuiToggle::SetToggle(BOOL bToggle,BOOL bUpdate/*=TRUE*/)
{
    m_bToggled=bToggle;
    if(bUpdate) NotifyInvalidate();
}

BOOL CDuiToggle::GetToggle()
{
    return m_bToggled;
}

void CDuiToggle::OnPaint(CDCHandle dc)
{
    DUIASSERT(m_pSkin);
    DWORD nState=0;
    if(GetState()&DuiWndState_Hover) nState=2;
    else if(GetState()&DuiWndState_Check) nState=3;
    if(m_bToggled) nState+=3;
    m_pSkin->Draw(dc,m_rcWindow,nState);
}

void CDuiToggle::OnLButtonUp(UINT nFlags,CPoint pt)
{
    m_bToggled=!m_bToggled;
    __super::OnLButtonUp(nFlags,pt);
}

CSize CDuiToggle::GetDesiredSize(LPRECT pRcContainer)
{
	CSize sz;
	if(m_pSkin) sz=m_pSkin->GetSkinSize();
	return sz;
}

#define GROUP_HEADER		20
#define GROUP_ROUNDCORNOR	4

CDuiGroup::CDuiGroup():m_nRound(GROUP_ROUNDCORNOR),m_crLine1(RGB(0xF0,0xF0,0xF0)),m_crLine2(RGB(0xA0,0xA0,0xA0))
{

}
void CDuiGroup::OnPaint(CDCHandle dc)
{

    DuiDCPaint DuiDC;

    BeforePaint(dc, DuiDC);

    CSize szFnt;
    dc.GetTextExtent(m_strInnerText, m_strInnerText.GetLength(),&szFnt);

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

    CRgnHandle hRgn;
    int nSavedDC=dc.SaveDC();
    if(!m_strInnerText.IsEmpty())
    {
        CRect rcClip=rcText;
        rcClip.InflateRect(5,5,5,5);
        hRgn.CreateRectRgnIndirect(&rcClip);
        dc.SelectClipRgn(hRgn,RGN_DIFF);
        hRgn.DeleteObject();
    }

    {
        CRect rcGroupBox = m_rcWindow;

        if(!m_strInnerText.IsEmpty()) rcGroupBox.top+=szFnt.cy/2;
        rcGroupBox.DeflateRect(1,1,1,0);
        CPenHandle pen1,pen2;
        CPen oldPen;
        pen1.CreatePen(PS_SOLID,1,m_crLine1);
        pen2.CreatePen(PS_SOLID,1,m_crLine2);
        oldPen=dc.SelectPen(pen1);
        CBrush oldBr=dc.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
        CGdiAlpha::RoundRect(dc,rcGroupBox,CPoint(m_nRound,m_nRound));
        dc.SelectPen(pen2);
        rcGroupBox.InflateRect(1,1,1,-1);
        CGdiAlpha::RoundRect(dc,rcGroupBox,CPoint(m_nRound,m_nRound));
        dc.SelectBrush(oldBr);

        dc.SelectPen(oldPen);
    }

    dc.RestoreDC(nSavedDC);
    if(!m_strInnerText.IsEmpty())
    {
        CGdiAlpha::DrawText(dc,m_strInnerText, m_strInnerText.GetLength(), rcText, DT_SINGLELINE|DT_VCENTER);
    }

    AfterPaint(dc, DuiDC);
}

}//namespace SOUI