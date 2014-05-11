#include "duistd.h"
#include "DuiPools.h"
#include "mybuffer.h"

namespace SOUI
{
	CDuiPools::CDuiPools()
	{
	}

	CDuiPools::~CDuiPools(void)
	{
	}

	void CDuiPools::Init( LPCTSTR pszInitXml ,LPCTSTR pszResType)
	{
		pugi::xml_document xmlDoc;
		if(LOADXML(xmlDoc,pszInitXml,pszResType))
		{
			Init(xmlDoc.first_child());
		}
	}

	void CDuiPools::Init( pugi::xml_node xmlNode )
	{
		//load string table
		pugi::xml_node xmlStr=xmlNode.child("string");
		if(xmlStr)
		{
			m_strPool.Init(xmlStr);
		}
		//load style table
		pugi::xml_node xmlStyle=xmlNode.child("style");
		if(xmlStyle)
		{
			m_stylePool.Init(xmlStyle);
		}
		//load skin
		pugi::xml_node xmlSkin=xmlNode.child("skins");
		if(xmlSkin)
		{
			m_skinPool.Init(xmlSkin);
		}
		pugi::xml_node xmlObjAttr=xmlNode.child("objattr");
		//load objattr
		if(xmlObjAttr)
		{
			m_cssPool.Init(xmlObjAttr);
		}
	}

	void CDuiPools::Clear()
	{
		m_cssPool.RemoveAll();
		m_skinPool.RemoveAll();
		m_strPool.RemoveAll();
		m_stylePool.RemoveAll();
	}
}//end of namespace
