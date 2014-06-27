#include "stdafx.h"
#include "gdialpha.h"
#include <malloc.h>

namespace SOUI
{

#define  MAX_ALPHABUF    1<<16

BYTE CGdiAlpha::s_byAlphaBack[MAX_ALPHABUF];

LPBYTE CGdiAlpha::ALPHABACKUP(BITMAP *pBitmap,int x,int y,int cx,int cy)
{
    if(pBitmap->bmBitsPixel != 32) return NULL;
    
    LPBYTE lpAlpha=s_byAlphaBack;
    if(x+cx>=pBitmap->bmWidth) cx=pBitmap->bmWidth-x;
    if(y+cy>=pBitmap->bmHeight) cy=pBitmap->bmHeight-y;
    if(cx<0 || cy<0 ||pBitmap->bmBits==NULL) return NULL;

    if(cx*cy>MAX_ALPHABUF) lpAlpha=(LPBYTE)malloc(cx*cy);
    LPBYTE lpBits=NULL;
    for(int iRow=0; iRow<cy; iRow++)
    {
//         if(pBitmap->bmHeight>0)//bottom-up
//             lpBits=(LPBYTE)pBitmap->bmBits+(pBitmap->bmHeight-1-y-iRow)*pBitmap->bmWidth*4+x*4;
//         else
            lpBits=(LPBYTE)pBitmap->bmBits+(y+iRow)*pBitmap->bmWidth*4+x*4;
        lpBits+=3;
        for(int iCol=0; iCol<cx; iCol++)
        {
            lpAlpha[iRow*cx+iCol]=*lpBits;
            lpBits+=4;
        }
    }
    return lpAlpha;
}

//恢复位图的Alpha通道
void CGdiAlpha::ALPHARESTORE(BITMAP *pBitmap,int x,int y,int cx,int cy,LPBYTE lpAlpha)
{
    if(pBitmap->bmBitsPixel != 32) return;
    
    if(x+cx>=pBitmap->bmWidth) cx=pBitmap->bmWidth-x;
    if(y+cy>=pBitmap->bmHeight) cy=pBitmap->bmHeight-y;
    if(cx<0 || cy<0) return;
    LPBYTE lpBits=NULL;
    for(int iRow=0; iRow<cy; iRow++)
    {
//         if(pBitmap->bmHeight>0)//bottom-up
//             lpBits=(LPBYTE)pBitmap->bmBits+(pBitmap->bmHeight-1-y-iRow)*pBitmap->bmWidth*4+x*4;
//         else
            lpBits=(LPBYTE)pBitmap->bmBits+(y+iRow)*pBitmap->bmWidth*4+x*4;
        lpBits+=3;
        for(int iCol=0; iCol<cx; iCol++)
        {
            *lpBits=lpAlpha[iRow*cx+iCol];
            lpBits+=4;
        }
    }
    if(lpAlpha!=s_byAlphaBack)    free(lpAlpha);
}

BOOL CGdiAlpha::AlphaBackup(HDC hdc,LPCRECT pRect,ALPHAINFO &alphaInfo)
{
    HBITMAP hBmp=(HBITMAP)GetCurrentObject(hdc,OBJ_BITMAP);
    DUIASSERT(hBmp);
    BITMAP  bm;
    GetObject(hBmp,sizeof(BITMAP),&bm);

    if(bm.bmBitsPixel!=32) return FALSE;
    alphaInfo.rc=*pRect;
    POINT pt;
    GetViewportOrgEx(hdc,&pt);
    RECT rcImg= {0,0,bm.bmWidth,bm.bmHeight};
    OffsetRect(&alphaInfo.rc,pt.x,pt.y);
    IntersectRect(&alphaInfo.rc,&alphaInfo.rc,&rcImg);
    alphaInfo.lpBuf=ALPHABACKUP(&bm,alphaInfo.rc.left,alphaInfo.rc.top,alphaInfo.rc.right - alphaInfo.rc.left, alphaInfo.rc.bottom - alphaInfo.rc.top);
    return TRUE;
}

void CGdiAlpha::AlphaRestore(HDC hdc,const ALPHAINFO &alphaInfo)
{
    if(!alphaInfo.lpBuf) return;
    HBITMAP hBmp=(HBITMAP)GetCurrentObject(hdc,OBJ_BITMAP);
    DUIASSERT(hBmp);
    BITMAP  bm;
    GetObject(hBmp,sizeof(BITMAP),&bm);
    DUIASSERT(bm.bmBitsPixel==32);
    ALPHARESTORE(&bm,alphaInfo.rc.left,alphaInfo.rc.top,alphaInfo.rc.right - alphaInfo.rc.left, alphaInfo.rc.bottom - alphaInfo.rc.top,alphaInfo.lpBuf);
}

}//namespace SOUI