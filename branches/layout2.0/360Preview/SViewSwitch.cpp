#include "StdAfx.h"
#include "SViewSwitch.h"

namespace SOUI
{
	SViewSwitch::SViewSwitch()
		:m_dwPageCount(MAX_VEIWPAGE_COUNT)
		,m_bWantMove(FALSE)
		,m_iDownX(0)
		,m_iMoveWidth(0)
		,m_bTimerMove(0)
		,m_iSelected(0)
		,m_iTimesMove(0)
		,m_SelButton(0)
	{
	}

	SViewSwitch::~SViewSwitch()
	{
	}
	void SViewSwitch::OnPaint(IRenderTarget *pRT)
	{
		SPainter painter;
		BeforePaint(pRT,painter);

		for(int i = 0; i < (int)m_dwPageCount; i++)
		{
			ISkinObj *pSkinObj = m_pSkin[i];
			if (pSkinObj !=NULL)
			{
				SIZE SkinSize =pSkinObj->GetSkinSize();
				CRect rct;

				rct.left = (i*m_rcWindow.Width() - (m_iSelected * m_rcWindow.Width())+m_iMoveWidth) ;
				rct.top = m_rcWindow.top +2; 
				rct.bottom = SkinSize.cy;
				rct.right = rct.left + SkinSize.cx;

				pSkinObj->Draw(pRT,rct,0);

			}
		}

		if (m_pSkinLightLevel)
			m_pSkinLightLevel->Draw(pRT,m_rcWindow,0);

		AfterPaint(pRT,painter);
	}
	void  SViewSwitch::SWitch(int nSelect)
	{
		if (nSelect> m_dwPageCount)
			return;

		m_SelButton = nSelect;
		if(m_SelButton != -1)
		{
			if(m_bTimerMove)
			{
				return;
			}
			m_iMoveWidth = (m_SelButton-m_iSelected)*m_rcWindow.Width();
			m_iSelected = m_SelButton;

			m_iTimesMove = (m_iMoveWidth>0?m_iMoveWidth:-m_iMoveWidth)/10;
			if(m_iTimesMove < 20)m_iTimesMove = 20;
			SetTimer(TIMER_MOVE, 30);
			m_bTimerMove = TRUE;
			return;
		}

	}
	void SViewSwitch::OnLButtonDown(UINT nFlags, CPoint point)
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
	void SViewSwitch::OnMouseMove(UINT nFlags,CPoint pt)
	{
		__super::OnMouseMove(nFlags,pt);

		if(m_bWantMove)
		{
			m_iMoveWidth = pt.x - m_iDownX;
			if(m_iSelected == 0 && m_iMoveWidth > m_rcWindow.Width())
			{
				m_iMoveWidth = m_rcWindow.Width();
			}
			if(m_iSelected == (int)m_dwPageCount-1 && -m_iMoveWidth > m_rcWindow.Width())
			{
				m_iMoveWidth = -m_rcWindow.Width();
			}
			Invalidate();
			return;
		}

	}
	void SViewSwitch::OnLButtonUp(UINT nFlags, CPoint point)
	{
		if(m_bWantMove)
		{
			m_bWantMove = FALSE;
			ReleaseCapture();
			if(m_iMoveWidth > 0)
			{
				if(m_iSelected > 0 && m_iMoveWidth > m_rcWindow.Width()/4)
				{
					m_iMoveWidth -= m_rcWindow.Width();
					m_iSelected--;
				}
			}
			else
			{
				if(m_iSelected < (int)m_dwPageCount-1 && -m_iMoveWidth > m_rcWindow.Width()/4)
				{
					m_iMoveWidth += m_rcWindow.Width();
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

	void SViewSwitch::OnTimer(char nIDEvent)
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

}