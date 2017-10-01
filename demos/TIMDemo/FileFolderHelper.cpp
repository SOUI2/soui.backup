#include "stdafx.h"
#include <algorithm>
#include <stdio.h>
#include <Shlwapi.h>
#include <ShellAPI.h>
#include <ShlObj.h>
#include "FileFolderHelper.h"
#include <tchar.h>
#include <list>
#include <regex>
#define MAX_PATH 260

namespace FileFolderHelper
{
	bool CreateDir(const wchar_t* lpDirPath)
	{
		wchar_t lpDir[MAX_PATH] = {0};
		lstrcpyn(lpDir, lpDirPath, MAX_PATH);
		PathRemoveFileSpec(lpDir);
		PathUnquoteSpaces(lpDir);	//去除路径中的首尾空格
		if (!PathFileExists(lpDir))
		{
			return 0 == ::SHCreateDirectory(NULL, lpDir);
		}

		return true;
	}

	bool DeleteDir(const wchar_t* lpDirPath)
	{
		SHFILEOPSTRUCT info;
		ZeroMemory(&info, sizeof(SHFILEOPSTRUCT));
		info.fFlags |= FOF_SILENT;			//不显示进度
		info.fFlags |= FOF_NOERRORUI;			//不显示错误报告
		info.fFlags |= FOF_NOCONFIRMATION;		//直接删除  不显示确认
		info.fFlags &= ~FOF_ALLOWUNDO;			//直接删除 不放 回收站
		info.hNameMappings = NULL;
		info.hwnd = NULL;
		info.lpszProgressTitle = NULL;


		info.wFunc = FO_DELETE;
		info.pFrom = lpDirPath;
		info.pTo = NULL;

		return 0 == SHFileOperation(&info);
	}

	bool IsDir(const wchar_t* lpDirPath)
	{
		DWORD dwAttribute = GetFileAttributes(lpDirPath);
		if((DWORD)-1 == dwAttribute)
			return false;

		if (FILE_ATTRIBUTE_DIRECTORY == (FILE_ATTRIBUTE_DIRECTORY & dwAttribute)) 
		{
			return true;
		}
		return false;
	}

	unsigned __int64 CalcFolderSize(const wchar_t* lpFolderPath)
	{
		unsigned __int64 ulFolderSize = 0;
		std::list<std::wstring> listFolder;
		listFolder.push_back(lpFolderPath);

		while (listFolder.size() > 0 )
		{
			//从队列首取出路径
			std::wstring sFolder = listFolder.front();
			listFolder.pop_front();

			std::wstring sFilePath = sFolder + _T("\\*");

			WIN32_FIND_DATA fd;
			HANDLE hFind = FindFirstFile(sFilePath.c_str(), &fd);
			if( INVALID_HANDLE_VALUE == hFind)
			{
				continue;
			}

			//枚举文件夹内的 文件信息
			do
			{
				std::wstring strName = fd.cFileName;
				if( _T(".") == strName || _T("..") == strName)
				{
					continue;
				}

				if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
				{
					//如果是文件夹 就先入列   
					listFolder.push_back(sFolder + _T("\\") + fd.cFileName);
					continue;
				}

				ULARGE_INTEGER liSize;
				liSize.LowPart = fd.nFileSizeLow;
				liSize.HighPart = fd.nFileSizeHigh;
				ulFolderSize += liSize.QuadPart;	

			}while( FindNextFile(hFind,&fd));		//查找下一个

			FindClose(hFind);
		}
		return ulFolderSize;
	}

	bool CalcFolderInfo(const wchar_t* lpFolderPath, WinFolderData& info)
	{
		std::list<std::wstring> listFolder;
		listFolder.push_back(lpFolderPath);

		while (listFolder.size() > 0 )
		{
			//从队列首取出路径
			std::wstring sFolder = listFolder.front();
			listFolder.pop_front();

			std::wstring sFilePath = sFolder + _T("\\*");

			WIN32_FIND_DATA fd;
			HANDLE hFind = FindFirstFile(sFilePath.c_str(), &fd);
			if( INVALID_HANDLE_VALUE == hFind)
			{
				continue;
			}

			//枚举文件夹内的 文件信息
			do
			{
				std::wstring strName = fd.cFileName;
				if( _T(".") == strName || _T("..") == strName)
				{
					continue;
				}

				if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
				{
					//如果是文件夹 就先入列   
					listFolder.push_back(sFolder + _T("\\") + fd.cFileName);
					++info.uFolderCount;
					continue;
				}

				ULARGE_INTEGER liSize;
				liSize.LowPart = fd.nFileSizeLow;
				liSize.HighPart = fd.nFileSizeHigh;
				info.ulFileSize += liSize.QuadPart;
				++info.uFileCount;

			}while( FindNextFile(hFind,&fd));		//查找下一个

			FindClose(hFind);
		}
		return true;
	}
	
	unsigned __int64 CalcFileSize(const wchar_t* lpFilePath) 
	{
		ULARGE_INTEGER liSize;
		liSize.QuadPart = 0;

		WIN32_FIND_DATA Find32;
		HANDLE hFind = FindFirstFile(lpFilePath, &Find32);
		if (INVALID_HANDLE_VALUE != hFind) 
		{
			liSize.LowPart = Find32.nFileSizeLow;
			liSize.HighPart = Find32.nFileSizeHigh;
			FindClose(hFind);
		}
		return liSize.QuadPart;
	}

	static wchar_t* sUnit[] = {_T("字节"), _T("KB"), _T("MB"), _T("GB")};
	std::wstring FileSizeToStr(unsigned __int64 ulFileSize)
	{
#if 0
		//这个方法 有局限性  没有四舍五入 也只有3位数字  
		TCHAR       szFileLen [64] = {0};
		StrFormatByteSize64(fileSize, szFileLen, 64);
		return CString(szFileLen);
#else			
		if(ulFileSize < 0) 
			return _T("未知");
		else if(0 == ulFileSize)
			return _T("0 字节");

		unsigned int nIndex = 0;

		double dSize = static_cast<double>(ulFileSize);
		while (dSize >= 1024.0)
		{
			dSize /= 1024.0;
			nIndex++;
		}

		wchar_t lpBuf[100] = {0};
		if(0 == nIndex)
			swprintf_s(lpBuf, 100, _T("%d %s"), (int)ulFileSize, sUnit[0]);
		else
			swprintf_s(lpBuf, 100, _T("%.2f %s"), dSize, sUnit[nIndex]);
		return std::wstring(lpBuf);
#endif
		
	}

	std::wstring FileSizeToBSize(unsigned __int64 ulFileSize)
	{		
		//CString sSizeText;
		//int len = _vscprintf( pszFormat, args ); // _vscprintf doesn't count terminating '\0'
		//*ppszDst = (char*)soui_mem_wrapper::SouiMalloc(len+1);
		////vsprintf(*ppszDst, pszFormat, args);
		//vsprintf_s(*ppszDst, len + 1, pszFormat, args);
		wchar_t lpBuf[MAX_PATH] = {0};
		swprintf_s(lpBuf, MAX_PATH, _T("%I64d"), ulFileSize);
		//sSizeText.Format(_T("%I64d"), ulFileSize);
		std::wstring szSizeText = lpBuf;
		int nLeng = szSizeText.length();
		for (int i=nLeng-3; i>0; i-=3)
		{
			szSizeText.insert(i, _T(","));
		}
		szSizeText.append(_T("字节"));
		return szSizeText;
	}


	bool IsFileNameValid(const wchar_t* lpFileName) 
	{
		/*CAtlRegExp<> RegExp;
		CAtlREMatchContext<> RegMatch;*/
		//LPCTSTR sRegStr = _T("[\\\\*:/?\\<\\>\\|\"]");
		//REParseError status = RegExp.Parse(sRegStr);
		//if (REPARSE_ERROR_OK != status)
		//{
		//	return false;
		//}

		//// 包含上面字符的字符串不能做文件名
		//if (RegExp.Match(lpFileName, &RegMatch)) 
		//{
		//	return false;
		//}
	/*	std::wregex regEx(L"^[^/*:?\\<\\>\\|\\\"\\\\]+$");
		if(std::regex_match(lpFileName, regEx))
			return false;*/
		return true;
	}

	std::wstring GetFileExt(const wchar_t* lpFileName)
	{
		//LPCTSTR lpExt = _tcsrchr(lpFileName, '.');
		LPCTSTR lpExt = PathFindExtension(lpFileName);
		if(NULL == lpExt)
			return _T("");

		std::wstring sExt(lpExt+1);
		return sExt;
	}

	std::wstring GetFileCurDir(const wchar_t* lpFileName)
	{
		std::wstring sFilePath(lpFileName);
		if(sFilePath.at(sFilePath.length()-1) == '\\')
		{
			sFilePath.pop_back();
		}
		//sFilePath.TrimRight('\\');
		//	sFilePath.
		int nPos = sFilePath.rfind('\\');
		if(-1 != nPos)
		{
			sFilePath = sFilePath.substr(0, nPos);
		}
		
		return sFilePath;
	}

}// end namespace