#include "StdAfx.h"
#include "SImgCanvas.h"

#define INITGUID
#include <Guiddef.h>

DEFINE_GUID(ImageFormatPNG, 0xb96b3caf,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);//copy from gdi+

namespace SOUI
{
    SImgCanvas::SImgCanvas(void):m_bVert(FALSE)
    {
    }

    SImgCanvas::~SImgCanvas(void)
    {
    }

    void SImgCanvas::OnPaint(IRenderTarget *pRT)
    {
        if(m_lstImg.IsEmpty())
        {
            SWindow::OnPaint(pRT);
        }else
        {
            IBitmap * pBmp = m_lstImg.GetHead();
            
            CRect rcClient = GetClientRect();
            CSize szBmp(pBmp->Size());
            CRect rcAll = rcClient;
            CSize szAll = szBmp;

            CPoint ptOffset;
            if(m_bVert) 
            {
                szAll.cy *= m_lstImg.GetCount();
                ptOffset.y=szBmp.cy;
            }
            else 
            {
                szAll.cx *= m_lstImg.GetCount();
                ptOffset.x = szBmp.cx;
            }
            
            rcAll.DeflateRect((rcClient.Size()-szAll)/2);
            CRect rcBmp(rcAll.TopLeft(),szBmp);
            
            
            SPOSITION pos = m_lstImg.GetHeadPosition();
            while(pos)
            {
                pBmp = m_lstImg.GetNext(pos);
                pRT->DrawBitmap(rcBmp,pBmp,0,0);
                rcBmp.OffsetRect(ptOffset);
            }
            CAutoRefPtr<IPen> pen,oldpen;
            pRT->CreatePen(PS_DASHDOT,RGBA(0,0,0,128),1,&pen);
            pRT->SelectObject(pen,(IRenderObj**)&oldpen);
            pRT->DrawRectangle(rcAll);
            pRT->SelectObject(oldpen);
        }
    }

    BOOL SImgCanvas::AddFile(LPCWSTR pszFileName)
    {
        IBitmap *pImg=SResLoadFromFile::LoadImage(pszFileName);
        if(!pImg) return FALSE;
        m_lstImg.AddTail(pImg);
        Invalidate();
        return TRUE;
    }

    void SImgCanvas::Clear()
    {
        SPOSITION pos = m_lstImg.GetHeadPosition();
        while(pos)
        {
            IBitmap *pbmp = m_lstImg.GetNext(pos);
            pbmp->Release();
        }
        m_lstImg.RemoveAll();
        Invalidate();
    }

    BOOL SImgCanvas::Save2File(LPCWSTR pszFileName)
    {
        SStringW strDesc = GETRENDERFACTORY->GetImgDecoderFactory()->GetDescription();
        if(strDesc != L"gdi+" || m_lstImg.IsEmpty())
            return FALSE;
        
        IBitmap *pBmp = m_lstImg.GetHead();
        CSize szBmp = pBmp->Size();
        if(m_bVert) szBmp.cy *= m_lstImg.GetCount();
        else szBmp.cx *= m_lstImg.GetCount();
        
        CAutoRefPtr<IRenderTarget> pMemRT;
        GETRENDERFACTORY->CreateRenderTarget(&pMemRT,szBmp.cx,szBmp.cy);
        
        CRect rcDst(CPoint(),pBmp->Size());
        
        SPOSITION pos = m_lstImg.GetHeadPosition();
        while(pos)
        {
            pBmp = m_lstImg.GetNext(pos);
            pMemRT->DrawBitmap(rcDst,pBmp,0,0);
            if(m_bVert) rcDst.OffsetRect(0,rcDst.Height());
            else rcDst.OffsetRect(rcDst.Width(),0);
        }
        
        IBitmap * pCache = (IBitmap*)pMemRT->GetCurrentObject(OT_BITMAP);
        return pCache->Save(pszFileName,(const LPVOID)&ImageFormatPNG);
    }

    void SImgCanvas::SetVertical(BOOL bVert)
    {
        m_bVert = bVert;
        Invalidate();
    }

}
