#include "stdafx.h"
#include "utils.h"
#include <shlobj.h>

namespace SOUI
{
    //
    // xml helpers
    //

    SStringW XMLEscape(SStringW& strXML)
    {
        strXML.Replace(L"&", L"&amp;");
        strXML.Replace(L"<", L"&lt;");
        strXML.Replace(L">", L"&gt;");
        strXML.Replace(L"'", L"&apos;");
        strXML.Replace(L"\"", L"&quot;");
        return strXML;
    }


    SStringW GenGuid()
    {
        SStringW sGUID;
        GUID guid;

        if (S_OK == ::CoCreateGuid(&guid))
        {
            sGUID.Format(_T("%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x"),
                guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2],
                guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
        }

        return sGUID;
    }

    int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
    {
        UINT  num = 0;          // number of image encoders  
        UINT  size = 0;         // size of the image encoder array in bytes  

        Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

        Gdiplus::GetImageEncodersSize(&num, &size);
        if (size == 0)
            return -1;  // Failure  

        pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
        if (pImageCodecInfo == NULL)
            return -1;  // Failure  

        Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

        for (UINT j = 0; j < num; ++j)
        {
            if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
            {
                *pClsid = pImageCodecInfo[j].Clsid;
                free(pImageCodecInfo);
                return j;  // Success  
            }
        }

        free(pImageCodecInfo);
        return -1;  // Failure  
    }

    int GetCodecInfo(const WCHAR* format, Gdiplus::ImageCodecInfo* p)
    {
        UINT  num = 0;          // number of image encoders  
        UINT  size = 0;         // size of the image encoder array in bytes  

        Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

        Gdiplus::GetImageEncodersSize(&num, &size);
        if (size == 0)
            return -1;  // Failure  

        pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
        if (pImageCodecInfo == NULL)
            return -1;  // Failure  

        GetImageEncoders(num, size, pImageCodecInfo);

        for (UINT n = 0; n < num; ++n)
        {
            if (wcscmp(pImageCodecInfo[n].MimeType, format) == 0)
            {
                *p = pImageCodecInfo[n];
                free(pImageCodecInfo);
                return n;  // Success  
            }
        }

        free(pImageCodecInfo);
        return -1;
    }

    double GetZoomRatio(SIZE normalSize, SIZE maxSize)
    {
        double fXRatio = 1.0f;
        double fYRatio = 1.0f;

        if (normalSize.cx > maxSize.cx)
        {
            fXRatio = (double)maxSize.cx / (double)normalSize.cx;
        }

        if (normalSize.cy > maxSize.cy)
        {
            fYRatio = (double)maxSize.cy / (double)normalSize.cy;
        }

        return fXRatio > fYRatio ? fYRatio : fXRatio;
    }

    SStringW GetTempPath()
    {
        WCHAR szTempPath[MAX_PATH + 1];
        ::GetTempPathW(MAX_PATH + 1, szTempPath);

        SStringW path;
        path.Format(L"%sImClient\\", szTempPath);

        SHCreateDirectoryEx(NULL, path, NULL);

        return path;
    }
}
