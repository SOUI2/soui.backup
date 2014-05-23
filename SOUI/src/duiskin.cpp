//////////////////////////////////////////////////////////////////////////
//   File Name: DuiSkinPool
// Description: DuiWindow Skin Definition
//     Creator: ZhangXiaoxuan
//     Version: 2009.4.22 - 1.0 - Create
//                2011.6.18   1.1   huangjianxiong
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "duistd.h"
#include "duiskin.h"

#include "duiimgpool.h"
#include "gdialpha.h"
#include "DuiSystem.h"
#include "GradientFillHelper.h"

namespace SOUI
{


CDuiSkinImgList::CDuiSkinImgList()
:m_lSubImageWidth(0)
,m_nStates(1)
,m_bTile(FALSE)
,m_bVertical(FALSE)
,m_bCache(FALSE)
,m_memdc(NULL)
{

}

CDuiSkinImgList::~CDuiSkinImgList()
{
    if(m_memdc)
    {
        m_memdc->SetBitmapOwner(TRUE);
        delete m_memdc;
    }
}

void CDuiSkinImgList::PrepareCache( HDC hdc,CSize & sz )
{
    DUIASSERT(m_bCache);
    if(m_szTarget!=sz)
    {
        m_szTarget=sz;
        CRect rcImg(0,0,sz.cx*GetStates(),sz.cy);
        HBITMAP hBmp=CGdiAlpha::CreateBitmap32(hdc,rcImg.Width(),rcImg.Height());
        if(!m_memdc)
        {
            m_memdc=new CMemDC(hdc,hBmp);
        }else
        {
            m_memdc->SelectBitmap(hBmp);
            m_memdc->SetBitmapOwner(TRUE);
        }
        CRect rc(CPoint(),sz);
        for(int i=0;i<GetStates();i++)
        {
            _Draw(m_memdc->m_hDC,rc,i,0xFF);
            rc.OffsetRect(sz.cx,0);
        }
    }
}

void CDuiSkinImgList::Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha)
{
    if(m_bCache)
    {
        PrepareCache(dc,rcDraw.Size());
        BLENDFUNCTION bf= {AC_SRC_OVER,0,0xFF,AC_SRC_ALPHA};
        CRect rcClip;
        GetClipBox(dc,&rcClip);
        CRect rcInter;
        rcInter.IntersectRect(rcDraw,rcClip);
        AlphaBlend(dc,rcInter.left,rcInter.top,rcInter.Width(),rcInter.Height(),
            m_memdc->m_hDC,
            dwState*m_szTarget.cx+rcInter.left-rcDraw.left,
            rcInter.top-rcDraw.top,
            rcInter.Width(),
            rcInter.Height(),
            bf);
    }else
    {
        _Draw(dc,rcDraw,dwState,byAlpha);
    }
}

void CDuiSkinImgList::_Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha)
{
    if(m_pDuiImg)
    {
        SIZE sz=GetSkinSize();
        if(m_bVertical)
            ExtentBlt(m_pDuiImg,m_bTile,dc,rcDraw.left,rcDraw.top,rcDraw.Width(),rcDraw.Height(),0,dwState*sz.cy,sz.cx,sz.cy,byAlpha);
        else
            ExtentBlt(m_pDuiImg,m_bTile,dc,rcDraw.left,rcDraw.top,rcDraw.Width(),rcDraw.Height(),dwState*sz.cx,0,sz.cx,sz.cy,byAlpha);
    }
}

SIZE CDuiSkinImgList::GetSkinSize()
{
    SIZE ret = {0, 0};
    if(m_pDuiImg) m_pDuiImg->GetImageSize(ret);
    if(m_bVertical) ret.cy/=m_nStates;
    else ret.cx/=m_nStates;
    return ret;
}

BOOL CDuiSkinImgList::IgnoreState()
{
    return GetStates()==1;
}

int CDuiSkinImgList::GetStates()
{
    return m_nStates;
}

void CDuiSkinImgList::OnAttributeFinish(pugi::xml_node xmlNode )
{
    __super::OnAttributeFinish(xmlNode);

    DUIASSERT(m_pDuiImg);
    if(m_nStates==1 && 0 != m_lSubImageWidth)
    {
        //定义了子图宽度，没有定义子图数量
        m_nStates=m_pDuiImg->GetWidth()/m_lSubImageWidth;
        m_bVertical=FALSE;
    }
}

CDuiSkinImgFrame::CDuiSkinImgFrame()
    : m_crBg(CLR_INVALID)
    , m_uDrawPart(Frame_Part_All)
    , m_rcMargin(0,0,-1,-1)
{
}

void CDuiSkinImgFrame::_Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha)
{
    if(!m_pDuiImg) return;
    SIZE sz=GetSkinSize();
    CPoint pt;
    if(IsVertical())
        pt.y=sz.cy*dwState;
    else
        pt.x=sz.cx*dwState;
    CRect rcSour(pt,sz);
    FrameDraw(dc, m_pDuiImg , rcSour,rcDraw, m_rcMargin, m_crBg, m_uDrawPart,m_bTile,byAlpha);
}

void CDuiSkinImgFrame::OnAttributeFinish(pugi::xml_node xmlNode)
{
    __super::OnAttributeFinish(xmlNode);
    SIZE szSkin=GetSkinSize();
    if(m_rcMargin.right==-1)
    {
        if(szSkin.cx>m_rcMargin.left*2)
            m_rcMargin.right=m_rcMargin.left;
        else
            m_rcMargin.right=0;
    }
    if(m_rcMargin.bottom==-1)
    {
        if(szSkin.cy>m_rcMargin.top*2)
            m_rcMargin.bottom=m_rcMargin.top;
        else
            m_rcMargin.bottom=0;
    }
}

CDuiSkinButton::CDuiSkinButton()
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

#define MAKECOLORALPHA(cr,alpha) ((cr&0x00ffffff)|(alpha<<24))

void CDuiSkinButton::Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha)
{
    rcDraw.DeflateRect(1, 1);
    GradientFillRectV(
        dc, rcDraw,m_crUp[dwState],m_crDown[dwState],
        byAlpha
        );

    CPen penFrame;
    penFrame.CreatePen(
        PS_SOLID,
        1,
        m_crBorder
    );

    HPEN hpenOld = (HPEN)SelectObject(dc,penFrame);
    HBRUSH hbshOld = NULL, hbshNull = (HBRUSH)::GetStockObject(NULL_BRUSH);

    hbshOld = (HBRUSH)SelectObject(dc,hbshNull);

    rcDraw.DeflateRect(-1, -1);
    CGdiAlpha::RoundRect(dc,rcDraw,CPoint(2,2));

    SelectObject(dc,hbshOld);
    SelectObject(dc,hpenOld);
}

BOOL CDuiSkinButton::IgnoreState()
{
    return FALSE;
}

int CDuiSkinButton::GetStates()
{
    return 4;
}

void CDuiSkinButton::SetColors( COLORREF crUp[4],COLORREF crDown[4],COLORREF crBorder )
{
    memcpy(m_crUp,crUp,4*sizeof(COLORREF));
    memcpy(m_crDown,crDown,4*sizeof(COLORREF));
    m_crBorder=crBorder;
}

//////////////////////////////////////////////////////////////////////////
CDuiSkinGradation::CDuiSkinGradation()
    : m_uDirection(DIR_HORZ)
    , m_crFrom(CLR_INVALID)
    , m_crTo(CLR_INVALID)
{
}

void CDuiSkinGradation::Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha)
{
    if (DIR_HORZ == m_uDirection)
    {
        GradientFillRectH(dc, rcDraw, m_crFrom,m_crTo,byAlpha);
    }
    else
    {
        GradientFillRectV(dc, rcDraw, m_crFrom,m_crTo,byAlpha);
    }
}


CDuiScrollbarSkin::CDuiScrollbarSkin():m_nMargin(0),m_bHasGripper(FALSE),m_bHasInactive(FALSE)
{
    
}

CRect CDuiScrollbarSkin::GetPartRect(int nSbCode, int nState,BOOL bVertical)
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

void CDuiScrollbarSkin::Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha)
{
    if(!m_pDuiImg) return;
    int nSbCode=LOWORD(dwState);
    int nState=LOBYTE(HIWORD(dwState));
    BOOL bVertical=HIBYTE(HIWORD(dwState));
    CRect rcMargin(0,0,0,0);
    if(bVertical)
        rcMargin.top=m_nMargin,rcMargin.bottom=m_nMargin;
    else
        rcMargin.left=m_nMargin,rcMargin.right=m_nMargin;

    CRect rcSour=GetPartRect(nSbCode,nState,bVertical);
    FrameDraw(dc, m_pDuiImg , rcSour,rcDraw,rcMargin, CLR_INVALID, m_uDrawPart,m_bTile,byAlpha);

    if(nSbCode==SB_THUMBTRACK && m_bHasGripper)
    {
        rcSour=GetPartRect(SB_THUMBGRIPPER,0,bVertical);
        if (bVertical)
            rcDraw.top+=(rcDraw.Height()-rcSour.Height())/2,rcDraw.bottom=rcDraw.top+rcSour.Height();
        else
            rcDraw.left+=(rcDraw.Width()-rcSour.Width())/2,rcDraw.right=rcDraw.left+rcSour.Width();
        FrameDraw(dc, m_pDuiImg , rcSour,rcDraw,rcMargin, CLR_INVALID, m_uDrawPart,m_bTile,byAlpha);
    }
}

}//namespace SOUI