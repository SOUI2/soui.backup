#pragma once

#include "DuiSkinPool.h"
#include "DuiCSS.h"
#include "duistringpool.h"
#include "DuiStylePool.h"

namespace SOUI
{
	class SOUI_EXP CDuiPools
	{
		friend class DuiPoolsStack;
	private:
		CDuiPools();
		~CDuiPools(void);

	public:
		DuiSkinPool * GetSkinPool(){return &m_skinPool;}
		DuiStringPool * GetStringPool(){return &m_strPool;}
		DuiCSS * GetDuiCSS(){return &m_cssPool;}
		DuiStylePool * GetStylePool(){return &m_stylePool;}

		void Init(LPCTSTR pszInitXml,LPCTSTR pszResType=DUIRES_XML_TYPE);

		void Init(pugi::xml_node xmlNode);

		void Clear();

	protected:
		DuiSkinPool	m_skinPool;
		DuiStringPool m_strPool;
		DuiCSS		m_cssPool;
		DuiStylePool m_stylePool;
	};

}//end of namespace
