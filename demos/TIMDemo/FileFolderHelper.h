#ifndef __WIN_DIRECTORY_HPP_INCLUDE_20160125__
#define __WIN_DIRECTORY_HPP_INCLUDE_20160125__

//#include "stdafx.h"
#include <string>

namespace FileFolderHelper
{
	//bool GetDirFormatSize(LPCTSTR sDir, LPTSTR dataBlock, size_t dataSize);
	//新建目录    
	bool CreateDir(const wchar_t* lpDirPath);
	//删除目录 及 其子目录
	bool DeleteDir(const wchar_t* lpDirPath);
	//判断是否 是文件夹
	bool IsDir(const wchar_t* sDirPath);
	//计算文件夹的大小
	unsigned __int64 CalcFolderSize(const wchar_t* lpFolderPath);

	struct WinFolderData
	{
		unsigned __int64				ulFileSize;
		unsigned int					uFileCount;
		unsigned int					uFolderCount;
		WinFolderData()
		{
			ulFileSize = 0;
			uFileCount = 0;
			uFolderCount = 0;
		}
	};
	//计算文件夹 的信息
	bool CalcFolderInfo(const wchar_t* lpFolderPath, WinFolderData& info);
	unsigned __int64 CalcFileSize(const wchar_t* lpFilePath) ;
	
	std::wstring FileSizeToStr(unsigned __int64 ulFileSize);
	std::wstring FileSizeToBSize(unsigned __int64 ulFileSize);
	
	
	//bool SlipStringByChar(LPCTSTR lpStr, TCHAR Char);
	std::wstring GetFileExt(const wchar_t* lpFileName);

	// 获取文件的当前目录 路径 就是 去掉文件名 
	std::wstring GetFileCurDir(const wchar_t* lpFileName);

	//检测文件 有效  就是 不能有 */\||之类的 字符
	bool IsFileNameValid(const wchar_t* lpFileName);
}//end namespace

//////////////////////////////////////////////////////////////////////////
#endif // __WIN_DIRECTORY_HPP_INCLUDE_20160125__

