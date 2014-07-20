#include "souistd.h"
#include "helper/DragWnd.h"

namespace SOUI
{

CDragWnd * CDragWnd:: s_pCurDragWnd=NULL;

CDragWnd::CDragWnd(void)
{
}

CDragWnd::~CDragWnd(void)
{
}

void CDragWnd::OnPaint( CDCHandle dc )
{
    CRect rc;
    GetClientRect(rc);
    dc.BitBlt(0,0,rc.Width(),rc.Height(),m_memdc,0,0,SRCCOPY);
}

BOOL CDragWnd::BeginDrag( HBITMAP hBmp,POINT ptHot ,COLORREF crKey, BYTE byAlpha,DWORD dwFlags)
{
    if(s_pCurDragWnd) return FALSE;
    s_pCurDragWnd=new CDragWnd;
    BITMAP bm;
    GetObject(hBmp,sizeof(bm),&bm);

    if(!s_pCurDragWnd->Create(NULL,WS_POPUP,WS_EX_TRANSPARENT|WS_EX_TOOLWINDOW|WS_EX_TOPMOST,0,0,bm.bmWidth,bm.bmHeight,0,NULL))
    {
        delete s_pCurDragWnd;
        s_pCurDragWnd=NULL;
        return FALSE;
    }
    
    s_pCurDragWnd->ModifyStyleEx(0,WS_EX_LAYERED);

    if(bm.bmBitsPixel==32)
    {
        CDCHandle dc=s_pCurDragWnd->GetDC();
        CMemDC memdc(dc,hBmp);
        BLENDFUNCTION bf={AC_SRC_OVER,0,byAlpha,AC_SRC_ALPHA};
        s_pCurDragWnd->UpdateLayeredWindow(dc,&CPoint(0,0),&CSize(bm.bmWidth,bm.bmHeight),memdc,&CPoint(0,0),crKey,&bf,LWA_ALPHA);
        s_pCurDragWnd->ReleaseDC(dc);
    }else
    {
        s_pCurDragWnd->SetLayeredWindowAttributes(crKey,byAlpha,dwFlags);
        CDCHandle dc=s_pCurDragWnd->GetDC();
        s_pCurDragWnd->m_memdc.CreateCompatibleDC(dc);
        s_pCurDragWnd->m_memdc.SelectBitmap(hBmp);
        s_pCurDragWnd->ReleaseDC(dc);
    }
    s_pCurDragWnd->m_ptHot=ptHot;
    return TRUE;
}


void CDragWnd::DragMove( POINT pt )
{
    ASSERT(s_pCurDragWnd);
    s_pCurDragWnd->SetWindowPos(HWND_TOPMOST,pt.x-s_pCurDragWnd->m_ptHot.x,pt.y-s_pCurDragWnd->m_ptHot.y,0,0,SWP_NOSIZE|SWP_NOSENDCHANGING|SWP_NOOWNERZORDER|SWP_SHOWWINDOW|SWP_NOACTIVATE);
}

void CDragWnd::EndDrag()
{
    ASSERT(s_pCurDragWnd);
    s_pCurDragWnd->DestroyWindow();
    delete s_pCurDragWnd;
    s_pCurDragWnd=NULL;
}

}//end of namespace
