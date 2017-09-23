#ifndef __ETIMES_TIMEFUN_H___
#define __ETIMES_TIMEFUN_H___
#pragma once

#include <string>

namespace EtimesTimeFun
{
	//************************************
	// Method:    获取 当前用户的用户目录  
	//************************************
	std::wstring Etimes_GetDateTimeFormat(SYSTEMTIME* pTime=NULL, wchar_t firstChar='-', wchar_t secondChar=':')
	{	
		TCHAR lpTimeBuf[1024] = {0};
		SYSTEMTIME sysTime;
		if(NULL == pTime)
		{			
			GetLocalTime(&sysTime);
			pTime = &sysTime;
		}
		

		int nLen = swprintf_s(lpTimeBuf, 1024, L"%04d%c%02d%c%02d %02d%c%02d%c%02d", 
			pTime->wYear,
			firstChar,
			pTime->wMonth,
			firstChar,
			pTime->wDay,
			
			pTime->wHour,
			secondChar,
			pTime->wMinute,
			secondChar,
			pTime->wSecond);
	
		return std::wstring(lpTimeBuf, nLen);
	}

	std::wstring Etimes_GetDateFormat(SYSTEMTIME* pTime=NULL, wchar_t cChar='-')
	{
		TCHAR lpTimeBuf[1024] = {0};
		SYSTEMTIME sysTime;
		if(NULL == pTime)
		{			
			GetLocalTime(&sysTime);
			pTime = &sysTime;
		}
		
		int nLen = swprintf_s(lpTimeBuf, 1024, L"%04d%c%02d%c%02d", 
			pTime->wYear,
			cChar,
			pTime->wMonth,
			cChar,
			pTime->wDay);

		return std::wstring(lpTimeBuf, nLen);
	}

	std::wstring Etimes_GetTimeFormat(SYSTEMTIME* pTime=NULL, wchar_t cChar=':')
	{
		TCHAR lpTimeBuf[1024] = {0};
		SYSTEMTIME sysTime;
		if(NULL == pTime)
		{			
			GetLocalTime(&sysTime);
			pTime = &sysTime;
		}

		int nLen = swprintf_s(lpTimeBuf, 1024, L"%02d%c%02d%c%02d", 
			pTime->wHour,
			cChar,
			pTime->wMinute,
			cChar,
			pTime->wSecond);

		return std::wstring(lpTimeBuf, nLen);
	}
}




#endif