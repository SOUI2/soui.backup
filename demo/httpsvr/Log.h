/****************************************************************************************
* ///////////////////////////////////////////////////////////////////////////////////////
*	Original Filename: 	Log.h
*
*	History:
*	Created/Modified by				Date			Main Purpose/Changes
*	Souren M. Abeghyan				2001/05/25		Interface for the CLog class
*	
*	Comments:	
* \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
****************************************************************************************/
#if !defined(AFX_LOG_H__82043CA7_5940_4F7E_A316_7F91096B2008__INCLUDED_)
#define AFX_LOG_H__82043CA7_5940_4F7E_A316_7F91096B2008__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

using namespace std;

#define MAX_MSG_SIZE	1024

class CLog  
{
public:
	BOOL ClearLog(const TCHAR*);
	BOOL LogMessage(const TCHAR*, const TCHAR*, const TCHAR* = NULL, long = NULL);
	CLog();
	virtual ~CLog();
private:
	FILE *m_f;
	TCHAR szLogFilePath[MAX_PATH];
	TCHAR szMessage[MAX_MSG_SIZE];
	TCHAR szDT[128];
	struct tm *newtime;
	time_t ltime;
	CRITICAL_SECTION cs;
};

#endif // !defined(AFX_LOG_H__82043CA7_5940_4F7E_A316_7F91096B2008__INCLUDED_)
