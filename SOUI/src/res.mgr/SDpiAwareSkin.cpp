#include "souistd.h"
#include "res.mgr/SDpiAwareSkin.h"

namespace SOUI
{
	SDpiAwareSkin::SDpiAwareSkin(void)
	{
	}

	SDpiAwareSkin::~SDpiAwareSkin(void)
	{
	}

	ISkinObj * SDpiAwareSkin::GetSkinPtr()
	{
		return (ISkinObj*)(IObjRef*)m_object;
	}

	IObjRef * SDpiAwareSkin::OnGetObject(const SStringW & strDesc,int nScale)
	{
		return GETSKIN(strDesc);
	}
}
