#include "souistd.h"
#include "res.mgr/SFontInfo.h"
#include "core/swnd.h"
namespace SOUI
{
	SFontInfo::SFontInfo(void)
	{
	}

	SFontInfo::~SFontInfo(void)
	{
	}

	IFontPtr SFontInfo::GetFont()
	{
		return m_font;
	}

	void SFontInfo::SetFontDesc(const SStringW & strFont,int nScale)
	{
		m_strFontDesc = strFont;
		m_font = SFontPool::getSingleton().GetFont(m_strFontDesc,nScale);
	}

	void SFontInfo::SetScale(int nScale)
	{
		if(m_strFontDesc.IsEmpty()) return;
		m_font = SFontPool::getSingleton().GetFont(m_strFontDesc,nScale);
	}


}
