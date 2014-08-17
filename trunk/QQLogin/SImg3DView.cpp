//////////////////////////////////////////////////////////////////////////
//   File Name: Dui3DView.h
// Description: Dui3DView
//     Creator: ZhangZhiBin QQ->276883782
//     Version: 2014.02.06 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SImg3DView.h"
#include <helper/MemDC.h>
#pragma comment(lib,"Msimg32.lib")

namespace SOUI
{

    SImg3DView::SImg3DView(void):m_hBmpOrig(NULL),m_hBmpTrans(NULL)
    {
        memset(&m_3dparam,0,sizeof(m_3dparam));
    }

    SImg3DView::~SImg3DView(void)
    {
        if(m_hBmpTrans) DeleteObject(m_hBmpTrans);
    }

    void SImg3DView::OnSize( UINT nType, CSize size )
    {
        HDC hdc=GetDC(NULL);

   //     m_hBmpTrans=CGdiAlpha::CreateBitmap32(hdc,size.cx,size.cy,NULL,0);
        ReleaseDC(NULL,hdc);
        Update();
    }

    void SImg3DView::OnPaint( IRenderTarget *pRT )
    {
        if( (NULL == m_hBmpTrans) ||
            (NULL == m_hBmpOrig) )
        {
            return ;
        }

        HDC hdc= pRT->GetDC();
        CMemDC memdc(hdc,m_hBmpTrans);
        CRect rcClient;
        GetClientRect(&rcClient);
        BLENDFUNCTION bf={AC_SRC_OVER,0,255,AC_SRC_ALPHA};
        AlphaBlend(hdc,rcClient.left,rcClient.top,rcClient.Width(),rcClient.Height(),memdc,0,0,rcClient.Width(),rcClient.Height(),bf);
        pRT->ReleaseDC(hdc);
    }

    void SImg3DView::Update()
    {
        if( (NULL == m_hBmpTrans) ||
            (NULL == m_hBmpOrig) )
        {
            return ;
        }

        C3DTransform image3d;
        BITMAP bmSour,bmDest;
        GetObject(m_hBmpOrig,sizeof(BITMAP),&bmSour);
        GetObject(m_hBmpTrans,sizeof(BITMAP),&bmDest);
        image3d.SetImage((LPBYTE)bmSour.bmBits,(LPBYTE)bmDest.bmBits,bmSour.bmWidth,bmSour.bmHeight,bmSour.bmBitsPixel);
        image3d.Render(m_3dparam);
        Invalidate();
    }
}
