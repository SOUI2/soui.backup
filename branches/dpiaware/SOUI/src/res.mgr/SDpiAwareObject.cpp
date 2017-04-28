#include "souistd.h"
#include "res.mgr/SDpiAwareObject.h"

namespace SOUI
{
	SDpiAwareObject::SDpiAwareObject(void)
	{
	}

	SDpiAwareObject::~SDpiAwareObject(void)
	{
	}

	IObjRef * SDpiAwareObject::GetObjectPtr()
	{
		return m_object;
	}

	void SDpiAwareObject::SetObjectDesc(const SStringW & strDesc,int nScale)
	{
		m_strDesc = strDesc;
		m_object = OnGetObject(m_strDesc,nScale);
	}

	void SDpiAwareObject::SetScale(int nScale)
	{
		if(m_strDesc.IsEmpty()) return;
		m_object = OnGetObject(m_strDesc,nScale);
	}


}
