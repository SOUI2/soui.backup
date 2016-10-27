#pragma once
#include "interface\render-i.h"
#include <map>

class ImageProvider
{
public:
    static BOOL Insert(LPCWSTR pszImageId, LPCWSTR pszImagePath, const LPRECT lprcMargin = NULL);
    static BOOL Insert(LPCWSTR pszImageId, HBITMAP hImageHandle, const LPRECT lprcMargin = NULL);
    static BOOL Insert(LPCWSTR pszImageId, LPBYTE pData, size_t sizeLen, const LPRECT lprcMargin= NULL);
    static BOOL Update(LPCWSTR pszImageId, LPCWSTR pszImagePath, const LPRECT lprcMargin = NULL);
    static BOOL Update(LPCWSTR pszImageId, HBITMAP hImageHandle, const LPRECT lprcMargin = NULL);
    static BOOL Update(LPCWSTR pszImageId, LPBYTE pData, size_t sizeLen, const LPRECT lprcMargin= NULL);
    static void Remove(LPCWSTR pszImageId);
    static BOOL IsExist(LPCWSTR pszImageId);
    static SSkinImgFrame *   GetImage(LPCWSTR pszImageId);
};
