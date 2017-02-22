#pragma once

namespace SOUI
{

	class SOUI_EXP SLayoutSize
	{
	public:
		enum Unit{
			px=0,dp,sp
		};

		float fSize;
		Unit  unit;

		SStringW toString() const;

		static SLayoutSize fromString(const SStringW & strSize);
	};
}