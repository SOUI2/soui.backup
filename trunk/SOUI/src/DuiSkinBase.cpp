#include "duistd.h"
#include "DuiSkinBase.h"
#include "gdialpha.h"

namespace SOUI
{

BOOL CDuiSkinBase::ExtentBlt(IDuiImage *pImgDraw,BOOL bTile,HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,int nWidSrc,int nHeiSrc,BYTE byAlpha/*=0xFF*/)
{
    if(bTile) return pImgDraw->TileBlt(hdc,x,y,nWid,nHei,xSrc,ySrc,nWidSrc,nHeiSrc,byAlpha);
    else return pImgDraw->StretchBlt(hdc,x,y,nWid,nHei,xSrc,ySrc,nWidSrc,nHeiSrc,byAlpha);
}

void CDuiSkinBase::FrameDraw(HDC dc, IDuiImage *pImgDraw, const CRect &rcSour,const  CRect &rcDraw, CRect rcMargin, COLORREF crBg, UINT uDrawPart ,BOOL bTile,BYTE byAlpha/*=0xFF*/)
{
    CRect rcCenter = rcDraw;

    if(rcDraw.IsRectEmpty() || rcSour.IsRectEmpty()) return;
    //检查边缘范围是否大于填充区域，如果超出则自动缩小连续范围
    int xOverflow=rcMargin.left+rcMargin.right-rcDraw.Width();
    if(xOverflow>0)
    {
        rcMargin.left-=xOverflow/2;
        rcMargin.right-=xOverflow/2;
        if(rcMargin.left<0) {rcMargin.right+=-rcMargin.left;rcMargin.left=0;}
        else if(rcMargin.right<0) {rcMargin.left+=-rcMargin.right;rcMargin.right=0;}
    }
    int yOverflow=rcMargin.top+rcMargin.bottom-rcDraw.Height();
    if(yOverflow>0)
    {
        rcMargin.top-=xOverflow/2;
        rcMargin.bottom-=xOverflow/2;
        if(rcMargin.top<0) {rcMargin.bottom+=-rcMargin.top;rcMargin.top=0;}
        else if(rcMargin.bottom<0) {rcMargin.top+=-rcMargin.bottom;rcMargin.bottom=0;}
    }

    DUIASSERT(dc);
    DUIASSERT(!pImgDraw->IsEmpty());

    rcCenter.DeflateRect(rcMargin.left, rcMargin.top , rcMargin.right ,rcMargin.bottom);

    if (Frame_Part_TopLeft & uDrawPart)
    {
        pImgDraw->BitBlt(
            dc,
            rcDraw.left, rcDraw.top,
            rcMargin.left, rcMargin.top,
            rcSour.left, rcSour.top,
            byAlpha
        );
    }
    if (Frame_Part_TopRight & uDrawPart)
    {
        pImgDraw->BitBlt(
            dc,
            rcCenter.right, rcDraw.top,
            rcMargin.right, rcMargin.top,
            rcSour.right - rcMargin.right, rcSour.top,
            byAlpha
        );
    }
    if (Frame_Part_BottomLeft & uDrawPart)
    {
        pImgDraw->BitBlt(
            dc,
            rcDraw.left, rcCenter.bottom,
            rcMargin.left, rcMargin.bottom,
            rcSour.left , rcSour.bottom-rcMargin.bottom,
            byAlpha
        );
    }
    if (Frame_Part_BottomRight & uDrawPart)
    {
        pImgDraw->BitBlt(
            dc,
            rcCenter.right, rcCenter.bottom,
            rcMargin.right , rcMargin.bottom,
            rcSour.right - rcMargin.right, rcSour.bottom - rcMargin.bottom,
            byAlpha
        );
    }
    if (Frame_Part_TopCenter & uDrawPart)
    {
        ExtentBlt(pImgDraw,bTile,
                  dc,
                  rcCenter.left, rcDraw.top,
                  rcCenter.Width(), rcMargin.top,
                  rcSour.left + rcMargin.left, rcSour.top,
                  rcSour.Width() - rcMargin.left-rcMargin.right, rcMargin.top,
                  byAlpha
                 );
    }
    if (Frame_Part_MidLeft & uDrawPart)
    {
        ExtentBlt(pImgDraw,bTile,
                  dc,
                  rcDraw.left, rcCenter.top,
                  rcMargin.left, rcCenter.Height(),
                  rcSour.left, rcSour.top + rcMargin.top,
                  rcMargin.left, rcSour.Height() - rcMargin.top-rcMargin.bottom,
                  byAlpha
                 );
    }
    if (Frame_Part_BottomCenter & uDrawPart)
    {
        ExtentBlt(pImgDraw,bTile,
                  dc,
                  rcCenter.left, rcCenter.bottom,
                  rcCenter.Width(), rcMargin.bottom,
                  rcSour.left+rcMargin.left, rcSour.bottom-rcMargin.bottom,
                  rcSour.Width() - rcMargin.left -rcMargin.right, rcMargin.bottom,
                  byAlpha
                 );
    }
    if (Frame_Part_MidRight & uDrawPart)
    {
        ExtentBlt(pImgDraw,bTile,
                  dc,
                  rcCenter.right, rcCenter.top,
                  rcMargin.right, rcCenter.Height(),
                  rcSour.right - rcMargin.right, rcSour.top + rcMargin.top,
                  rcMargin.right, rcSour.Height()-rcMargin.top-rcMargin.bottom,
                  byAlpha
                 );
    }

    if(Frame_Part_MidCenter & uDrawPart && !rcCenter.IsRectEmpty()) 
    {
        CRect rcSourMD=rcSour;
        rcSourMD.DeflateRect(rcMargin.left,rcMargin.top,rcMargin.right,rcMargin.bottom);
        if (CLR_INVALID != crBg)
        {//采用纯色填充中间部分
            CGdiAlpha::FillSolidRect(dc,rcCenter, crBg);
        }else
        {
            ExtentBlt(pImgDraw,bTile,
                dc,
                rcCenter.left, rcCenter.top,
                rcCenter.Width(), rcCenter.Height(),
                rcSourMD.left, rcSourMD.top,
                rcSourMD.Width(), rcSourMD.Height(),
                byAlpha
                );
        }
    }
}


}//namespace SOUI