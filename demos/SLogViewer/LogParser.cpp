#include "StdAfx.h"
#include "LogParser.h"

static const int      kLevelNum = 6;
static const wchar_t  kLevelsApp[kLevelNum][MAX_LEVEL_LENGTH]={
	L"[V]",L"[D]",L"[I]",L"[W]",L"[E]",L"[A]"
};

//////////////////////////////////////////////////////////////////////////
//2017-05-22 10:10:18.794 (24894841) 32376 [D][RTCSDK] - 
#define LOG_APP_SEP L" - "
void CAppLogParse::GetLevelText(wchar_t szLevels[][50]) const
{
	memcpy(szLevels,kLevelsApp,sizeof(kLevelsApp));
}

int CAppLogParse::GetLevels() const
{
	return kLevelNum;
}

BOOL CAppLogParse::ParseLine(LPCWSTR pszLine,SLogInfo **ppLogInfo) const
{
	UINT year,month,day,hour,minute,second,millisec,tickCount;
	UINT pid;
	WCHAR level[MAX_LEVEL_LENGTH]={0},tag[250]={0};
	const WCHAR *pSep = wcsstr(pszLine,LOG_APP_SEP);
	if(!pSep) return FALSE;

	if(9!=swscanf(pszLine,L"%4d-%2d-%2d %2d:%2d:%2d.%3d (%u) %u",&year,&month,&day,&hour,&minute,&second,&millisec,&tickCount,&pid))
		return FALSE;
	
	if(ppLogInfo == NULL) return TRUE;

	
	const WCHAR* p1 = wcschr(pszLine,L'[');
	const WCHAR* p2 = wcschr(p1,L']');
	wcsncpy(level,p1+1,p2-p1-1);

	p1 = p2+1;
	p2 = wcschr(p1,L']');
	wcsncpy(tag,p1+1,p2-p1-1);

	SLogInfo * info = new SLogInfo;
	info->time = CTime(year,month,day,hour,minute,second);
	info->strTime.Format(L"%04d-%02d-%02d %02d:%02d:%02d.%03d",year,month,day,hour,minute,second,millisec);
	info->strLevel = SStringW(L"[")+level + L"]";
	info->strTag = SStringW(L"[")+tag + L"]";
	info->strContent = pSep + 3;
	info->dwPid = pid;
	info->dwTid = 0;
	
	info->iLevel = 0;
	for(int i=0;i< kLevelNum;i++)
	{
		if(wcscmp(info->strLevel,kLevelsApp[i])==0)
		{
			info->iLevel = i;
		}
	}
	*ppLogInfo = info;
	return TRUE;
}

SStringW CAppLogParse::GetName() const
{
	return L"AppLogParser";
}


//////////////////////////////////////////////////////////////////////////
static const wchar_t  kLevelsLogcat[kLevelNum][MAX_LEVEL_LENGTH]={
	L"V",L"D",L"I",L"W",L"E",L"A"
};

//05-27 03:55:12.360 948-948/vulture.app.home I/NEMO_UI: (58276990) - [BaiduAsr] - onPutData,
//date time PID-TID/package priority/tag: message
#define LOGCAT_SEP L": "

void CLogcatParse::GetLevelText(wchar_t szLevels[][50]) const
{
	memcpy(szLevels,kLevelsLogcat,sizeof(kLevelsLogcat));
}

int CLogcatParse::GetLevels() const
{
	return kLevelNum;
}

BOOL CLogcatParse::ParseLine(LPCWSTR pszLine,SLogInfo **ppLogInfo) const
{
	UINT month,day,hour,minute,second,millisec;
	UINT pid,tid;
	WCHAR level[300]={0}, tag[500]={0};
	const WCHAR *pSep = wcsstr(pszLine,LOGCAT_SEP);
	if(!pSep) return FALSE;

	if(10!= swscanf(pszLine,L"%2d-%2d %2d:%2d:%2d.%3d %u %u %s %s ",&month,&day,&hour,&minute,&second,&millisec,&pid,&tid,level,tag))
		return FALSE;

	if(ppLogInfo == NULL) return TRUE;

	SLogInfo * info = new SLogInfo;
	info->strTime.Format(L"%2d-%2d %2d:%2d:%2d.%3d",month,day,hour,minute,second,millisec);
	info->time = CTime(2017,month,day,hour,minute,second);
	info->strLevel = level;
	info->strTag = tag;
	info->strContent = pSep + 2;
	info->dwPid = pid;
	info->dwTid = tid;
	info->iLevel = 0;
	for(int i=0;i< kLevelNum;i++)
	{
		if(wcscmp(info->strLevel,kLevelsLogcat[i])==0)
		{
			info->iLevel = i;
		}
	}
	*ppLogInfo = info;
	return TRUE;
}

SOUI::SStringW CLogcatParse::GetName() const
{
	return L"LogcatParse";

}


//////////////////////////////////////////////////////////////////////////
ILogParse * CParseFactory::CreateLogParser(int iParser) const
{
	if(iParser == 0)
	{
		return new CAppLogParse;
	}else
	{
		return new CLogcatParse;
	}
}

int CParseFactory::GetLogParserCount() const
{
	return 2;
}
