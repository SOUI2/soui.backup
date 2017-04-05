#include "stdafx.h"
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>
#include "FilesHelp.h"



CFileHelp::CFileHelp(void)
{

}


CFileHelp::~CFileHelp(void)
{
}

BOOL CFileHelp::OpenFile(LPCWSTR lpstrFilter, HWND hwndOwner, vector<SStringT> &fileNames, bool IsMulti)
{
	DWORD dwFlag = IsMulti ? OFN_ALLOWMULTISELECT : 0;
	TCHAR szFileName[MAX_PATH * 101 + 1] = _T("");

	OPENFILENAME openfilename = { 0 };

	ZeroMemory(&openfilename, sizeof(OPENFILENAME));

	SStringT s_title;

	openfilename.lStructSize = sizeof(OPENFILENAME);
	openfilename.hwndOwner = hwndOwner;
	openfilename.hInstance = NULL;
	openfilename.lpstrFilter = lpstrFilter;
	openfilename.lpstrCustomFilter = NULL;
	openfilename.nMaxCustFilter = 0L;
	openfilename.nFilterIndex = 1L;
	openfilename.lpstrFile = szFileName;
	openfilename.nMaxFile = MAX_PATH * 101 + 1;
	openfilename.lpstrFileTitle = NULL;
	openfilename.nMaxFileTitle = 0;
	openfilename.lpstrInitialDir = NULL;
	openfilename.lpstrTitle = s_title;
	openfilename.nFileOffset = 0;
	openfilename.nFileExtension = 0;
	openfilename.lpstrDefExt = _T("*.*");
	openfilename.lCustData = 0;
	openfilename.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_READONLY | OFN_EXPLORER | dwFlag;

	// 弹出打开文件的对话框
	SStringT str;

	if (::GetOpenFileName(&openfilename))
	{
		LPTSTR p = szFileName;
		SStringT TempPath;
		if (*p != NULL)
		{
			TempPath = p;
			p += TempPath.GetLength() + 1;
		}

		if (*p == NULL)
		{
			//	TempPath = TempPath.Left(TempPath.ReverseFind(L'\\'));
			fileNames.push_back(TempPath);
		}


		while (*p != NULL)
		{
			SStringT str = p;

			p += str.GetLength() + 1;

			fileNames.push_back(TempPath + _T("\\") + str);
		}

		return TRUE;
	}
	else
	{
		return FALSE;
	}

}

void CFileHelp::SplitPathFileName(SStringT fileName, SStringT &szName, SStringT &szExt)
{
	TCHAR       p_szName[_MAX_FNAME];
	TCHAR       p_szExt[_MAX_EXT];
	_tsplitpath_s(fileName, NULL, 0, NULL, 0, p_szName, _MAX_FNAME, p_szExt, _MAX_EXT);
	_tcsupr_s(p_szExt, _MAX_EXT);
	szName = p_szName;
	szExt = p_szExt;
}

DWORD CFileHelp::GetFileSize(LPCTSTR fileName)
{
	HANDLE hFile = ::CreateFile(fileName, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
	DWORD dwSize = ::GetFileSize(hFile, 0);
	if (dwSize != INVALID_FILE_SIZE)
	{
		return dwSize;
	}
	else
		return 0;

	CloseHandle(hFile);

}

LPCTSTR CFileHelp::FileSizeToString(DWORD dwSize)
{
	TCHAR* strSize = new TCHAR[20];
	ZeroMemory(strSize, sizeof(TCHAR) * 20);
	_stprintf_s(strSize, 20, _T("%.2f M"), float(dwSize) / 1024 / 1024);
	return strSize;
}

SStringT CFileHelp::TimeToToleString(int time)
{
	SStringT sTime, sMin, sSec;
	if (time / 60<10)
	{
		sMin.Format(L"0%d", time / 60);
	}
	else
	{
		sMin.Format(L"%d", time / 60);
	}

	if (time % 60<10)
	{
		sSec.Format(L"0%d", time % 60);
	}
	else
	{
		sSec.Format(L"%d", time % 60);
	}

	sTime.Append(sMin);
	sTime.Append(L":");
	sTime.Append(sSec);
	return sTime;
}
BOOL CFileHelp::BrowseDir(SStringT &path, HWND hwndOwner, SStringT title)
{
	TCHAR szPathName[MAX_PATH];
	BROWSEINFO bInfo = { 0 };
	bInfo.hwndOwner = hwndOwner;//父窗口  
	bInfo.lpszTitle = title;
	bInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI/*包含一个编辑框 用户可以手动填写路径 对话框可以调整大小之类的..*/ |
		BIF_UAHINT/*带TIPS提示*/ | BIF_NONEWFOLDERBUTTON /*不带新建文件夹按钮*/;
	LPITEMIDLIST lpDlist;
	lpDlist = SHBrowseForFolder(&bInfo);
	if (lpDlist != NULL)//单击了确定按钮  
	{
		SHGetPathFromIDList(lpDlist, szPathName);
		path.Format(_T("%s"), szPathName);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
bool CFileHelp::FindFileExt(LPCTSTR pstrPath, LPCTSTR pstrExtFilter)
{
	if (!pstrPath || !pstrExtFilter)
	{
		return false;
	}

	TCHAR szExt[_MAX_EXT] = _T("");

	_tsplitpath_s(pstrPath, NULL, 0, NULL, 0, NULL, 0, szExt, _MAX_EXT);
	_tcslwr_s(szExt, _MAX_EXT);

	if (_tcslen(szExt))
	{
		_tcscat_s(szExt, _MAX_EXT, _T(";"));    // .mo不符合条件，由于会匹配到.mov，所以在后面加上【;】来判断是否完全匹配
		return NULL != _tcsstr(pstrExtFilter, szExt);
	}

	return false;
}
void CFileHelp::EnumerateFiles(vector<SStringT> &vctString, LPCTSTR p_strExtFilter)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(_T("*.*"), &fd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// 如果为目录
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (_tcscmp(fd.cFileName, _T(".")) && _tcscmp(fd.cFileName, _T("..")))
				{
					::SetCurrentDirectory(fd.cFileName);
					EnumerateFiles(vctString, p_strExtFilter);
					::SetCurrentDirectory(_T(".."));
				}
			}
			// 如果为文件
			else
			{
				SStringT strDir;
				TCHAR      lpDir[MAX_PATH];

				::GetCurrentDirectory(MAX_PATH, lpDir);
				strDir = lpDir;
				if (strDir.Right(1) != _T("\\"))
				{
					strDir += _T("\\");
				}
				strDir += fd.cFileName;
				if (CFileHelp::FindFileExt(strDir, p_strExtFilter))
					vctString.push_back(strDir);
			}
		} while (::FindNextFile(hFind, &fd));

		::FindClose(hFind);
	}

}
BOOL CFileHelp::CheckFileExist(SStringT pathFileName)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(pathFileName, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;
	else
		return TRUE;
}