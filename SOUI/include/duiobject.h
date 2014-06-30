//////////////////////////////////////////////////////////////////////////
//   File Name: DuiObject.h
// Description: DuiObject Definition
//     Creator: Zhang Xiaoxuan
//     Version: 2009.04.28 - 1.0 - Create
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


    virtual BOOL Load(pugi::xml_node xmlNode);

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
        return E_FAIL;
    }
    //tolua_end
protected:
    virtual void OnAttributeFinish(pugi::xml_node xmlNode) {}
    virtual void OnAttributeChanged(const SStringW & strAttrName,BOOL bLoading,HRESULT hRet) {}

public:
    static ULONG HexStringToULong(LPCWSTR lpszValue, int nSize = -1)
    {
        LPCWSTR pchValue = lpszValue;
        ULONG ulValue = 0;
        while (*pchValue && nSize != 0)
        {
            ulValue <<= 4;

            if ('a' <= *pchValue && L'f' >= *pchValue)
                ulValue |= (*pchValue - L'a' + 10);
            else if ('A' <= *pchValue && L'F' >= *pchValue)
                ulValue |= (*pchValue - L'A' + 10);
            else if ('0' <= *pchValue && L'9' >= *pchValue)
                ulValue |= (*pchValue - L'0');
            else
                return 0;

            ++ pchValue;
            -- nSize;
        }

        return ulValue;
    }

    static COLORREF HexStringToColor(LPCWSTR lpszValue)
    {
        COLORREF cr=RGB(
            HexStringToULong(lpszValue, 2),
            HexStringToULong(lpszValue + 2, 2),
            HexStringToULong(lpszValue + 4, 2)
            );
        if(wcslen(lpszValue)>6)
        {
            cr |= HexStringToULong(lpszValue + 6, 2)<<24;
        }else
        {
            cr |= 0xFF000000;
        }
        return cr;
    }

#ifdef    _DEBUG
    SStringW m_strXml;
#endif//_DEBUG
};

}//namespace SOUI
