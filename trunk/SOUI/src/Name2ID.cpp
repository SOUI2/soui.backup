#include "duistd.h"
#include "Name2ID.h"

namespace DuiEngine
{

	template<> DuiName2ID * Singleton<DuiName2ID>::ms_Singleton=0;

	DuiName2ID::DuiName2ID(void):m_pBuf(NULL),m_nCount(0)
{
}

DuiName2ID::~DuiName2ID(void)
{
	if(m_pBuf) delete []m_pBuf;
}

int DuiName2ID::Init( pugi::xml_node xmlNamedID )
{
	if(m_nCount)
	{
		DUIASSERT(m_pBuf);
		delete []m_pBuf;
		m_pBuf=NULL;
		m_nCount=0;
	}

	pugi::xml_node xmlNode=xmlNamedID;
	while(xmlNode)
	{
		DUIASSERT(strcmp(xmlNode.name(),"name2id")==0);
		xmlNode=xmlNode.next_sibling();
		m_nCount++;
	}
	m_pBuf=new CNamedID[m_nCount];

	xmlNode=xmlNamedID;
	int iItem=0;
	while(xmlNode)
	{
		m_pBuf[iItem++]=CNamedID(xmlNode.attribute("name").value(),xmlNode.attribute("id").as_int(0));
		xmlNode=xmlNode.next_sibling();
	}
	qsort(m_pBuf,m_nCount,sizeof(CNamedID),CNamedID::Compare);

	return m_nCount;
}

UINT DuiName2ID::Name2ID( LPCSTR pszName )
{
	if(m_nCount==0) return 0;
	CNamedID *pFind=(CNamedID*)bsearch(&CNamedID(pszName,0),m_pBuf,m_nCount,sizeof(CNamedID),CNamedID::Compare);
	if(pFind) return pFind->uID;
	else return 0;
}

}