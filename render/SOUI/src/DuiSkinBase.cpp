#include "duistd.h"
#include "DuiSkinBase.h"
#include "gdialpha.h"

namespace SOUI
{

BOOL CDuiSkinBase::ExtentBlt(IBitmap *pImgDraw,BOOL bTile,HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,int nWidSrc,int nHeiSrc,BYTE byAlpha/*=0xFF*/)
{
    return FALSE;
//     if(bTile) return pImgDraw->TileBlt(hdc,x,y,nWid,nHei,xSrc,ySrc,nWidSrc,nHeiSrc,byAlpha);
//     else return pImgDraw->StretchBlt(hdc,x,y,nWid,nHei,xSrc,ySrc,nWidSrc,nHeiSrc,byAlpha);
}

void CDuiSkinBase::FrameDraw(HDC dc, IBitmap *pImgDraw, const CRect &rcSour,const  CRect &rcDraw, CRect rcMargin, COLORREF crBg, UINT uDrawPart ,BOOL bTile,BYTE byAlpha/*=0xFF*/)
{
    
}


}//namespace SOUI