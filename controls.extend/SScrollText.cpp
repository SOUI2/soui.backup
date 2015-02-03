#include "stdafx.h"
#include "SScrollText.h"

namespace SOUI
{
    SScrollText::SScrollText(void):m_nSpeed(20),m_nOffset(0),m_nScrollWidth(0)
    {
    }

    SScrollText::~SScrollText(void)
    {
    }

    void SScrollText::OnPaint(IRenderTarget *pRT)
    {
        SPainter painter;
        BeforePaint(pRT,painter);
        CRect rcClient = GetClientRect();
        if(m_nScrollWidth==0)
        {
            pRT->DrawText(m_strText,m_strText.GetLength(),&rcClient,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
        }else
        {
            pRT->PushClipRect(&rcClient);
                        
            CRect rcText = rcClient;
            rcText.left -= m_nOffset;
            pRT->DrawText(m_strText,m_strText.GetLength(),&rcText,DT_SINGLELINE|DT_VCENTER);
            if(m_nScrollWidth - m_nOffset < rcClient.Width())
            {
                rcText.left += m_nScrollWidth;
//                 pRT->SetTextColor(RGBA(0,0,0,255));
                pRT->DrawText(m_strText,m_strText.GetLength(),&rcText,DT_SINGLELINE|DT_VCENTER);
            }
            
            pRT->PopClip();
        }
        AfterPaint(pRT,painter);
    }

    void SScrollText::OnTimer(char cTimer)
    {
        m_nOffset ++;
        if(m_nOffset>m_nScrollWidth)
            m_nOffset = 0;
        Invalidate();
    }

    void SScrollText::OnSize(UINT nType, CSize size)
    {
        __super::OnSize(nType,size);
        UpdateScrollInfo(size);
    }

    void SScrollText::OnShowWindow(BOOL bShow, UINT nStatus)
    {
        __super::OnShowWindow(bShow,nStatus);
        if(m_nScrollWidth>0)
        {
            if(bShow)
                SetTimer(1,m_nSpeed);
            else
                KillTimer(1);
        }
    }

    void SScrollText::SetWindowText(const SStringT & strText)
    {
        m_strText = strText;
        UpdateScrollInfo(GetClientRect().Size());//重新计算滚动长度
    }

    void SScrollText::UpdateScrollInfo(CSize size)
    {
        CAutoRefPtr<IRenderTarget> pRT;
        GETRENDERFACTORY->CreateRenderTarget(&pRT,0,0);
        BeforePaintEx(pRT);
        SIZE sz;
        pRT->MeasureText(m_strText,m_strText.GetLength(),&sz);

        if(sz.cx - size.cx>0)
        {
            if(IsVisible(TRUE)) SetTimer(1,m_nSpeed);
            m_nScrollWidth = sz.cx;
        }
        else
        {
            KillTimer(1);
            m_nOffset = 0;
            m_nScrollWidth = 0;
        }
    }

}
