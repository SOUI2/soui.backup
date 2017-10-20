#include "StdAfx.h"
#include "LogParser.h"

const wchar_t KLogField_Time[]=L"time";
const wchar_t KLogField_Pid[]=L"pid";
const wchar_t KLogField_Tid[]=L"tid";
const wchar_t KLogField_Package[]=L"package";
const wchar_t KLogField_Level[]=L"level";
const wchar_t KLogField_Tag[]=L"tag";
const wchar_t KLogField_Content[]=L"content";
const wchar_t KLogField_Module[]=L"module";
const wchar_t KLogField_File[]=L"file";
const wchar_t KLogField_Line[]=L"line";
const wchar_t KLogField_Function[]=L"function";

const wchar_t KLogField_Head[]=L"$(";
const wchar_t KLogField_Tail=L')';
const wchar_t KLogField_Opt_Head=L'[';
const wchar_t KLogField_Opt_Tail=L']';


#define FIELD_ELE(x,fid) {x,ARRAYSIZE(x)-1,fid}

struct LogFieldInfo
{
	const wchar_t * szName;
	const int       nLen;
	Field			field;
};

static const LogFieldInfo KLogFields[]=
{
	{NULL,0,col_line_index},
	FIELD_ELE(KLogField_Time,col_time),
	FIELD_ELE(KLogField_Pid,col_pid),
	FIELD_ELE(KLogField_Tid,col_tid),
	FIELD_ELE(KLogField_Level,col_level),
	FIELD_ELE(KLogField_Tag,col_tag),
	FIELD_ELE(KLogField_Module,col_module),
	FIELD_ELE(KLogField_File,col_source_file),
	FIELD_ELE(KLogField_Line,col_source_line),
	FIELD_ELE(KLogField_Function,col_function),
	FIELD_ELE(KLogField_Content,col_content),
	FIELD_ELE(KLogField_Package,col_package),
};


CLogParse::CLogParse(const SStringW & strName,const SStringW & strFmt, const SStringW & strLevels,int nCodePage)
{
	SplitString(strLevels,L',',m_levels);

	int pos=0;
	for(;;)
	{
		FieldInfo fi = {col_invalid,0};
		int nFind = FindNextField(strFmt,pos,fi.field,fi.nLeastLength);
		if(nFind==-1) break;
		SStringW strSep=strFmt.Mid(pos,nFind-pos);
		m_seps.Add(strSep);
		m_fields.Add(fi);
		pos = strFmt.Find(KLogField_Tail,nFind+2+KLogFields[fi.field].nLen);
		if(pos==-1) break;
		pos += 1;//sizeof(KLogField_Tail)
	}
	SStringW strTail= strFmt.Mid(pos);
	m_seps.Add(strTail);
	m_strName = strName;
	m_codePage = nCodePage;
}

Field CLogParse::DetectField(LPCWSTR pszBuf) const
{
	for(int i=1;i<ARRAYSIZE(KLogFields);i++)
	{
		if(wcsnicmp(pszBuf,KLogFields[i].szName,KLogFields[i].nLen)==0)
			return KLogFields[i].field;
	}
	return col_invalid;
}	

int CLogParse::FindNextField(const SStringW & strFmt,int iStart,Field &iField,int &nFieldLength) const
{
	int iFind = strFmt.Find(KLogField_Head,iStart);
	if(iFind==-1) return -1;
	LPCWSTR pszFmt = strFmt;
	pszFmt += iFind+2;//2 = ARRAYSIZE(KLogField_Head)-1
	iField = DetectField(pszFmt);	
	if(iField!=col_invalid)
	{
		pszFmt +=  KLogFields[iField].nLen;
		if(*pszFmt == KLogField_Opt_Head)
		{
			nFieldLength = _wtoi(pszFmt+1);
		}else
		{
			nFieldLength = 0;
		}
		return iFind;
	}else
	{
		return FindNextField(strFmt,iFind+1,iField,nFieldLength);
	}
}

BOOL CLogParse::ParseLine(LPCWSTR pszLine,int nLen,SLogInfo **ppLogInfo) const
{
	LPCWSTR p = pszLine;
	CAutoRefPtr<SLogInfo> pLogInfo;
	pLogInfo.Attach(new SLogInfo);

	SStringW strHead = m_seps[0];
	if(!strHead.IsEmpty())
	{
		if(wcsncmp(p,strHead,strHead.GetLength()) != 0)
			return FALSE;
		p += strHead.GetLength();
	}

	int iContent = -1;

	LPCWSTR pEnd = pszLine+nLen;

	for(int i=0;i<m_fields.GetCount();i++)
	{
		if(m_fields[i].field == col_content)
		{
			iContent = i;
			break;
		}

		SStringW strTail=m_seps[i+1];
		SASSERT(!strTail.IsEmpty());

		if(p+m_fields[i].nLeastLength >= pEnd) return FALSE;
		LPCWSTR pszTail= wcsstr(p+m_fields[i].nLeastLength,strTail);
		if(!pszTail) return FALSE;
		int nLen = (int) (pszTail - p);
		//p -> pTail : field
		FillField(pLogInfo,p,pszTail,m_fields[i].field);
		p += nLen + strTail.GetLength();
	}

	LPCWSTR pszTail = pEnd;

	SStringW strTail = m_seps[m_fields.GetCount()];
	if(!strTail.IsEmpty())
	{
		if(pszTail - p < strTail.GetLength()) return FALSE;
		if(wcsncmp(pszTail-strTail.GetLength(),strTail,strTail.GetLength())!=0) return FALSE;
		pszTail -= strTail.GetLength();
	}

	for(int i=m_fields.GetCount()-1;i>=0;i--)
	{
		if(m_fields[i].field == col_content) break;

		SStringW strHead = m_seps[i];
		SASSERT(!strHead.IsEmpty());
		LPCWSTR pszHead = StrRStr(p,pszTail,strHead);
		if(!pszHead) return FALSE;
		//pszHead + strHead.GetLength -> pszTail;
		FillField(pLogInfo,pszHead + strHead.GetLength(),pszTail,m_fields[i].field);

		pszTail = pszHead;
	}


	//p -> pszTail = content
	FillField(pLogInfo,p,pszTail,col_content);

	if(ppLogInfo)
	{
		*ppLogInfo = pLogInfo;
		pLogInfo->AddRef();
	}
	return TRUE;
}

LPCWSTR CLogParse::StrRStr(LPCWSTR pszSource,LPCWSTR pszTail,LPCWSTR pszDest) const
{
	SASSERT(pszSource);
	SASSERT(pszDest);
	LPCWSTR p = pszTail;
	if(!p) p = pszSource + wcslen(pszSource);
	int nLen = wcslen(pszDest);
	p-=nLen;
	while(p>pszSource)
	{
		if(wcsncmp(p,pszDest,nLen) == 0) return p;
		p--;
	}
	return NULL;
}

void CLogParse::FillField(SLogInfo *info, LPCWSTR pszHead,LPCWSTR pszTail,int fieldId) const
{
	SStringW strTmp(pszHead,(int)(pszTail-pszHead));
	switch(fieldId)
	{
	case col_time:
		info->strTime = strTmp;
		break;
	case col_pid:
		info->dwPid = _wtoi(strTmp);
		break;
	case col_tid:
		info->dwTid = _wtoi(strTmp);
		break;
	case col_package:
		info->strPackage = strTmp;
		break;
	case col_tag:
		info->strTag = strTmp;
		break;
	case col_level:
		info->iLevel = Str2Level(strTmp);
		info->strLevel = strTmp;
		break;
	case col_content:
		info->strContent = strTmp;
		info->strContentLower = strTmp.MakeLower();
		break;
	case col_source_file:
		info->strSourceFile = strTmp;
		break;
	case col_source_line:
		info->iSourceLine = _wtoi(strTmp);
		break;
	case col_function:
		info->strFunction = strTmp;
		break;
	case col_module:
		info->strModule = strTmp;
		break;
	}
}

int CLogParse::Str2Level(const SStringW & strLevel) const
{
	for(int i=0;i<m_levels.GetCount();i++)
	{
		if(m_levels[i]==strLevel) return i;
	}
	return 0;
}

int CLogParse::GetLevels() const
{
	return m_levels.GetCount();
}

void CLogParse::GetLevelText(wchar_t szLevels[][MAX_LEVEL_LENGTH]) const
{
	for(int i=0;i<m_levels.GetCount();i++)
	{
		wcscpy(szLevels[i],m_levels[i]);
	}
}

SOUI::SStringW CLogParse::GetName() const
{
	return m_strName;
}

bool CLogParse::IsFieldValid(Field field) const
{
	for(int i=0;i<m_fields.GetCount();i++)
	{
		if(m_fields[i].field == field) return true;
	}
	return false;
}

int CLogParse::GetCodePage() const
{
	return m_codePage;
}

BOOL CLogParse::TestLogBuffer(LPCSTR pszBuf, int nLength)
{
	LPCSTR pszNextLine = strstr(pszBuf,"\n");
	if(!pszNextLine) return FALSE;
	SStringA strLine(pszBuf,pszNextLine-pszBuf);
	SStringW wstrLine = S_CA2W(strLine,GetCodePage());
	return ParseLine(wstrLine,wstrLine.GetLength(),NULL);
}
