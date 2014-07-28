//////////////////////////////////////////////////////////////////////////
//   File Name: SSkinPool
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "souistd.h"
#include "core/Sskin.h"

namespace SOUI
{

//////////////////////////////////////////////////////////////////////////
// SSkinImgList
SSkinImgList::SSkinImgList()
:m_nStates(1)
,m_bTile(FALSE)
,m_bVertical(FALSE)
,m_pImg(NULL)
{

}

SSkinImgList::~SSkinImgList()
{
    if(m_pImg) m_pImg->Release();
}

SIZE SSkinImgList::GetSkinSize()
{
    SIZE ret = {0, 0};
    if(m_pImg) ret=m_pImg->Size();
    if(m_bVertical) ret.cy/=m_nStates;
    else ret.cx/=m_nStates;
    return ret;
}

BOOL SSkinImgList::IgnoreState()
{
    return GetStates()==1;
}

int SSkinImgList::GetStates()
{
    return m_nStates;
}

void SSkinImgList::Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha)
{
    if(!m_pImg) return;
    SIZE sz=GetSkinSize();
    RECT rcSrc={0,0,sz.cx,sz.cy};
    if(m_bVertical) 
        OffsetRect(&rcSrc,0, dwState * sz.cy);
    else
        OffsetRect(&rcSrc, dwState * sz.cx, 0);
    pRT->DrawBitmapEx(rcDraw,m_pImg,&rcSrc,m_bTile?EM_TILE:EM_STRETCH,byAlpha);
}

HRESULT SSkinImgList::OnAttrImage( const SStringW & strValue,BOOL bLoading )
{
    IBitmap *pImg=NULL;
    SStringT strValueT=S_CW2T(strValue); 
    int nPos=strValueT.ReverseFind(_T(':'));
    if(nPos!=-1)
    {
        SStringT strName=strValueT.Right(strValue.GetLength()-nPos-1);
        pImg = LOADIMAGE(strValueT.Left(nPos),strName);
    }else 
    {
        pImg = LOADIMAGE(NULL,strValueT);
    }
    if(!pImg) return E_FAIL;
    if(m_pImg) m_pImg->Release();
    m_pImg=pImg;
    return S_FALSE;
}

//////////////////////////////////////////////////////////////////////////
//  SSkinImgFrame
SSkinImgFrame::SSkinImgFrame()
    : m_rcMargin(0,0,0,0)
{
}

void SSkinImgFrame::Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha)
{
    if(!m_pImg) return;
    SIZE sz=GetSkinSize();
    CPoint pt;
    if(IsVertical())
        pt.y=sz.cy*dwState;
    else
        pt.x=sz.cx*dwState;
    CRect rcSour(pt,sz);
    pRT->DrawBitmap9Patch(rcDraw,m_pImg,&rcSour,&m_rcMargin,m_bTile?EM_TILE:EM_STRETCH,byAlpha);
}

//////////////////////////////////////////////////////////////////////////
// SSkinButton
SSkinButton::SSkinButton()
    : m_crBorder(RGB(0x70, 0x70, 0x70))
{
    m_crUp[0]=(RGB(0xEE, 0xEE, 0xEE));
    m_crDown[0]=(RGB(0xD6, 0xD6, 0xD6));
    m_crUp[1]=(RGB(0xEE, 0xEE, 0xEE));
    m_crDown[1]=(RGB(0xE0, 0xE0, 0xE0));
    m_crUp[2]=(RGB(0xCE, 0xCE, 0xCE));
    m_crDown[2]=(RGB(0xC0, 0xC0, 0xC0));
    m_crUp[3]=(RGB(0x8E, 0x8E, 0x8E));
    m_crDown[3]=(RGB(0x80, 0x80, 0x80));
}

void SSkinButton::Draw(IRenderTarget *pRT, LPCRECT prcDraw, DWORD dwState,BYTE byAlpha)
{
    CRect rcDraw = *prcDraw;
    
    rcDraw.DeflateRect(1, 1);
    pRT->GradientFill(rcDraw,TRUE,m_crUp[dwState],m_crDown[dwState],byAlpha);

    CAutoRefPtr<IPen> pPen,pOldPen;
    pRT->CreatePen(PS_SOLID,m_crBorder,1,&pPen);
    pRT->SelectObject(pPen,(IRenderObj**)&pOldPen);
    pRT->DrawRoundRect(prcDraw,CPoint(2,2));
    pRT->SelectObject(pOldPen);
}

BOOL SSkinButton::IgnoreState()
{
    return FALSE;
}

int SSkinButton::GetStates()
{
    return 4;
}

void SSkinButton::SetColors( COLORREF crUp[4],COLORREF crDown[4],COLORREF crBorder )
{
    memcpy(m_crUp,crUp,4*sizeof(COLORREF));
    memcpy(m_crDown,crDown,4*sizeof(COLORREF));
    m_crBorder=crBorder;
}

//////////////////////////////////////////////////////////////////////////
// SSkinGradation
SSkinGradation::SSkinGradation()
    : m_bVert(TRUE)
    , m_crFrom(CR_INVALID)
    , m_crTo(CR_INVALID)
{
}

void SSkinGradation::Draw(IRenderTarget *pRT, LPCRECT prcDraw, DWORD dwState,BYTE byAlpha)
{
    pRT->GradientFill(prcDraw,m_bVert,m_crFrom,m_crTo,byAlpha);
}

//////////////////////////////////////////////////////////////////////////
// SScrollbarSkin
SSkinScrollbar::SSkinScrollbar():m_nMargin(0),m_bHasGripper(FALSE),m_bHasInactive(FALSE)
{
    
}

CRect SSkinScrollbar::GetPartRect(int nSbCode, int nState,BOOL bVertical)
{
    CSize sz=GetSkinSize();
    CSize szFrame(sz.cx/9,sz.cx/9);
    if(nSbCode==SB_CORNOR)
    {
        return CRect(CPoint(szFrame.cx*8,0),szFrame);
    }else if(nSbCode==SB_THUMBGRIPPER)
    {
        return CRect(CPoint(szFrame.cx*8,(1+(bVertical?0:1))*szFrame.cy),szFrame);
    }else
    {
        if(nState==SBST_INACTIVE)
        {
            if(nSbCode==SB_THUMBTRACK || !m_bHasInactive)
            {
                nState=SBST_NORMAL;
            }
        }
        CRect rcRet;
        int iPart=-1;
        switch(nSbCode)
        {
        case SB_LINEUP:
            iPart=0;
            break;
        case SB_LINEDOWN:
            iPart=1;
            break;
        case SB_THUMBTRACK:
            iPart=2;
            break;
        case SB_PAGEUP:
        case SB_PAGEDOWN:
            iPart=3;
            break;
        }
        if(!bVertical) iPart+=4;
        
        return CRect(CPoint(szFrame.cx*iPart,szFrame.cy*nState),szFrame);
    }
}

void SSkinScrollbar::Draw(IRenderTarget *pRT, LPCRECT prcDraw, DWORD dwState,BYTE byAlpha)
{
    if(!m_pImg) return;
    int nSbCode=LOWORD(dwState);
    int nState=LOBYTE(HIWORD(dwState));
    BOOL bVertical=HIBYTE(HIWORD(dwState));
    CRect rcMargin(0,0,0,0);
    if(bVertical)
        rcMargin.top=m_nMargin,rcMargin.bottom=m_nMargin;
    else
        rcMargin.left=m_nMargin,rcMargin.right=m_nMargin;

    CRect rcSour=GetPartRect(nSbCode,nState,bVertical);
    
    pRT->DrawBitmap9Patch(prcDraw,m_pImg,&rcSour,&rcMargin,m_bTile?EM_TILE:EM_STRETCH,byAlpha);
    
    if(nSbCode==SB_THUMBTRACK && m_bHasGripper)
    {
        rcSour=GetPartRect(SB_THUMBGRIPPER,0,bVertical);
        CRect rcDraw=*prcDraw;
        
        if (bVertical)
            rcDraw.top+=(rcDraw.Height()-rcSour.Height())/2,rcDraw.bottom=rcDraw.top+rcSour.Height();
        else
            rcDraw.left+=(rcDraw.Width()-rcSour.Width())/2,rcDraw.right=rcDraw.left+rcSour.Width();
        pRT->DrawBitmap9Patch(&rcDraw,m_pImg,&rcSour,&rcMargin,m_bTile?EM_TILE:EM_STRETCH,byAlpha);
    }
}

}//namespace SOUI