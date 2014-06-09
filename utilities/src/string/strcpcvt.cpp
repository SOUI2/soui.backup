#include "stdafx.h"
#include "string/strcpcvt.h"

namespace SOUI
{

CDuiStringW CDuiStrCpCvt::CvtW2W( const CDuiStringW &str,unsigned int cp/*=CP_ACP*/ )
{
    return str;
}

CDuiStringA CDuiStrCpCvt::CvtA2A( const CDuiStringA & str,unsigned int cpFrom/*=CP_UTF8*/,unsigned int cpTo/*=CP_ACP*/ )
{
    if(cpTo==cpFrom)
        return str;
    CDuiStringW strw=CvtA2W(str,cpFrom);
    return CvtW2A(strw,cpTo);
}

CDuiStringW CDuiStrCpCvt::CvtA2W( const CDuiStringA & str,unsigned int cp/*=CP_ACP*/,unsigned int cp2/*=0*/ )
{
    UNREFERENCED_PARAMETER(cp2);
    wchar_t szBuf[1024];
    int nRet=MultiByteToWideChar(cp,0,str,str.GetLength(),szBuf,1024);
    if(nRet>0)
    {
        return CDuiStringW(szBuf,nRet);
    }
    if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
    {
        int nRet=MultiByteToWideChar(cp,0,str,str.GetLength(),NULL,0);
        if(nRet>0)
        {
            wchar_t *pBuf=new wchar_t[nRet];
            MultiByteToWideChar(cp,0,str,str.GetLength(),pBuf,nRet);
            CDuiStringW strRet(pBuf,nRet);
            delete []pBuf;
            return strRet;
        }
    }
    return L"";
}

CDuiStringA CDuiStrCpCvt::CvtW2A( const CDuiStringW & str,unsigned int cp/*=CP_ACP*/ )
{
    char szBuf[1024];
    int nRet=WideCharToMultiByte(cp,0,str,str.GetLength(),szBuf,1024,NULL,NULL);
    if(nRet>0) return CDuiStringA(szBuf,nRet);
    if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
    {
        int nRet=WideCharToMultiByte(cp,0,str,str.GetLength(),NULL,0,NULL,NULL);
        if(nRet>0)
        {
            char *pBuf=new char[nRet];
            WideCharToMultiByte(cp,0,str,str.GetLength(),pBuf,nRet,NULL,NULL);
            CDuiStringA strRet(pBuf,nRet);
            delete []pBuf;
            return strRet;
        }
    }
    return "";
}


}
