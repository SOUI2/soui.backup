#include "stdafx.h"
#include "SImageEx.h"
#include "extend.skins\SSkinMutiFrameImg.h"

SImageEx::SImageEx()
{
}

SImageEx::~SImageEx()
{
}

HRESULT SImageEx::OnSrc(const SStringW& strValue, BOOL bLoading)
{
    SSkinPool *pBuiltinSkinPool = SSkinPoolMgr::getSingletonPtr()->GetBuiltinSkinPool();
    ISkinObj *pSkin=pBuiltinSkinPool->GetSkin(strValue); // 以文件名作为KEY
    if(pSkin)
    {
        if(!pSkin->IsClass(SSkinAni::GetClassName())) return FALSE;
        m_pBgSkin=static_cast<SSkinAni*>(pSkin);
    }else
    {
        SSkinAni *pSkin = (SSkinAni*)SApplication::getSingleton().CreateSkinByName(SSkinMutiFrameImg::GetClassName());
        if(!pSkin) return FALSE;
        if(0==pSkin->LoadFromFile(strValue))
        {
            pSkin->Release();
            return FALSE;
        }

        pSkin->SetAttribute(L"filterLevel", L"high");
        pBuiltinSkinPool->AddKeyObject(strValue, pSkin);//将创建的skin交给skinpool管理
        m_pBgSkin = pSkin;
    }
    return TRUE;
}
