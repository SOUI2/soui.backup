#include "duistd.h"
#include "gdialpha.h"

namespace SOUI
{

#define  MAX_ALPHABUF	1<<16

BYTE CGdiAlpha::s_byAlphaBack[MAX_ALPHABUF];
BOOL CGdiAlpha::s_bAlphaEnable=TRUE;

LPBYTE CGdiAlpha::ALPHABACKUP(BITMAP *pBitmap,int x,int y,int cx,int cy)
{
    if(!s_bAlphaEnable) return NULL;
    LPBYTE lpAlpha=s_byAlphaBack;
    if(x+cx>=pBitmap->bmWidth) cx=pBitmap->bmWidth-x;
    if(y+cy>=pBitmap->bmHeight) cy=pBitmap->bmHeight-y;
    if(cx<0 || cy<0 ||pBitmap->bmBits==NULL) return NULL;

    if(cx*cy>MAX_ALPHABUF) lpAlpha=(LPBYTE)malloc(cx*cy);
    for(int yy=0; yy<cy; yy++)
    {
        LPBYTE lpBits=(LPBYTE)pBitmap->bmBits+(pBitmap->bmHeight-1-y-yy)*pBitmap->bmWidth*4+x*4;
        lpBits+=3;
        for(int xx=0; xx<cx; xx++)
        {
            lpAlpha[yy*cx+xx]=*lpBits;
            lpBits+=4;
        }
    }
    return lpAlpha;
}

//恢复位图的Alpha通道
void CGdiAlpha::ALPHARESTORE(BITMAP *pBitmap,int x,int y,int cx,int cy,LPBYTE lpAlpha)
{
    if(!s_bAlphaEnable) return;

    if(x+cx>=pBitmap->bmWidth) cx=pBitmap->bmWidth-x;
    if(y+cy>=pBitmap->bmHeight) cy=pBitmap->bmHeight-y;
    if(cx<0 || cy<0) return;
    for(int yy=0; yy<cy; yy++)
    {
        LPBYTE lpBits=(LPBYTE)pBitmap->bmBits+(pBitmap->bmHeight-1-y-yy)*pBitmap->bmWidth*4+x*4;
        lpBits+=3;
        for(int xx=0; xx<cx; xx++)
        {
            *lpBits=lpAlpha[yy*cx+xx];
            lpBits+=4;
        }
    }
    if(lpAlpha!=s_byAlphaBack)	free(lpAlpha);
}

void CGdiAlpha::_swap(int &a,int &b)
{
    int t=a;
    a=b;
    b=t;
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

BOOL CGdiAlpha::RoundRect(HDC hdc,LPCRECT pRc, POINT point)
{
    ALPHAINFO ai;
    AlphaBackup(hdc,pRc,ai);
    BOOL  bRet= ::RoundRect(hdc, pRc->left, pRc->top, pRc->right, pRc->bottom, point.x, point.y);
    AlphaRestore(hdc,ai);
    return bRet;
}

int CGdiAlpha::FillSolidRect(HDC hdc,LPCRECT pRect,COLORREF cr)
{
    HBRUSH hbr=CreateSolidBrush(cr);
    int nRet=FillRect(hdc,pRect,hbr);
    DeleteObject(hbr);
    return nRet;
}

int CGdiAlpha::FillRect(HDC hdc,LPCRECT pRc,HBRUSH hbrh)
{
    ALPHAINFO ai;
    AlphaBackup(hdc,pRc,ai);
    int nRet=::FillRect(hdc,pRc,hbrh);
    AlphaRestore(hdc,ai);
    return nRet;
}

BOOL CGdiAlpha::Rectangle(HDC hdc,LPRECT pRc,COLORREF crBorder,HBRUSH hbrh)
{
	ALPHAINFO ai;
	AlphaBackup(hdc,pRc,ai);
	HPEN hPen=CreatePen(PS_SOLID,1,crBorder);
	HGDIOBJ hOldPen=SelectObject(hdc,hPen);
	HGDIOBJ hOldBrh=SelectObject(hdc,hbrh);
	BOOL bRet=::Rectangle(hdc,pRc->left,pRc->top,pRc->right,pRc->bottom);
	SelectObject(hdc,hOldPen);
	SelectObject(hdc,hOldBrh);
	AlphaRestore(hdc,ai);
	return bRet;
}

SIZE CGdiAlpha::TextOut(HDC hdc,int x, int y, LPCTSTR pszText,int nCount/*=-1*/)
{
    SIZE szRet= {0};
    if(nCount==-1) nCount=(int)_tcslen(pszText);
    GetTextExtentPoint(hdc,pszText,nCount,&szRet);
    ALPHAINFO ai;
    RECT rcDest= {x,y,x+szRet.cx,y+szRet.cy};
    AlphaBackup(hdc,&rcDest,ai);
    ::TextOut(hdc,x,y,pszText,nCount);
    AlphaRestore(hdc,ai);
    return szRet;
}

int CGdiAlpha::DrawText(HDC hdc,LPCTSTR pszText,int nCount,LPRECT pRect,UINT uFormat)
{
    if(nCount==-1) nCount=(int)_tcslen(pszText);
    if(nCount==0)
    {
        if(uFormat & DT_CALCRECT)
        {
            pRect->left=pRect->right=pRect->top=pRect->bottom=0;
        }
        return 0;
    }
    ALPHAINFO ai;
    AlphaBackup(hdc,pRect,ai);
    int nRet=::DrawText(hdc,pszText,nCount,pRect,uFormat);
    AlphaRestore(hdc,ai);
    return nRet;
}

void CGdiAlpha::DrawLine(HDC hdc,int x1,int y1,int x2,int y2,COLORREF cr,UINT style, int iLineSize)
{
    RECT rcDest;
	int iLineRadius = iLineSize / 2;
	int iRemainder = iLineSize % 2;
    if(x1==x2)	//竖线
    {
		if(y1>y2) _swap(y1,y2);
        SetRect(&rcDest,x1-iLineRadius,y1,x1+iLineRadius+iRemainder,y2);
    }
    else if(y1==y2)//横线
    {
		if(x1>x2) _swap(x1,x2);
        SetRect(&rcDest,x1,y1-iLineRadius,x2,y1+iLineRadius+iRemainder);
    }
    else
    {
        if(x1>x2) _swap(x1,x2);
        if(y1>y2) _swap(y1,y2);
        SetRect(&rcDest,x1-iLineRadius,y1-iLineRadius,x2+iLineRadius+iRemainder,y2+iLineRadius+iRemainder);
    }

	ALPHAINFO ai;
	AlphaBackup(hdc,&rcDest,ai);

	LOGBRUSH lb;
	lb.lbStyle = BS_SOLID; 
	lb.lbColor = cr;
	lb.lbHatch = 0;
	HPEN hPen = ExtCreatePen(PS_GEOMETRIC | PS_ENDCAP_FLAT | style, iLineSize, &lb, 0, NULL);
	HPEN hOld=(HPEN)SelectObject(hdc,hPen);
	MoveToEx(hdc,x1,y1,NULL);
	LineTo(hdc,x2,y2);
	SelectObject(hdc,hOld);
	DeleteObject(hPen);

	AlphaRestore(hdc,ai);

}

HBITMAP CGdiAlpha::CreateBitmap32(HDC hdc,int nWid,int nHei,LPVOID * ppBits/*=NULL*/,BYTE byAlpha/*=0*/)
{
    HBITMAP hRet=NULL;
    LPVOID tmpBits=NULL;
    BITMAPINFOHEADER          bmih;
    ZeroMemory( &bmih, sizeof( BITMAPINFOHEADER ) );

    bmih.biSize                  = sizeof (BITMAPINFOHEADER) ;
    bmih.biWidth                = nWid ;
    bmih.biHeight                = nHei;
    bmih.biPlanes                = 1 ;
    bmih.biBitCount              = 32;
    bmih.biCompression          = BI_RGB ;
    bmih.biSizeImage            = 0 ;
    bmih.biXPelsPerMeter        = 0 ;
    bmih.biYPelsPerMeter        = 0 ;
    bmih.biClrUsed              = 0 ;
    bmih.biClrImportant          = 0 ;

    hRet = CreateDIBSection (hdc, (BITMAPINFO *)  &bmih, DIB_RGB_COLORS, (VOID**)&tmpBits, NULL, 0) ;
    LPBYTE pBit=(LPBYTE)tmpBits+3;
    for(int y=0; y<nHei; y++)
    {
        for(int x=0; x<nWid; x++)
        {
            *pBit=byAlpha;
            pBit+=4;
        }
    }
    if(ppBits) *ppBits=tmpBits;
    return hRet;
}

BOOL CGdiAlpha::Bitmap32PreMul( HDC hdc )
{
	HBITMAP hBmp=(HBITMAP)GetCurrentObject(hdc,OBJ_BITMAP);
	DUIASSERT(hBmp);
	BITMAP  bm;
	GetObject(hBmp,sizeof(BITMAP),&bm);

	if(bm.bmBitsPixel!=32) return FALSE;
	
	LPBYTE pbyLine=(LPBYTE)bm.bmBits;
	for(int i=0;i<bm.bmWidth;i++)
	{
		LPBYTE p=pbyLine;
		for(int j=0;j<bm.bmHeight;j++)
		{
			p[0]=(((WORD)p[1])*p[3])>>8;
			p[1]=(((WORD)p[1])*p[3])>>8;
			p[2]=(((WORD)p[1])*p[3])>>8;
			p+=4;
		}
		pbyLine=p;
	}
	return TRUE;
}
}//namespace SOUI