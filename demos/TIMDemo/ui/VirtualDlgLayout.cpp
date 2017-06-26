#include "StdAfx.h"
#include "VirtualDlgLayout.h"

VirtualDlgLayout::VirtualDlgLayout(SWindow* pRoot, LPCTSTR pszResName)
	: m_pLayout(NULL)
{
	pugi::xml_document xmlDoc;
	SStringTList strLst;

	if(2 == ParseResID(pszResName, strLst))
	{
		LOADXML(xmlDoc, strLst[1], strLst[0]);
		pRoot->CreateChildren(xmlDoc.child(L"include"));
		m_pLayout = pRoot->GetWindow(GSW_LASTCHILD);
		// 默认一开始 不显示
		m_pLayout->SetVisible(FALSE, TRUE);
	}
}

VirtualDlgLayout::~VirtualDlgLayout(void)
{

}

void VirtualDlgLayout::ShowLayout(bool bShow)
{
	if(NULL == m_pLayout)
		return ;

	m_pLayout->SetVisible(bShow ? TRUE : FALSE, TRUE);
	if(bShow)
	{
		m_hFocus = m_pLayout->GetContainer()->GetFocus();
		m_pLayout->SetFocus();
	}
	else
	{
		m_pLayout->KillFocus();
		m_pLayout->GetContainer()->OnSetSwndFocus(m_hFocus);
	}
}



