#pragma once

#include <unknown/obj-ref-i.h>
#include <string/tstring.h>

namespace SOUI
{
    struct ILang : public IObjRef
    {
        virtual BOOL Load(LPVOID pData,UINT uType)=0;
        virtual SStringW name()=0;
        virtual GUID     guid()=0;
        virtual BOOL tr(const SStringW & strSrc,const SStringW & strCtx,SStringW & strRet)=0;
    };

    struct ITranslator : public IObjRef
    {
        virtual BOOL CreateLang(ILang ** ppLang)=0;
        virtual BOOL InstallLang(ILang * pLang) =0;
        virtual BOOL UninstallLang(REFGUID id) =0;
        virtual SStringW tr(const SStringW & strSrc,const SStringW & strCtx)=0;
    };

    struct ITranslatorFactory : public IObjRef
    {
        virtual BOOL CreateTranslator(ITranslator **ppTranslator)=0;
    };
}