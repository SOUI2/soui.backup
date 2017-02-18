#pragma once

#include "../interface/SPropBag-i.h"
#include <souicoll.h>
#include <string/tstring.h>
#include <unknown/obj-ref-impl.hpp>

namespace SOUI
{
	class SOUI_EXP SPropBag : public TObjRefImpl<IPropBag>
	{
	public:
		SPropBag(void);
		~SPropBag(void);

		virtual void SetKeyValue(LPCTSTR pszKey, LPCTSTR pszValue);
		virtual LPCTSTR GetValue(LPCTSTR pszKey) const;
		virtual void RemoveKey(LPCTSTR pszKey);
		virtual void RemoveAll();
		virtual SStringT ToXml() const;
	protected:
		SMap<SStringT,SStringT> m_propBag;
	};

}
