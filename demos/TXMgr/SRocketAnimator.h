#pragma once

namespace SOUI
{
	class SRocketAnimator : public SImageWnd, protected ITimelineHandler
	{
		SOUI_CLASS_NAME(SRocketAnimator,L"rocketAnimator")
	public:
		SRocketAnimator(void);
		~SRocketAnimator(void);

		void Fire();

		SOUI_ATTRS_BEGIN()
			ATTR_INT(L"speed",m_nSpeed,FALSE)
			ATTR_INT(L"steps",m_nSteps,FALSE)
			ATTR_INTERPOLATOR(L"interpolator",m_aniInterpolator,FALSE)
			ATTR_CHAIN_PTR(m_aniInterpolator,0)//chain attributes to interpolator
		SOUI_ATTRS_END()

	protected:
		virtual void OnNextFrame();

		void OnPaint(IRenderTarget *pRT);

		void OnDestroy();

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_DESTROY(OnDestroy)
		SOUI_MSG_MAP_END()
	protected:

		CAutoRefPtr<IInterpolator> m_aniInterpolator;
		int						   m_nSpeed;
		int						   m_iTimeStep;
		float					   m_iStep;
		int						   m_nSteps;
	};


}
