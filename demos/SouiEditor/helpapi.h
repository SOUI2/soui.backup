#pragma once

void PrintLassErrorMessage();
bool FileIsExist(const SStringT &filepath);
SStringT GetFileExtname(const SStringT& filepath);
SStringT GetFilename(const SStringT& filepath);

bool SortSString(const SStringT &v1, const SStringT &v2);
bool SortSStringNoCase(const SStringT &v1, const SStringT &v2);