#include "souistd.h"
#include "res.mgr/SDpiAwareFont.h"

namespace SOUI
{
	SDpiAwareFont::SDpiAwareFont(void)
	{
	}

	SDpiAwareFont::~SDpiAwareFont(void)
	{
	}

	IFont * SDpiAwareFont::GetFontPtr()
	{
		return (IFont*)(IObjRef*)m_object;
	}

	IObjRef * SDpiAwareFont::OnGetObject(const SStringW & strDesc,int nScale)
	{
		return SFontPool::getSingleton().GetFont(strDesc,nScale);
	}


}
