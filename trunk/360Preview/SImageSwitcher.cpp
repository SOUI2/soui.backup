#include "StdAfx.h"
#include "SImageSwitcher.h"
#include <helper/SplitString.h>

#define TIMER_MOVE		1

namespace SOUI
{
	SImageSwitcher::SImageSwitcher()
		:m_bWantMove(FALSE)
		,m_iDownX(0)
		,m_iMoveWidth(0)
		,m_bTimerMove(0)
		,m_iSelected(0)
		,m_iTimesMove(0)
	{
	}

	SImageSwitcher::~SImageSwitcher()
	{
	    RemoveAll();
	}
	
	void SImageSwitcher::OnPaint(IRenderTarget *pRT)
	{
        CRect rcWnd = GetClientRect();
		if (1 == m_lstImages.GetCount())
		{
			IBitmap *pBmp = m_lstImages[0];
			SIZE szImg = pBmp->Size();
			pRT->DrawBitmapEx(rcWnd,pBmp,CRect(CPoint(),szImg),EM_STRETCH);
		}
		else
		{
			for(UINT i = 0; i < m_lstImages.GetCount(); i++)
			{
				IBitmap *pBmp = m_lstImages[i];
				SIZE szImg = pBmp->Size();
				CRect rct;

				rct.left = rcWnd.left + (i*rcWnd.Width() - (m_iSelected * rcWnd.Width())+m_iMoveWidth) ;
				rct.right = rct.left + rcWnd.Width();
				rct.top = rcWnd.top; 
				rct.bottom = rct.top + rcWnd.Height();

				pRT->DrawBitmapEx(rct,pBmp,CRect(CPoint(),szImg),EM_STRETCH);
			}
		}
	}
	
	void  SImageSwitcher::Switch(int iSelect)
	{
		if (iSelect > (int)m_lstImages.GetCount() || iSelect < 0)
			return;

        CRect rcWnd = GetClientRect();
        if(m_bTimerMove)
        {
            return;
        }
        m_iMoveWidth = (iSelect-m_iSelected)*rcWnd.Width();
        m_iSelected = iSelect;

        m_iTimesMove = (m_iMoveWidth>0?m_iMoveWidth:-m_iMoveWidth)/10;
        if(m_iTimesMove < 20)m_iTimesMove = 20;
        SetTimer(TIMER_MOVE, 30);
        m_bTimerMove = TRUE;
    }

	void SImageSwitcher::OnLButtonDown(UINT nFlags, CPoint point)
	{
		if(m_bWantMove)
			return;

		if(m_bTimerMove)
		{
			m_bTimerMove = FALSE;
			KillTimer(TIMER_MOVE);
			m_bWantMove = TRUE;
			m_iDownX = point.x - m_iMoveWidth;
			SetCapture();

		}
		else
		{
			m_bWantMove = TRUE;
			m_iDownX = point.x;
			SetCapture();
		}
		__super::OnLButtonDown(nFlags,point);
	}
	void SImageSwitcher::OnMouseMove(UINT nFlags,CPoint pt)
	{
		__super::OnMouseMove(nFlags,pt);

        CRect rcWnd = GetWindowRect();

		if(m_bWantMove)
		{
			m_iMoveWidth = pt.x - m_iDownX;
			if(m_iSelected == 0 && m_iMoveWidth > rcWnd.Width())
			{
				m_iMoveWidth = rcWnd.Width();
			}
			if(m_iSelected == (int)m_lstImages.GetCount()-1 && -m_iMoveWidth > GetWindowRect().Width())
			{
				m_iMoveWidth = -rcWnd.Width();
			}
			Invalidate();
			return;
		}

	}
	void SImageSwitcher::OnLButtonUp(UINT nFlags, CPoint point)
	{
        CRect rcWnd = GetClientRect();

		if(m_bWantMove)
		{
			m_bWantMove = FALSE;
			ReleaseCapture();
			if(m_iMoveWidth > 0)
			{
				if(m_iSelected > 0 && m_iMoveWidth > rcWnd.Width()/4)
				{
					m_iMoveWidth -= rcWnd.Width();
					m_iSelected--;
				}
			}
			else
			{
				if(m_iSelected < (int)m_lstImages.GetCount()-1 && -m_iMoveWidth > rcWnd.Width()/4)
				{
					m_iMoveWidth += rcWnd.Width();
					m_iSelected++;
				}
			}
			m_iTimesMove = (m_iMoveWidth>0?m_iMoveWidth:-m_iMoveWidth)/10;
			if(m_iTimesMove < 20)m_iTimesMove = 20;
			SetTimer(TIMER_MOVE, 30);
			m_bWantMove = FALSE;
			m_bTimerMove = TRUE;
			return;
		}
	}

	void SImageSwitcher::OnTimer(char nIDEvent)
	{
		if(m_iMoveWidth > 0)
		{
			if(m_iMoveWidth - m_iTimesMove <= 0)
			{
				m_iMoveWidth = 0;
				Invalidate();
				KillTimer(TIMER_MOVE);
				m_bTimerMove = FALSE;
			}
			else
			{
				m_iMoveWidth-= m_iTimesMove;
				Invalidate();
			}
		}
		else
		{
			if(m_iMoveWidth + m_iTimesMove >= 0)
			{
				m_iMoveWidth = 0;
				Invalidate();
				KillTimer(TIMER_MOVE);
				m_bTimerMove = FALSE;
			}
			else
			{
				m_iMoveWidth+= m_iTimesMove;
				Invalidate();
			}
		}
	}

    void SImageSwitcher::InsertImage(int iTo, IBitmap * pImage)
    {
        if(iTo<0) iTo = m_lstImages.GetCount();
        m_lstImages.InsertAt(iTo,pImage);
        pImage->AddRef();
        Invalidate();
    }

    void SImageSwitcher::RemoveAll()
    {
        for(UINT i=0;i<m_lstImages.GetCount();i++)
        {
            m_lstImages[i]->Release();
        }
        m_lstImages.RemoveAll();
    }

    HRESULT SImageSwitcher::OnAttrImages(const SStringW strValue,BOOL bLoading)
    {
        SStringWList imgLstSrc ;
        SplitString(strValue,L'|',imgLstSrc);
        for(UINT i=0;i<imgLstSrc.GetCount();i++)
        {
            IBitmap * pImg = LOADIMAGE2(imgLstSrc[i]);
            if(pImg) 
            {
                m_lstImages.Add(pImg);
            }
        }
        return bLoading?S_OK:S_FALSE;
    }

}