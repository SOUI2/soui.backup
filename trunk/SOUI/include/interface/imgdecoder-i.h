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
        virtual int GetDelay() = 0;
    };

    struct IImgX : public IObjRef
    {
        virtual int LoadFromMemory(void *pBuf,size_t bufLen)=0;
        virtual int LoadFromFile(LPCWSTR pszFileName)=0;
        virtual int LoadFromFile(LPCSTR pszFileName)=0;
        virtual UINT GetFrameCount()=0;
        virtual IImgFrame * GetFrame(UINT iFrame)=0;
    };

    struct IImgDecoderFactory : public IObjRef
    {
        virtual BOOL IsAlphaPremultiple()=0;
        virtual void SetAlphaPremultiple(BOOL bPreMultiple)=0;
        virtual BOOL CreateImgX(IImgX **ppImgDecoder)=0;
    };
}