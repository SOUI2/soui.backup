#include "stdafx.h"
#include "SRocketAnimator.h"
#include <animator/SInterpolatorImpl.h>
namespace SOUI
{
	SRocketAnimator::SRocketAnimator(void):m_iStep(0),m_nSteps(20),m_nSpeed(30)
	{
		m_aniInterpolator.Attach(new SLinearInterpolator());
		m_bVisible = FALSE;
	}

	SRocketAnimator::~SRocketAnimator(void)
	{
	}

	void SRocketAnimator::OnNextFrame()
	{
		int nSpeed = m_nSpeed/10;

		m_iTimeStep++;

		if((m_iTimeStep % nSpeed)==0)
		{
			m_iTimeStep=0;
			m_iStep ++;
			Invalidate();
			if(m_iStep>=m_nSteps)
			{
				GetContainer()->UnregisterTimelineHandler(this);
				SetVisible(FALSE,TRUE);
			}
		}
	}

	void SRocketAnimator::OnPaint(IRenderTarget *pRT)
	{
		float prog = ((float)m_iStep)/m_nSteps;
		prog = m_aniInterpolator->getInterpolation(prog);
		CRect rcClient = GetClientRect();
		CSize szSkin =  GetSkin()->GetSkinSize();
		int length = rcClient.Height()-szSkin.cy;
		int offset = (int)(length * prog);
		CRect rcImg = CRect(CPoint(rcClient.left+(rcClient.Width()-szSkin.cx)/2,rcClient.bottom-szSkin.cy-offset),szSkin);
		GetSkin()->Draw(pRT,rcImg,0);
	}

	void SRocketAnimator::Fire()
	{
		SetVisible(TRUE,TRUE);
		GetContainer()->RegisterTimelineHandler(this);
		m_iStep = 0;
		m_iTimeStep = 0;
		Invalidate();
	}

	void SRocketAnimator::OnDestroy()
	{
		GetContainer()->UnregisterTimelineHandler(this);
		__super::OnDestroy();
	}

}
