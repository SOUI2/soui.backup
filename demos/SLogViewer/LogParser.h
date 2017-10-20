#pragma once
#include "SLogAdapter.h"


class CLogParse : public TObjRefImpl<ILogParse>
{
public:
	CLogParse(const SStringW & strName,const SStringW & strFmt, const SStringW & strLevels,int nCodePage);

	virtual BOOL ParseLine(LPCWSTR pszLine,int nLen,SLogInfo **ppLogInfo) const;

	virtual int GetLevels() const;

	virtual void GetLevelText(wchar_t szLevels[][MAX_LEVEL_LENGTH]) const;

	virtual SStringW GetName() const;

	virtual bool IsFieldValid(Field field) const;

	virtual int GetCodePage() const;

	virtual BOOL TestLogBuffer(LPCSTR pszBuf, int nLength);
private:
	Field DetectField(LPCWSTR pszBuf) const;
	int FindNextField(const SStringW & strFmt,int iStart,Field &iField,int &nFieldLength) const;
	
	LPCWSTR StrRStr(LPCWSTR pszSource,LPCWSTR pszTail,LPCWSTR pszDest) const;
	void FillField(SLogInfo *ppLogInfo, LPCWSTR pszHead,LPCWSTR pszTail,int fieldId) const;
	int Str2Level(const SStringW & strLevel) const;
private:
	SStringWList m_levels;
	SStringWList m_seps;

	struct FieldInfo
	{
		Field field;
		int   nLeastLength;
	};
	SArray<FieldInfo>  m_fields;
	SStringW	 m_strName;
	int			 m_codePage;
};
