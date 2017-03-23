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