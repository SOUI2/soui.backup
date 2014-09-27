/****************************************************************************************
* ///////////////////////////////////////////////////////////////////////////////////////
*	Original Filename: 	Log.cpp
*
*	History:
*	Created/Modified by				Date			Main Purpose/Changes
*	Souren M. Abeghyan				2001/05/25		Implementation of the CLog class.
*	
*	Comments:	
* \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
****************************************************************************************/
#include "stdafx.h"
#include "Log.h"
#include <time.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLog::CLog()
{
	InitializeCriticalSection(&cs);
}

CLog::~CLog()
{
	DeleteCriticalSection(&cs);
}



BOOL CLog::LogMessage(const TCHAR *szFolder, const TCHAR *szMsg, const TCHAR *szMsg1, long nNumber)
{
	EnterCriticalSection(&cs);
	time(&ltime);

	if((!_tcslen(szFolder)) || (!_tcslen(szMsg)))
		return FALSE;
	
	if(!GetWindowsDirectory(szLogFilePath, MAX_PATH))
	{
		LeaveCriticalSection(&cs);
		return FALSE;
	}
	
	if(szLogFilePath[0] != _T('\\'))
		_tcscat(szLogFilePath, _T("\\"));
	_tcscat(szLogFilePath, szFolder);

	m_f = _tfopen(szLogFilePath, _T("a"));
	if(m_f != NULL)				
	{
		newtime = localtime(&ltime);
		_tcsftime(szDT, 128,
					_T("%a, %d %b %Y %H:%M:%S"), newtime);
		
		if(szMsg1 != NULL)
			_stprintf(szMessage, _T("%s - %s.\t[%s]\t[%d]\n"), szDT, szMsg, szMsg1, nNumber);
		else
			_stprintf(szMessage, _T("%s - %s.\t[%d]\n"), szDT, szMsg, nNumber);

		int n = fwrite(szMessage, sizeof(TCHAR), _tcslen(szMessage), m_f);
		if(n != _tcslen(szMessage))
		{
			LeaveCriticalSection(&cs);
			fclose(m_f);
			return FALSE;
		}

		fclose(m_f);
		LeaveCriticalSection(&cs);
		return TRUE;
	}

	LeaveCriticalSection(&cs);
	return FALSE;
}



BOOL CLog::ClearLog(const TCHAR *szFolder)
{
	if(!_tcslen(szFolder))
		return FALSE;
	
	if(!GetWindowsDirectory(szLogFilePath, MAX_PATH))
		return FALSE;

	if(szLogFilePath[0] != _T('\\'))
		_tcscat(szLogFilePath, _T("\\"));
	_tcscat(szLogFilePath, szFolder);

	return 	DeleteFile(szLogFilePath);
}
