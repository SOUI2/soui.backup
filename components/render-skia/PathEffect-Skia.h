#pragma once

#include <interface/SPathEffect-i.h>
#include <unknown/obj-ref-impl.hpp>
#include <effects/SkCornerPathEffect.h>
namespace SOUI
{
	class SPathEffect_Corner : public TObjRefImpl<ICornerPathEffect>
	{
	public:
		SPathEffect_Corner();
		~SPathEffect_Corner();

		virtual void Init(float radius);

		virtual void * GetRealPathEffect();
	private:
		SkCornerPathEffect * m_skCornerPathEffect;
	};
}

