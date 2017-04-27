#pragma once

#include "interface/sdpiaware-i.h"

namespace SOUI
{
	class SOUI_EXP SFontInfo
	{
	public:
		SFontInfo(void);
		~SFontInfo(void);


		IFontPtr GetFont();

		void SetFontDesc(const SStringW & strFont,int nScale);

		void SetScale(int nScale);
	protected:
		SStringW m_strFontDesc;
		CAutoRefPtr<IFont> m_font;
	};

}
