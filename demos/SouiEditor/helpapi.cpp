#include "stdafx.h"
#include "helpapi.h"

bool FileIsExist(const SStringT &filepath)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile(filepath, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		return false;
	}
	else {
		FindClose(hFind);
		return true;
	}
}
void PrintLassErrorMessage() {
	TCHAR* buffer;
	::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		0,
		(LPTSTR)&buffer,
		0,
		NULL);
	SMessageBox(NULL, buffer, NULL, 0);
	LocalFree(buffer);
}

SStringT GetFileExtname(const SStringT& filepath)
{
	wchar_t wcDirPath[MAX_PATH] = { 0 };
	wchar_t* wcExtension = PathFindExtensionW(filepath);
	SStringT strRet = L"";
	if (wcExtension)
		strRet = wcExtension;
	return strRet;
}

SStringT GetFilename(const SStringT& filepath)
{
	wchar_t wcDirPath[MAX_PATH] = { 0 };
	wchar_t* wcExtension = PathFindFileNameW(filepath);
	SStringT strRet = L"";
	if (wcExtension)
		strRet = wcExtension;
	return strRet;
}

//自定义排序函数  
bool SortSString(const SStringT &v1, const SStringT &v2)
{
	return v1 < v2;		//升序排列  
}

bool SortSStringNoCase(const SStringT &v1, const SStringT &v2)
{
	return v1.CompareNoCase(v2) < 0;		//升序排列  
}