//////////////////////////////////////////////////////////////////////////
//   File Name: sobject.h
//////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////

// SObject Class Name Declaration
#define SOUI_CLASS_NAME(theclass, classname)   \
public:                                                 \
    static BOOL CheckAndNew(LPCWSTR lpszName,void **ppRet)       \
    {                                                   \
        if (wcscmp(GetClassName(), lpszName)  == 0)     \
        {                                                \
            * (theclass**)ppRet=new theclass;            \
            return TRUE;                                \
        }                                                \
        else                                            \
            return FALSE;                               \
    }                                                   \
                                                        \
    static LPCWSTR GetClassName()                        \
    {                                                   \
        return classname;                               \
    }                                                   \
                                                        \
    static LPCWSTR BaseClassName()                        \
    {                                                    \
        return __super::GetClassName();                    \
    }                                                    \
                                                        \
    virtual LPCWSTR GetObjectClass()                     \
    {                                                   \
        return classname;                               \
    }                                                   \
                                                        \
    virtual BOOL IsClass(LPCWSTR lpszName)               \
    {                                                   \
        if(wcscmp(GetClassName(), lpszName)  == 0) return TRUE;  \
        return __super::IsClass(lpszName);                \
    }                                                   \


namespace SOUI
{

class SOUI_EXP SObject
{
public:
    SObject()
    {
    }

    virtual ~SObject()
    {
    }

    static LPCWSTR GetClassName()
    {
        return NULL;
    }

    static LPCWSTR BaseClassName()
    {
        return NULL;
    }

    virtual BOOL IsClass(LPCWSTR lpszName)
    {
        return FALSE;
    }

    virtual LPCWSTR GetObjectClass()
    {
        return NULL;
    }

    virtual BOOL InitFromXml(pugi::xml_node xmlNode);

    virtual HRESULT SetAttribute(const SStringA &  strAttribName, const SStringA &  strValue, BOOL bLoading)
    {
        return DefAttributeProc(S_CA2W(strAttribName),S_CA2W(strValue),bLoading);
    }

    virtual HRESULT SetAttribute(const SStringW &  strAttribName, const SStringW &  strValue, BOOL bLoading)
    {
        return DefAttributeProc(strAttribName,strValue,bLoading);
    }

    virtual HRESULT DefAttributeProc(const SStringW & strAttribName,const SStringW & strValue, BOOL bLoading)
    {
        return E_FAIL;
    }

    virtual SStringW tr(const SStringW &strSrc);

protected:
    virtual void OnInitFinished(pugi::xml_node xmlNode) {}

#ifdef    _DEBUG
    SStringW m_strXml;
#endif//_DEBUG
};

}//namespace SOUI
