//////////////////////////////////////////////////////////////////////////
//   File Name: sobject.h
//////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////

// DuiObject Class Name Declaration
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
    SObject():m_nID(0)
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
        return DefAttributeProc(DUI_CA2W(strAttribName),DUI_CA2W(strValue),bLoading);
    }

    virtual HRESULT SetAttribute(const SStringW &  strAttribName, const SStringW &  strValue, BOOL bLoading)
    {
        return DefAttributeProc(strAttribName,strValue,bLoading);
    }

    virtual HRESULT DefAttributeProc(const SStringW & strAttribName,const SStringW & strValue, BOOL bLoading)
    {
        if(strAttribName == L"name")
        {
            m_strName=strValue;
            return S_OK;
        }else if(strAttribName==L"id")
        {
            m_nID=::StrToIntW(strValue);
        }
        return E_FAIL;
    }

    LPCWSTR GetName(){return m_strName;}
    void SetName(LPCWSTR pszName){m_strName=pszName;}

    int GetID(){return m_nID;}
    void SetID(int nID){m_nID=nID;}
protected:
    virtual void OnInitFinished(pugi::xml_node xmlNode) {}
    virtual void OnAttributeChanged(const SStringW & strAttrName,BOOL bLoading,HRESULT hRet) {}

#ifdef    _DEBUG
    SStringW m_strXml;
#endif//_DEBUG
    SStringW m_strName;
    int     m_nID;
};

}//namespace SOUI
