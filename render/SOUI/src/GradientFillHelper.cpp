#include "duistd.h"
#include "GradientFillHelper.h"
#include "memdc.h"
#include <Wingdi.h>
#pragma comment(lib,"msimg32.lib")

namespace SOUI
{

void GradientFillRect(HDC hdc, const CRect &rcFill, COLORREF cr1, COLORREF cr2,BOOL bVert,BYTE byAlpha)
{
    CRect rcTmp=rcFill;
    HDC hdc1=hdc;
    CMemDC *pMemDC=NULL;
    if(byAlpha!=0xFF)
    {
        rcTmp.MoveToXY(0,0);
        pMemDC= new CMemDC(hdc,rcTmp);
        hdc1=pMemDC->m_hDC;
    }

    TRIVERTEX        vert[2] ;
    vert [0] .x      = rcTmp.left;
    vert [0] .y      = rcTmp.top;
    vert [0] .Red    = GetRValue(cr1)<<8;
    vert [0] .Green  = GetGValue(cr1)<<8;
    vert [0] .Blue   = GetBValue(cr1)<<8;
    vert [0] .Alpha  = 0xff00;

    vert [1] .x      = rcTmp.right;
    vert [1] .y      = rcTmp.bottom; 
    vert [1] .Red    = GetRValue(cr2)<<8;
    vert [1] .Green  = GetGValue(cr2)<<8;
    vert [1] .Blue   = GetBValue(cr2)<<8;
    vert [1] .Alpha  = 0xff00;

    GRADIENT_RECT    gRect={0,1};
    GradientFill(hdc1,vert,2,&gRect,1,bVert?GRADIENT_FILL_RECT_V:GRADIENT_FILL_RECT_H);

    if(byAlpha!=0xFF)
    {
        BLENDFUNCTION bf={AC_SRC_OVER,0,byAlpha,0};
        AlphaBlend(hdc,rcFill.left,rcFill.top,rcFill.Width(),rcFill.Height(),
            hdc1,0,0,rcFill.Width(),rcFill.Height(),bf);
        delete pMemDC;
    }

}

void GradientFillRectH(HDC hdc, const CRect &rcFill, COLORREF crLeft, COLORREF crRight,BYTE byAlpha)
{
    GradientFillRect(hdc, rcFill, crLeft,crRight,FALSE,byAlpha);
}

void GradientFillRectV( HDC hdc,const CRect &rcFill, COLORREF crTop, COLORREF crBottom,BYTE byAlpha )
{
    GradientFillRect(hdc, rcFill, crTop,crBottom,TRUE,byAlpha);
}

}//namespace SOUI