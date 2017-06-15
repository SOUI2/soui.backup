#pragma once
#include "SLogAdapter.h"

class CAppLogParse : public TObjRefImpl<ILogParse>
{
public:
	virtual BOOL ParseLine(LPCWSTR pszLine,SLogInfo **ppLogInfo) const;


	virtual int GetLevels() const;

	virtual void GetLevelText(wchar_t szLevels[][MAX_LEVEL_LENGTH]) const;

	virtual SStringW GetName() const;

	virtual bool IsFieldValid(Field field) const;

	virtual int GetCodePage() const;

	virtual BOOL TestLogBuffer(LPCSTR pszBuf, int nLength);

};

class CLogcatParse : public TObjRefImpl<ILogParse>
{
public:
	virtual BOOL ParseLine(LPCWSTR pszLine,SLogInfo **ppLogInfo) const;

	virtual int GetLevels() const;

	virtual void GetLevelText(wchar_t szLevels[][MAX_LEVEL_LENGTH]) const;

	virtual SStringW GetName() const;

	virtual bool IsFieldValid(Field field) const;

	virtual int GetCodePage() const;

	virtual BOOL TestLogBuffer(LPCSTR pszBuf, int nLength);

};

class CSouiLogParse: public TObjRefImpl<ILogParse>
{
	virtual BOOL ParseLine(LPCWSTR pszLine,SLogInfo **ppLogInfo) const;

	virtual int GetLevels() const;

	virtual void GetLevelText(wchar_t szLevels[][MAX_LEVEL_LENGTH]) const;

	virtual SStringW GetName() const;

	virtual bool IsFieldValid(Field field) const;

	virtual int GetCodePage() const;

	virtual BOOL TestLogBuffer(LPCSTR pszBuf, int nLength);

};

class CParseFactory : public TObjRefImpl<IParserFactory>
{
public:

	virtual ILogParse * CreateLogParser(int iParser) const;

	virtual int GetLogParserCount() const;

};