#include "stdafx.h"
#include "ImgProvider.h"
#include <commctrl.inl>
#include <Gdiplus.h>
#include "SApp.h"
#include "res.mgr\SSkinPool.h"
using namespace Gdiplus;

//////////////////////////////////////////////////////////////////////////
// helpers
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)  
{  
    UINT  num = 0;          // number of image encoders  
    UINT  size = 0;         // size of the image encoder array in bytes  

    ImageCodecInfo* pImageCodecInfo = NULL;  

    GetImageEncodersSize(&num, &size);  
    if(size == 0)  
        return -1;  // Failure  

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));  
    if(pImageCodecInfo == NULL)  
        return -1;  // Failure  

    GetImageEncoders(num, size, pImageCodecInfo);  

    for(UINT j = 0; j < num; ++j)  
    {  
        if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )  
        {  
            *pClsid = pImageCodecInfo[j].Clsid;  
            free(pImageCodecInfo);  
            return j;  // Success  
        }  
    }  

    free(pImageCodecInfo);  
    return -1;  // Failure  
}  

Bitmap* CreateBitmapFromHBITMAP(IN HBITMAP hBitmap)
{
    BITMAP bmp = { 0 };
    if ( 0 == GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&bmp) )
    {
        return NULL;
    }

    BYTE *piexlsSrc = NULL;
    LONG cbSize = bmp.bmWidthBytes * bmp.bmHeight;
    piexlsSrc = new BYTE[cbSize];

    BITMAPINFO bmpInfo = { 0 };
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth = bmp.bmWidth;
    bmpInfo.bmiHeader.biHeight = bmp.bmHeight;
    bmpInfo.bmiHeader.biPlanes = bmp.bmPlanes;
    bmpInfo.bmiHeader.biBitCount = bmp.bmBitsPixel;
    bmpInfo.bmiHeader.biCompression = BI_RGB;

    HDC hdcScreen = CreateDC(L"DISPLAY", NULL, NULL,NULL);
    LONG cbCopied = GetDIBits(hdcScreen, hBitmap, 0,
        bmp.bmHeight, piexlsSrc, &bmpInfo, DIB_RGB_COLORS);
    DeleteDC(hdcScreen);
    if ( 0 == cbCopied )
    {
        delete [] piexlsSrc;
        return NULL;
    }

    Bitmap *pBitmap = new Bitmap(bmp.bmWidth, bmp.bmHeight, PixelFormat32bppPARGB);

    BitmapData bitmapData;
    Rect rect(0, 0, bmp.bmWidth, bmp.bmHeight);
    if ( Ok != pBitmap->LockBits(&rect, 
        ImageLockModeRead,
        PixelFormat32bppPARGB, 
        &bitmapData) )
    {
        delete (pBitmap);
        return NULL;
    }

    BYTE *pixelsDest = (BYTE*)bitmapData.Scan0;
    int nLinesize = bmp.bmWidth * sizeof(UINT);
    int nHeight = bmp.bmHeight;

    for ( int y = 0; y < nHeight; y++ )
    {
        memcpy_s( (pixelsDest + y * nLinesize), nLinesize,
            (piexlsSrc + (nHeight - y - 1) * nLinesize), nLinesize);
    }

    if ( Ok != pBitmap->UnlockBits(&bitmapData) )
    {
        delete pBitmap;
    }

    delete [] piexlsSrc;
    return pBitmap;
}

BOOL GetBitmapData(LPBYTE &pImageContent, DWORD& dwSize, HBITMAP hBMP)
{
    // GET png class id
    CLSID clsidPng;
    if (GetEncoderClsid(L"image/png", &clsidPng) < 0)
    {
        return FALSE;
    }

    // Create global stream
    IStream * pStream = NULL;
    CreateStreamOnHGlobal(NULL, TRUE, &pStream);

    // Create a Bitmap object from a BMP file.  
    Bitmap * pBitmap = CreateBitmapFromHBITMAP(hBMP);
    if (pBitmap == NULL)
    {
        pStream->Release();
        return FALSE;
    }

    // Convert and save to png stream
    ULONG quality = 100;
    EncoderParameters encoderParameters;  
    encoderParameters.Count = 1;  
    encoderParameters.Parameter[0].Guid = EncoderQuality;  
    encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;  
    encoderParameters.Parameter[0].NumberOfValues = 1;  
    encoderParameters.Parameter[0].Value = &quality;  
    Status status = pBitmap->Save(pStream, &clsidPng);

    // get byte array
    ULONG ulBytesRead = 0;
    ULARGE_INTEGER ulnSize;
    LARGE_INTEGER lnOffset;
    lnOffset.QuadPart = 0;
    if (pStream->Seek(lnOffset, STREAM_SEEK_END, &ulnSize) == S_OK)
    {
        if (pStream->Seek(lnOffset, STREAM_SEEK_SET, NULL) == S_OK)
        {
            pImageContent = new BYTE[(UINT)ulnSize.QuadPart];
            pStream->Read(pImageContent, (ULONG)ulnSize.QuadPart, &ulBytesRead);
            dwSize = ulBytesRead;
        }
    }

    delete pBitmap;
    pStream->Release();
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// ImageProvider
BOOL ImageProvider::IsExist(LPCWSTR pszImageId)
{
    if (!pszImageId)
    {
        return FALSE;
    }

    SSkinPool *pBuiltinSkinPool = SSkinPoolMgr::getSingletonPtr()->GetBuiltinSkinPool();
    ISkinObj *pSkin=pBuiltinSkinPool->GetSkin(pszImageId);
    
    return (pSkin != NULL) && (pSkin->IsClass(SSkinImgFrame::GetClassName()));
}

BOOL ImageProvider::Insert(LPCWSTR pszImageId, LPCWSTR pszImagePath, const LPRECT lprcMargin/* = NULL*/)
{
    if (!pszImageId || !pszImagePath)
    {
        return FALSE;
    }

    if (IsExist(pszImageId))
    {
        // already exist
        return FALSE;
    }

    IBitmap * pImg=NULL;
    GETRENDERFACTORY->CreateBitmap(&pImg);
    pImg->LoadFromFile(pszImagePath);
    
    // load skin
    CRect rcMargin;
    if (lprcMargin)
        rcMargin = lprcMargin;

    SSkinImgFrame * pSkin = new SSkinImgFrame();
    pSkin->SetImage(pImg);
    pSkin->SetMargin(rcMargin);
    
    SSkinPool *pBuiltinSkinPool = SSkinPoolMgr::getSingletonPtr()->GetBuiltinSkinPool();
    pBuiltinSkinPool->AddKeyObject(pszImageId, pSkin);

    pImg->Release();
    return TRUE;
}

BOOL ImageProvider::Insert(LPCWSTR pszImageId, LPBYTE pData, size_t sizeLen, const LPRECT lprcMargin/* = NULL*/)
{
    if (!pszImageId || !pData || sizeLen == 0)
    {
        return FALSE;
    }

    if (IsExist(pszImageId))
    {
        // already exist
        return FALSE;
    }

    // prepare image
    IBitmap * pImg=NULL;
    GETRENDERFACTORY->CreateBitmap(&pImg);
    pImg->LoadFromMemory(pData, sizeLen);

    // load skin
    CRect rcMargin;
    if (lprcMargin)
        rcMargin = lprcMargin;

    SSkinImgFrame * pSkin = new SSkinImgFrame();
    pSkin->SetImage(pImg);
    pSkin->SetMargin(rcMargin);
    
    SSkinPool *pBuiltinSkinPool = SSkinPoolMgr::getSingletonPtr()->GetBuiltinSkinPool();
    pBuiltinSkinPool->AddKeyObject(pszImageId, pSkin);

    pImg->Release();
    return TRUE;
}

BOOL ImageProvider::Insert(LPCWSTR pszImageId, HBITMAP hImageHandle, const LPRECT lprcMargin/* = NULL*/)
{
    if (IsExist(pszImageId))
    {
        // already exist
        return FALSE;
    }

    BOOL    bRet = FALSE;
    DWORD   dwTotalSize = 0;
    LPBYTE  pBytes = NULL;
    if (GetBitmapData(pBytes, dwTotalSize, hImageHandle))
    {
        bRet = Insert(pszImageId, pBytes, dwTotalSize);
        delete []pBytes;
    }

    return TRUE;
}

SSkinImgFrame * ImageProvider::GetImage(LPCWSTR pszImageId)
{
    if (!pszImageId)
    {
        return NULL;
    }

    SSkinPool * pBuiltinSkinPool = SSkinPoolMgr::getSingletonPtr()->GetBuiltinSkinPool();
    ISkinObj  * pSkin = pBuiltinSkinPool->GetSkin(pszImageId);

    if (pSkin == NULL)
    {
        pSkin = SSkinPoolMgr::getSingletonPtr()->GetSkin(pszImageId);
    }

    // NOT found
    return static_cast<SSkinImgFrame*>(pSkin);
}

BOOL ImageProvider::Update(LPCWSTR pszImageId, LPCWSTR pszImagePath, const LPRECT lprcMargin/* = NULL*/)
{
    if (!pszImageId || !pszImagePath)
    {
        return FALSE;
    }

    Remove(pszImageId);

    return Insert(pszImageId, pszImagePath, lprcMargin);
}

BOOL ImageProvider::Update(LPCWSTR pszImageId, HBITMAP hImageHandle, const LPRECT lprcMargin/* = NULL*/)
{
    if (!pszImageId)
    {
        return FALSE;
    }

    Remove(pszImageId);

    return Insert(pszImageId, hImageHandle, lprcMargin);
}

BOOL ImageProvider::Update(LPCWSTR pszImageId, LPBYTE pData, size_t sizeLen, const LPRECT lprcMargin/* = NULL*/)
{
    if (!pszImageId || !pData || sizeLen == 0)
    {
        return FALSE;
    }

    Remove(pszImageId);

    return Insert(pszImageId, pData, sizeLen, lprcMargin);
}

void ImageProvider::Remove(LPCWSTR pszImageId)
{
    if (!pszImageId)
    {
        return;
    }

    SSkinPool *pBuiltinSkinPool = SSkinPoolMgr::getSingletonPtr()->GetBuiltinSkinPool();
    ISkinObj *pSkin=pBuiltinSkinPool->GetSkin(pszImageId);

    if ((pSkin != NULL) && (pSkin->IsClass(SSkinImgFrame::GetClassName())))
    {
        pBuiltinSkinPool->RemoveKeyObject(pszImageId);
    }
}
