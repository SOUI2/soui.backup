#ifndef __DIALOG_H___
#define __DIALOG_H___
#pragma once
#include <commdlg.h>

class EFileDialog
{
public:
	EFileDialog(bool bOpenDialog, 
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT ,
		LPCTSTR lpszFilter = NULL,
		HWND hWndParent = NULL)
	{
		m_bOpenFileDialog = bOpenDialog;

		memset(&m_ofn, 0, sizeof(m_ofn)); // initialize structure to 0/NULL
		m_szFileName[0] = '\0';
		m_szFileTitle[0] = '\0';

		
		m_ofn.lStructSize = sizeof(m_ofn);
		m_ofn.lpstrFile = m_szFileName;
		m_ofn.nMaxFile = _MAX_PATH;
		m_ofn.lpstrDefExt = lpszDefExt;
		m_ofn.lpstrFileTitle = (LPTSTR)m_szFileTitle;
		m_ofn.nMaxFileTitle = _MAX_FNAME;
		m_ofn.Flags = dwFlags | OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLESIZING| OFN_NOCHANGEDIR;
		m_ofn.lpstrFilter = lpszFilter;
		m_ofn.hwndOwner = hWndParent;

		// setup initial file name
		if(lpszFileName != NULL)
			_tcscpy_s(m_szFileName, _countof(m_szFileName), lpszFileName);
	}

	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		if(m_ofn.hwndOwner == NULL)   // set only if not specified before
			m_ofn.hwndOwner = hWndParent;

		if(m_bOpenFileDialog)
			return ::GetOpenFileName(&m_ofn);
		else
			return ::GetSaveFileName(&m_ofn);
	}

	LPCTSTR GetFilePath()
	{
		return m_szFileName;
	}
protected:
	OPENFILENAME				m_ofn;
	bool								m_bOpenFileDialog;					// TRUE for file open, FALSE for file save
	TCHAR							m_szFileTitle[_MAX_FNAME];		// contains file title after return
	TCHAR							m_szFileName[_MAX_PATH];		// contains full path name after return

};

#endif