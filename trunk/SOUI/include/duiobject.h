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
    static BOOL CheckAndNew(LPCSTR lpszName,void **ppRet)       \
    {                                                   \
        if (strcmp(GetClassName(), lpszName)  == 0)     \
        {                                                \
            * (theclass**)ppRet=new theclass;            \
            return TRUE;                                \
        }                                                \
        else                                            \
            return FALSE;                               \
    }                                                   \
                                                        \
    static LPCSTR GetClassName()                        \
    {                                                   \
        return classname;                               \
    }                                                   \
                                                        \
    static LPCSTR BaseClassName()                        \
    {                                                    \
        return __super::GetClassName();                    \
    }                                                    \
                                                        \
    virtual LPCSTR GetObjectClass()                     \
    {                                                   \
        return classname;                               \
    }                                                   \
                                                        \
    virtual BOOL IsClass(LPCSTR lpszName)               \
    {                                                   \
        if(strcmp(GetClassName(), lpszName)  == 0) return TRUE;  \
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

    static LPCSTR GetClassName()
    {
        return NULL;
    }

    static LPCSTR BaseClassName()
    {
        return NULL;
    }

    virtual BOOL IsClass(LPCSTR lpszName)
    {
        return FALSE;
    }

    virtual LPCSTR GetObjectClass()
    {
        return NULL;
    }


    virtual BOOL Load(pugi::xml_node xmlNode);

    virtual HRESULT SetAttribute(const CDuiStringA &  strAttribName, const CDuiStringA &  strValue, BOOL bLoading)
    {
        return DefAttributeProc(strAttribName,strValue,bLoading);
    }

    virtual HRESULT SetAttributeW(const CDuiStringA &  strAttribName, const CDuiStringW &  strValue, BOOL bLoading)
    {
        CDuiStringA strValueUTF8=DUI_CW2A(strValue,CP_UTF8);
        return SetAttribute(strAttribName,strValueUTF8,bLoading);
    }

    virtual HRESULT DefAttributeProc(const CDuiStringA & strAttribName,const CDuiStringA & strValue, BOOL bLoading)
    {
        return E_FAIL;
    }
    //tolua_end
protected:
    virtual void OnAttributeFinish(pugi::xml_node xmlNode) {}
    virtual void OnAttributeChanged(const CDuiStringA & strAttrName,BOOL bLoading,HRESULT hRet) {}

public:
    static ULONG HexStringToULong(LPCSTR lpszValue, int nSize = -1)
    {
        LPCSTR pchValue = lpszValue;
        ULONG ulValue = 0;
        while (*pchValue && nSize != 0)
        {
            ulValue <<= 4;

            if ('a' <= *pchValue && 'f' >= *pchValue)
                ulValue |= (*pchValue - 'a' + 10);
            else if ('A' <= *pchValue && 'F' >= *pchValue)
                ulValue |= (*pchValue - 'A' + 10);
            else if ('0' <= *pchValue && '9' >= *pchValue)
                ulValue |= (*pchValue - '0');
            else
                return 0;

            ++ pchValue;
            -- nSize;
        }

        return ulValue;
    }

    static COLORREF HexStringToColor(LPCSTR lpszValue)
    {
        COLORREF cr=RGB(
            HexStringToULong(lpszValue, 2),
            HexStringToULong(lpszValue + 2, 2),
            HexStringToULong(lpszValue + 4, 2)
            );
        if(strlen(lpszValue)>6)
        {
            cr |= HexStringToULong(lpszValue + 6, 2)<<24;
        }
        return cr;
    }

#ifdef    _DEBUG
    CDuiStringA m_strXml;
#endif//_DEBUG
};

}//namespace SOUI
