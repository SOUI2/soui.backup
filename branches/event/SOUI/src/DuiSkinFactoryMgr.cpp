#include "duistd.h"
#include "DuiSkinFactoryMgr.h"
#include "duiskin.h"

namespace SOUI
{

    void DuiSkinFactoryMgr::AddStandardSkin()
    {
        AddKeyObject(SSkinImgList::GetClassName(),new TplSkinFactory<SSkinImgList>);
        AddKeyObject(SSkinImgFrame::GetClassName(),new TplSkinFactory<SSkinImgFrame>);
        AddKeyObject(SSkinButton::GetClassName(),new TplSkinFactory<SSkinButton>);
        AddKeyObject(SSkinGradation::GetClassName(),new TplSkinFactory<SSkinGradation>);
        AddKeyObject(SSkinScrollbar::GetClassName(),new TplSkinFactory<SSkinScrollbar>);
        AddKeyObject(SSkinMenuBorder::GetClassName(),new TplSkinFactory<SSkinMenuBorder>);
    }

    void DuiSkinFactoryMgr::OnSkinRemoved( const CSkinFactoryPtr & obj )
    {
        delete obj;
    }

    ISkinObj * DuiSkinFactoryMgr::CreateSkinByName( LPCWSTR pszClassName )
    {
        if(!HasKey(pszClassName)) return NULL;
        return GetKeyObject(pszClassName)->NewSkin();
    }

}//end of namespace