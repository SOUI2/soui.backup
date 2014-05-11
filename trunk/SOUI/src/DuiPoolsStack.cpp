#include "duistd.h"
#include "DuiPoolsStack.h"

namespace SOUI
{
	DuiPoolsStack::DuiPoolsStack(void)
	{
		Push(new CDuiPools);
	}

	DuiPoolsStack::~DuiPoolsStack(void)
	{
		delete Pop();
	}

	CDuiPools * DuiPoolsStack::GetCurResMgr()
	{
		if(m_lstResMgr.IsEmpty()) return NULL;
		return m_lstResMgr.GetTail();
	}

	void DuiPoolsStack::Push( CDuiPools * pResMgr )
	{
		m_lstResMgr.AddTail(pResMgr);
	}

	CDuiPools * DuiPoolsStack::Pop()
	{
		if(m_lstResMgr.IsEmpty()) return NULL;
		return m_lstResMgr.RemoveTail();
	}

	CDuiPools * DuiPoolsStack::CreateResMgr()
	{
		return new CDuiPools;
	}

	void DuiPoolsStack::DestroyResMgr(CDuiPools *pResMgr)
	{
		delete pResMgr;
	}

	CDuiSkinBase * DuiPoolsStack::GetSkin( LPCSTR pszName )
	{
		if(m_lstResMgr.IsEmpty()) return NULL;
		return GetCurResMgr()->GetSkinPool()->GetSkin(pszName);
	}

	BOOL DuiPoolsStack::GetStyle( LPCSTR pszName,DuiStyle &style )
	{
		if(m_lstResMgr.IsEmpty()) return FALSE;
		return GetCurResMgr()->GetStylePool()->GetStyle(pszName,style);
	}

	pugi::xml_node DuiPoolsStack::GetObjDefAttr( LPCSTR pszName )
	{
		if(m_lstResMgr.IsEmpty()) return pugi::xml_node();
		return GetCurResMgr()->GetDuiCSS()->GetDefAttribute(pszName);
	}
	
	//字符串只能当前的资源管理器中获取
	void DuiPoolsStack::BuildString( CDuiStringT & str )
	{
		if(!m_lstResMgr.IsEmpty())
			GetCurResMgr()->GetStringPool()->BuildString(str);
	}
}//end of namespace
