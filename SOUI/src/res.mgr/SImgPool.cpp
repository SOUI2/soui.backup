//////////////////////////////////////////////////////////////////////////
//  Class Name: SImgPool
// Description: Image Pool
//     Creator: Huang jianxiong
//     Version: 2012.8.30 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "souistd.h"
#include "core/sobject.h"
#include "res.mgr/simgpool.h"
#include "res.mgr/sresprovider.h"
#include "SApp.h"

namespace SOUI
{

template<> SImgPool * SSingleton<SImgPool>::ms_Singleton =0;

SImgPool::SImgPool()
{
    m_pFunOnKeyRemoved=OnImageRemoved;
}

SImgPool::~SImgPool()
{
    RemoveAll();
}

IBitmap * SImgPool::GetImage(LPCTSTR pszImgName,LPCTSTR pszType)
{
    IResProvider * pResProvider=GETRESPROVIDER;
    if(!pszType) pszType=pResProvider->FindImageType(pszImgName);
    if(!pszType) return NULL;
    
    SResID resid(pszType,pszImgName);
    if(HasKey(resid))
    {
        return GetKeyObject(resid);
    }
    else
    {
        IBitmap *pImg=pResProvider->LoadImage(pszType,pszImgName);
        if(pImg)
        {
            AddKeyObject(resid,pImg);
        }
        return pImg;
    }
}

}//namespace SOUI