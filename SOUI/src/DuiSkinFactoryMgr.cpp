#include "duistd.h"
#include "DuiSkinFactoryMgr.h"
#include "duiskin.h"

namespace SOUI
{

	void DuiSkinFactoryManager::AddStandardSkin()
	{
		AddKeyObject(CDuiSkinImgList::GetClassName(),new TplSkinFactory<CDuiSkinImgList>);
		AddKeyObject(CDuiSkinImgFrame::GetClassName(),new TplSkinFactory<CDuiSkinImgFrame>);
		AddKeyObject(CDuiSkinButton::GetClassName(),new TplSkinFactory<CDuiSkinButton>);
		AddKeyObject(CDuiSkinGradation::GetClassName(),new TplSkinFactory<CDuiSkinGradation>);
		AddKeyObject(CDuiScrollbarSkin::GetClassName(),new TplSkinFactory<CDuiScrollbarSkin>);
		AddKeyObject(CDuiSkinMenuBorder::GetClassName(),new TplSkinFactory<CDuiSkinMenuBorder>);
	}

	void DuiSkinFactoryManager::OnSkinRemoved( const CSkinFactoryPtr & obj )
	{
		delete obj;
	}

	CDuiSkinBase * DuiSkinFactoryManager::CreateSkinByName( LPCSTR pszClassName )
	{
		if(!HasKey(pszClassName)) return NULL;
		return GetKeyObject(pszClassName)->NewSkin();
	}

}//end of namespace