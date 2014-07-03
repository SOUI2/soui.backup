#pragma once

#include <unknown/obj-ref-i.h>

namespace SOUI
{
    struct IImgFrame
    {
        virtual BOOL GetSize(UINT *pWid,UINT *pHei)=0;
        virtual BOOL CopyPixels( 
            /* [unique][in] */ const RECT *prc,
            /* [in] */ UINT cbStride,
            /* [in] */ UINT cbBufferSize,
            /* [size_is][out] */ BYTE *pbBuffer) = 0;
    };

    struct IImgDecoder : public IObjRef
    {
        virtual int DecodeFromMemory(void *pBuf,size_t bufLen)=0;
        virtual int DecodeFromFile(LPCWSTR pszFileName)=0;
        virtual int DecodeFromFile(LPCSTR pszFileName)=0;
        virtual UINT GetFrameCount()=0;
        virtual IImgFrame * GetFrame(UINT iFrame)=0;
    };

    struct IImgDecoderFactory : public IObjRef
    {
        virtual BOOL CreateImgDecoder(IImgDecoder **ppImgDecoder)=0;
    };
}