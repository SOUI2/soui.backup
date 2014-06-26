#pragma once

#ifndef IMGDECODER_API 
#define IMGDECODER_API
#endif

#ifndef OR_API
#define OR_API IMGDECODER_API
#endif

#include <unknown/obj-ref-i.h>

namespace SOUI
{
    struct IMGDECODER_API IImgFrame
    {
        virtual BOOL GetSize(UINT *pWid,UINT *pHei)=0;
        virtual BOOL CopyPixels( 
            /* [unique][in] */ const RECT *prc,
            /* [in] */ UINT cbStride,
            /* [in] */ UINT cbBufferSize,
            /* [size_is][out] */ BYTE *pbBuffer) = 0;
    };

    struct IMGDECODER_API IImgDecoder : public IObjRef
    {
        virtual int DecodeFromMemory(void *pBuf,size_t bufLen)=0;
        virtual int DecodeFromFile(LPCWSTR pszFileName)=0;
        virtual int DecodeFromFile(LPCSTR pszFileName)=0;
        virtual UINT GetFrameCount()=0;
        virtual IImgFrame * GetFrame(UINT iFrame)=0;
    };

    struct IMGDECODER_API IImgDecoderFactory : public IObjRef
    {
        virtual BOOL CreateImgDecoder(IImgDecoder **ppImgDecoder)=0;
    };
}