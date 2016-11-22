#include "souistd.h"
#include "helper\SPropBag.h"

namespace SOUI
{
	SPropBag::SPropBag(void)
	{
	}

	SPropBag::~SPropBag(void)
	{
	}

	void SPropBag::SetKeyValue(LPCTSTR pszKey, LPCTSTR pszValue)
	{
		m_propBag.SetAt(pszKey,pszValue);

	}

	LPCTSTR SPropBag::GetValue(LPCTSTR pszKey) const
	{
		SStringT key(pszKey);
		const SMap<SStringT,SStringT>::CPair *p = m_propBag.Lookup(key);
		if(p!=NULL)
			return p->m_value;
		else
			return NULL;

	}

	void SPropBag::RemoveKey(LPCTSTR pszKey)
	{
		m_propBag.RemoveKey(pszKey);
	}

	void SPropBag::RemoveAll()
	{
		m_propBag.RemoveAll();
	}

	SStringT SPropBag::ToXml() const
	{
		pugi::xml_document doc;
		pugi::xml_node root = doc.append_child(L"propbag");
		SPOSITION pos = m_propBag.GetStartPosition();
		while(pos)
		{
			const SMap<SStringT,SStringT>::CPair *p = m_propBag.GetNext(pos);
			root.append_attribute(S_CT2W(p->m_key)).set_value(S_CT2W(p->m_value));
		}
		pugi::xml_writer_buff writer;
		root.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
		return S_CW2T(SStringW(writer.buffer(),writer.size()));
	}

}
