/**
* Copyright (C) 2014-2050 
* All rights reserved.
* 
* @file       SAttrCracker.h
* @brief      
* @version    v1.0      
* @author     SOUI group   
* @date       2014/08/15
* 
* Describe    定义SOUI的XML属性解析宏
*/

#ifndef _SATTRCRACK_H
#define _SATTRCRACK_H

#pragma  once


// Attribute Declaration
#define SOUI_ATTRS_BEGIN()                            \
public:                                                             \
    virtual HRESULT SetAttribute(                                   \
    const SStringW & strAttribName,                                     \
    const SStringW &  strValue,                                          \
    BOOL     bLoading=FALSE)                                    \
    {                                                               \
    HRESULT hRet = E_FAIL;                                        \
 

//从SObject派生的类是属性结尾
#define SOUI_ATTRS_END()                                        \
    return __super::SetAttribute(                               \
    strAttribName,                                              \
    strValue,                                                   \
    bLoading                                                    \
    );                                                          \
    return AfterAttribute(strAttribName,strValue,hRet);         \
    }                                                           \
    

//不是从SObject派生的类是属性结尾
#define SOUI_ATTRS_BREAK()                                      \
        return E_NOTIMPL;                                       \
    return hRet;                                                \
    }                                                           \

 
#define ATTR_CHAIN(varname)                               \
    if (SUCCEEDED(hRet = varname.SetAttribute(strAttribName, strValue, bLoading)))   \
        {                                                           \
        /*return hRet;*/                                            \
        }                                                           \
        else                                                        \
 
#define ATTR_CUSTOM(attribname, func)                    \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        hRet = func(strValue, bLoading);                        \
        }                                                           \
        else                                                        \
 
// Int = %d StringA
#define ATTR_INT(attribname, varname, allredraw)         \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        int nRet=0;                                                \
        ::StrToIntExW(strValue,STIF_SUPPORT_HEX,&nRet);            \
        varname=nRet;                                            \
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
        else                                                        \
 
// Rect = %d,%d,%d,%d StringA
#define ATTR_RECT(attribname, varname, allredraw)         \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        swscanf(strValue,L"%d,%d,%d,%d",&varname.left,&varname.top,&varname.right,&varname.bottom);\
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
        else                                                        \
 

// Size = %d,%d StringA
#define ATTR_SIZE(attribname, varname, allredraw)         \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        swscanf(strValue,L"%d,%d",&varname.cx,&varname.cy);\
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
        else                                                        \
 
// Point = %d,%d StringA
#define ATTR_POINT(attribname, varname, allredraw)         \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        swscanf(strValue,L"%d,%d",&varname.x,&varname.y);\
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
        else                                                        \
 

// Float = %f StringA
#define ATTR_FLOAT(attribname, varname, allredraw)         \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        swscanf(strValue,L"%f",&varname);                        \
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
        else                                                        \
 
// UInt = %u StringA
#define ATTR_UINT(attribname, varname, allredraw)        \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        int nRet=0;                                                \
        ::StrToIntExW(strValue,STIF_SUPPORT_HEX,&nRet);            \
        varname = (UINT)nRet;                                    \
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
        else                                                        \
 
// DWORD = %u StringA
#define ATTR_DWORD(attribname, varname, allredraw)       \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        int nRet=0;                                                \
        ::StrToIntExW(strValue,STIF_SUPPORT_HEX,&nRet);            \
        varname = (DWORD)nRet;                                    \
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
        else                                                        \
 
// WORD = %u StringA
#define ATTR_WORD(attribname, varname, allredraw)       \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        int nRet=0;                                                \
        ::StrToIntExW(strValue,STIF_SUPPORT_HEX,&nRet);            \
        varname = (WORD)nRet;                                    \
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
        else                                                        \
 

// bool = 0 or 1 StringA
#define ATTR_BIT(attribname, varname, maskbit, allredraw) \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        int nRet=0;                                                \
        ::StrToIntW(strValue,&nRet);                                \
        if(nRet) varname|=maskbit;                                \
            else     varname &=~(maskbit);                            \
            hRet = allredraw ? S_OK : S_FALSE;                        \
        }                                                           \
        else                                                        \
 

// StringA = StringA
#define ATTR_STRINGA(attribname, varname, allredraw)      \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        SStringW strTmp=strValue;                                   \
        BUILDSTRING(strTmp);                                    \
        varname = S_CW2A(strTmp);                              \
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
        else                                                        \
 
// StringW = StringA
#define ATTR_STRINGW(attribname, varname, allredraw)      \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        SStringT strTmp=S_CW2T(strValue);                          \
        BUILDSTRING(strTmp);                                    \
        varname = S_CT2W(strTmp);                                        \
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
        else                                                        \
 

// StringT = StringA
#define ATTR_STRINGT(attribname, varname, allredraw)     \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        varname=S_CW2T(strValue);                          \
        BUILDSTRING(varname);                                    \
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
        else                                                        \


// StringA = StringA
#define ATTR_I18NSTRA(attribname, varname, allredraw)      \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                       \
        SStringT strTmp=S_CW2T(tr(strValue));                   \
        BUILDSTRING(strTmp);                                    \
        varname = S_CT2A(strTmp);                                     \
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
        else                                                        \

// StringW = StringA
#define ATTR_I18NSTRW(attribname, varname, allredraw)           \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                       \
        SStringT strTmp=S_CW2T(tr(strValue));                   \
        BUILDSTRING(strTmp);                                    \
        varname = S_CT2W(strTmp);                                \
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
        else                                                        \


// StringT = StringA
#define ATTR_I18NSTRT(attribname, varname, allredraw)           \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                       \
        varname=S_CW2T(tr(strValue));                           \
        BUILDSTRING(varname);                                   \
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                       \
        else                                                    \

// DWORD = %X StringA
#define ATTR_HEX(attribname, varname, allredraw)                \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                       \
        varname = HexStringToULong(strValue);                   \
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                       \
        else                                                    \
 
// COLORREF = %06X StringA
#define ATTR_COLOR(attribname, varname, allredraw)                  \
    if (0 == strAttribName.CompareNoCase(attribname))               \
        {                                                           \
            if(!strValue.IsEmpty())                                 \
            {                                                       \
                varname = StringToColor(strValue);                  \
                hRet = allredraw ? S_OK : S_FALSE;                  \
            }else                                                   \
            {                                                       \
                hRet=E_FAIL;                                        \
            }                                                       \
        }                                                           \
        else                                                        \
 

//font="face:宋体;bold:1;italic:1;underline:1;adding:10"
#define ATTR_FONT(attribname, varname, allredraw)                       \
    if (0 == strAttribName.CompareNoCase(attribname))                   \
    {                                                                    \
        varname = SFontPool::getSingleton().GetFont(strValue);           \
        hRet = allredraw ? S_OK : S_FALSE;                               \
    }                                                                    \
    else                                                                 \


// Value In {String1 : Value1, String2 : Value2 ...}
#define ATTR_ENUM_BEGIN(attribname, vartype, allredraw)        \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        vartype varTemp;                                        \
        \
        hRet = allredraw ? S_OK : S_FALSE;                      \
 
#define ATTR_ENUM_VALUE(enumstring, enumvalue)                     \
    if (strValue == enumstring)                             \
    varTemp = enumvalue;                                \
            else                                                    \
 
#define ATTR_ENUM_END(varname)                                     \
    return E_FAIL;                                      \
    \
    varname = varTemp;                                      \
        }                                                           \
        else                                                        \
 
// SwndStyle From StringA Key
#define ATTR_STYLE(attribname, varname, allredraw)       \
    if (0 == strAttribName.CompareNoCase(attribname))                            \
        {                                                           \
        GETSTYLE(strValue,varname);                  \
        hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
        else                                                        \
 
// SSkinPool From StringA Key
#define ATTR_SKIN(attribname, varname, allredraw)                   \
    if (0 == strAttribName.CompareNoCase(attribname))               \
        {                                                           \
        varname = GETSKIN(strValue);                                \
        hRet = allredraw ? S_OK : S_FALSE;                          \
        }                                                           \
        else                                                        \
 

//ATTR_IMAGE:直接使用IResProvider::LoadImage创建IBitmap对象，创建成功后引用计数为1
//不需要调用AddRef，但是用完后需要调用Release
#define ATTR_IMAGE(attribname, varname, allredraw)                  \
    if (0 == strAttribName.CompareNoCase(attribname))               \
    {                                                               \
        IBitmap *pImg=LOADIMAGE2(strValue);                          \
        if(!pImg) hRet =E_FAIL;                                     \
        else{                                                       \
            if(varname) varname->Release();                         \
            varname = pImg;                                         \
            hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
     }                                                              \
     else                                                           \


//ATTR_IMAGEAUTOREF:varname应该是一个CAutoRefPtr<IBitmap>对象
#define ATTR_IMAGEAUTOREF(attribname, varname, allredraw)           \
    if (0 == strAttribName.CompareNoCase(attribname))               \
    {                                                               \
        IBitmap *pImg=LOADIMAGE2(strValue);                         \
        if(!pImg) hRet =E_FAIL;                                     \
        else{                                                       \
            varname = pImg;                                         \
            pImg->Release();                                        \
            hRet = allredraw ? S_OK : S_FALSE;                      \
        }                                                           \
     }                                                              \
     else                                                           \

 
#define ATTR_ICON(attribname, varname, allredraw)                  \
    if (0 == strAttribName.CompareNoCase(attribname))              \
    {                                                              \
        if(varname) DeleteObject(varname);                         \
        varname = LOADICON2(strValue);                             \
        hRet = allredraw ? S_OK : S_FALSE;                         \
    }                                                              \
    else                                                           \

#endif//_SATTRCRACK_H