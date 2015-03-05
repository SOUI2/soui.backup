
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "imgdecoder-png.h"
#include "decoder-apng.h"

namespace SOUI
{
    //////////////////////////////////////////////////////////////////////////
    //  SImgFrame_PNG
    SImgFrame_PNG::SImgFrame_PNG()
        :m_pdata(NULL)
        ,m_nWid(0)
        ,m_nHei(0)
        ,m_nFrameDelay(0)
    {

    }


    void SImgFrame_PNG::Attach( const BYTE * pdata,int nWid,int nHei,int nDelay )
    {
        m_pdata=(pdata);
        m_nWid=(nWid);
        m_nHei=(nHei);
        m_nFrameDelay=(nDelay);
    }

    BOOL SImgFrame_PNG::GetSize( UINT *pWid,UINT *pHei )
    {
        if(!m_pdata) return FALSE;
        *pWid = m_nWid;
        *pHei = m_nHei;
        return TRUE;
    }

    BOOL SImgFrame_PNG::CopyPixels( /* [unique][in] */ const RECT *prc, /* [in] */ UINT cbStride, /* [in] */ UINT cbBufferSize, /* [size_is][out] */ BYTE *pbBuffer )
    {
        if(!m_pdata || cbBufferSize != m_nHei * m_nWid *4) return FALSE;
        memcpy(pbBuffer,m_pdata,cbBufferSize);
        return TRUE;
    }

    
    //////////////////////////////////////////////////////////////////////////
    // SImgX_PNG
    int SImgX_PNG::LoadFromMemory( void *pBuf,size_t bufLen )
    {
        APNGDATA * pdata =LoadAPNG_from_memory((char*)pBuf,bufLen);
        return _DoDecode(pdata);
    }

    int SImgX_PNG::LoadFromFile( LPCWSTR pszFileName )
    {
        APNGDATA * pdata =LoadAPNG_from_file(pszFileName);
        return _DoDecode(pdata);
    }

    int SImgX_PNG::LoadFromFile( LPCSTR pszFileName )
    {
        wchar_t wszFileName[MAX_PATH+1];
        MultiByteToWideChar(CP_ACP,0,pszFileName,-1,wszFileName,MAX_PATH);
        if(GetLastError()==ERROR_INSUFFICIENT_BUFFER) return 0;
        return LoadFromFile(wszFileName);
    }

    SImgX_PNG::SImgX_PNG( BOOL bPremultiplied )
        :m_bPremultiplied(bPremultiplied)
        ,m_pImgArray(NULL)
        ,m_pngData(NULL)
    {

    }

    SImgX_PNG::~SImgX_PNG( void )
    {
        if(m_pImgArray) delete []m_pImgArray;
        m_pImgArray = NULL;
        if(m_pngData) APNG_Destroy(m_pngData);
    }

    int SImgX_PNG::_DoDecode(APNGDATA *pData)
    {
        if(!pData) return 0;
        m_pngData = pData;

        int nWid = m_pngData->nWid;
        int nHei = m_pngData->nHei;

        //swap rgba to bgra and do premultiply
        BYTE *p=m_pngData->pdata;
        int pixel_count = nWid * nHei * m_pngData->nFrames;
        for (int i=0; i < pixel_count; ++i) {
            BYTE a = p[3];
            BYTE t = p[0];
            if (a) 
            {
                p[0] = (p[2] *a)/255;
                p[1] = (p[1] * a)/255;
                p[2] =  (t   * a)/255;
            }else
            {
                memset(p,0,4);
            }
            p += 4;
        }

        p=m_pngData->pdata;
        m_pImgArray = new SImgFrame_PNG[m_pngData->nFrames];
        for(int i=0;i<m_pngData->nFrames;i++)
        {
            m_pImgArray[i].Attach(p,nWid,nHei,m_pngData->pDelay?m_pngData->pDelay[i]:0);
            p += nWid*nHei*4;
        }
        return m_pngData->nFrames;
    }

    UINT SImgX_PNG::GetFrameCount()
    {
        return m_pngData?m_pngData->nFrames:0;
    }

    //////////////////////////////////////////////////////////////////////////
    //  SImgDecoderFactory_PNG

    SImgDecoderFactory_PNG::SImgDecoderFactory_PNG()
    {

    }

    SImgDecoderFactory_PNG::~SImgDecoderFactory_PNG()
    {

    }

    BOOL SImgDecoderFactory_PNG::CreateImgX( IImgX **ppImgDecoder )
    {
        *ppImgDecoder = new SImgX_PNG(TRUE);
        return TRUE;
    }

    HRESULT SImgDecoderFactory_PNG::SaveImage(IBitmap *pImg, LPCWSTR pszFileName, const LPVOID pFormat)
    {
        return E_FAIL;
    }
    
    LPCWSTR SImgDecoderFactory_PNG::GetDescription() const
    {
        return DESC_IMGDECODER;
    }

    //////////////////////////////////////////////////////////////////////////
    BOOL IMGDECODOR_PNG::SCreateInstance( IObjRef **pImgDecoderFactory )
    {
        *pImgDecoderFactory = new SImgDecoderFactory_PNG();
        return TRUE;
    }

}//end of namespace SOUI

