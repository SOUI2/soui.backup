//////////////////////////////////////////////////////////////////////////
//  Class Name: DuiImgPool
// Description: Image Pool
//     Creator: Huang jianxiong
//     Version: 2012.8.30 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "duistd.h"
#include "duiobject.h"
#include "res.mgr/duiimgpool.h"
#include "res.mgr/duiresprovider.h"
#include "DuiSystem.h"

namespace SOUI
{

template<> DuiImgPool * Singleton<DuiImgPool>::ms_Singleton =0;

DuiImgPool::DuiImgPool()
{
    m_pFunOnKeyRemoved=OnImageRemoved;
}

DuiImgPool::~DuiImgPool()
{
    RemoveAll();
}

IBitmap * DuiImgPool::GetImage(LPCTSTR pszImgName,LPCTSTR pszType)
{
    DuiResID resid(pszType,pszImgName);
    if(HasKey(resid))
    {
        return GetKeyObject(resid);
    }
    else
    {
        IDuiResProvider * pResProvider=GETRESPROVIDER;
        DUIASSERT(pResProvider);
        IBitmap *pImg=pResProvider->LoadImage(pszType,pszImgName);

        if(pImg)
        {
            AddKeyObject(resid,pImg);
            if(pszType!=NULL)
            {
                pImg->AddRef();
                AddKeyObject(DuiResID(NULL,pszImgName),pImg);//name唯一时保证不使用类型也能找到该图片资源
            }
        }
        return pImg;
    }
}

}//namespace SOUI