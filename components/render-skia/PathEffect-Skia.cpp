#include "stdafx.h"
#include "PathEffect-Skia.h"

namespace SOUI
{

	SPathEffect_Corner::SPathEffect_Corner():m_skCornerPathEffect(NULL)
	{
	}

	SPathEffect_Corner::~SPathEffect_Corner()
	{
		if(m_skCornerPathEffect) m_skCornerPathEffect->unref();
	}

	void SPathEffect_Corner::Init(float radius)
	{
		if(m_skCornerPathEffect)
		{
			m_skCornerPathEffect->unref();
			m_skCornerPathEffect = NULL;
		}
		m_skCornerPathEffect = SkCornerPathEffect::Create(radius);
	}

	void * SPathEffect_Corner::GetRealPathEffect()
	{
		return m_skCornerPathEffect;
	}

}