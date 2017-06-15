#include "StdAfx.h"
#include "LogParser.h"

static const int      kLevelNum = 6;
static const wchar_t  kLevelsApp[kLevelNum][MAX_LEVEL_LENGTH]={
	L"[V]",L"[D]",L"[I]",L"[W]",L"[E]",L"[A]"
};

//////////////////////////////////////////////////////////////////////////
//2017-05-22 10:10:18.794 (24894841) 32376 [D][RTCSDK] - 
#define LOG_APP_SEP L" - "
void CAppLogParse::GetLevelText(wchar_t szLevels[][MAX_LEVEL_LENGTH]) const
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

bool CAppLogParse::IsFieldValid(Field field) const
{
	bool fieldValid[]=
	{
		true,//col_line_index,
		true,//col_time,
		true,//col_pid,
		true,//col_tid,
		true,//col_level,
		true,//col_tag,
		false,//col_moduel,
		false,//col_source_file,
		false,//col_source_line,
		false,//col_function,
		true,//col_content
	};
	return fieldValid[field];
}

int CAppLogParse::GetCodePage() const
{
	return CP_UTF8;
}

BOOL CAppLogParse::TestLogBuffer(LPCSTR pszBuf, int nLength)
{
	LPCSTR pszNextLine = strstr(pszBuf,"\n");
	if(!pszNextLine) return FALSE;
	SStringA strLine(pszBuf,pszNextLine-pszBuf);
	SStringW wstrLine = S_CA2W(strLine,GetCodePage());
	return ParseLine(wstrLine,NULL);
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


bool CLogcatParse::IsFieldValid(Field field) const
{
	bool fieldValid[]=
	{
		true,//col_line_index,
		true,//col_time,
		true,//col_pid,
		true,//col_tid,
		true,//col_level,
		true,//col_tag,
		false,//col_moduel,
		false,//col_source_file,
		false,//col_source_line,
		false,//col_function,
		true,//col_content
	};
	return fieldValid[field];
}


int CLogcatParse::GetCodePage() const
{
	return CP_UTF8;
}

BOOL CLogcatParse::TestLogBuffer(LPCSTR pszBuf, int nLength)
{
	LPCSTR pszNextLine = strstr(pszBuf,"\n");
	if(!pszNextLine) return FALSE;
	SStringA strLine(pszBuf,pszNextLine-pszBuf);
	SStringW wstrLine = S_CA2W(strLine,GetCodePage());
	return ParseLine(wstrLine,NULL);
}

//////////////////////////////////////////////////////////////////////////
static const int KSLogLevels = 7;
static const wchar_t  kLevelsSouiLog[KSLogLevels][MAX_LEVEL_LENGTH]={
	L"TRACE",
	L"DEBUG",
	L"INFO ",
	L"WARN ",
	L"ERROR",
	L"ALARM",
	L"FATAL",
};

//pid=1740 tid=11768 2017-06-12 10:14:49.803 WARN  soui.dll soui-lib "Warning: no ojbect menuItem of type:1 in SOUI!!" SOUI::SObjectFactoryMgr::CreateObject (SObjectFactory.cpp):61 

BOOL CSouiLogParse::ParseLine(LPCWSTR pszLine,SLogInfo **ppLogInfo) const
{
	UINT year,month,day,hour,minute,second,millisec;
	UINT pid,tid;
	WCHAR level[300]={0}, tag[500]={0}, module[MAX_PATH];

	if(12!= swscanf(pszLine,L"pid=%u tid=%u %04d-%2d-%2d %2d:%2d:%2d.%3d %s %s %s ",&pid,&tid,&year,&month,&day,&hour,&minute,&second,&millisec,level,module,tag))
		return FALSE;
	
	if(ppLogInfo == NULL) return TRUE;
	
	LPCWSTR pContent = wcsstr(pszLine,L" \"");
	SASSERT(pContent);
	//find the reverse second blank space
	LPCWSTR pContentEnd = NULL;
	int nLen = wcslen(pContent);
	int iSpace = 0;
	for(int i=nLen-1;i>=0;i--)
	{
		if(pContent[i] == L' ')
		{
			iSpace++;
			if(iSpace==2)
			{
				if(pContent[i-1]!=L'\"') return FALSE;
				pContentEnd = pContent+i-1;
				break;
			}
		}
	}
	if(iSpace!=2) return FALSE;
	LPCWSTR pszFunction = pContentEnd+2;
	LPCWSTR pSourceFile = wcsstr(pContentEnd,L" (")+2;
	LPCWSTR pSourceLine = wcsstr(pSourceFile,L"):")+2;

	SLogInfo * info = new SLogInfo;
	info->strTime.Format(L"%04d-%02d-%02d %02d:%02d:%02d.%03d",year,month,day,hour,minute,second,millisec);
	info->time = CTime(2017,month,day,hour,minute,second);
	info->strLevel = level;
	info->strTag = tag;
	info->strContent = SStringW(pContent+2,pContentEnd-pContent-3);
	info->strFunction = SStringW(pszFunction,pSourceFile-pszFunction-2);
	info->strSourceFile = SStringW(pSourceFile,pSourceLine-pSourceFile-2);
	info->strModule = module;
	info->iSourceLine = _wtoi(pSourceLine);

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

int CSouiLogParse::GetLevels() const
{
	return KSLogLevels;
}


void CSouiLogParse::GetLevelText(wchar_t szLevels[][MAX_LEVEL_LENGTH]) const
{
	memcpy(szLevels,kLevelsSouiLog,sizeof(kLevelsSouiLog));
}

SOUI::SStringW CSouiLogParse::GetName() const
{
	return L"SouilogParser";
}

bool CSouiLogParse::IsFieldValid(Field field) const
{
	return true;
}

int CSouiLogParse::GetCodePage() const
{
	return CP_ACP;
}

BOOL CSouiLogParse::TestLogBuffer(LPCSTR pszBuf, int nLength)
{
	LPCSTR pszNextLine = strstr(pszBuf,"\n");
	if(!pszNextLine) return FALSE;
	SStringA strLine(pszBuf,pszNextLine-pszBuf);
	SStringW wstrLine = S_CA2W(strLine,GetCodePage());
	return ParseLine(wstrLine,NULL);
}


//////////////////////////////////////////////////////////////////////////
ILogParse * CParseFactory::CreateLogParser(int iParser) const
{
	switch(iParser)
	{
	case 0:
		return new CAppLogParse;
	case 1:
		return new CLogcatParse;
	case 2:
		return new CSouiLogParse;
		break;
	default:
		return NULL;
	}
}

int CParseFactory::GetLogParserCount() const
{
	return 3;
}

