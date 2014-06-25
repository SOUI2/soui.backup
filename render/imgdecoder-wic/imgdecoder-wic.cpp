// imgdecoder-wic.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "imgdecoder-wic.h"

#pragma  comment(lib,"windowscodecs.lib")

namespace SOUI
{


    //////////////////////////////////////////////////////////////////////////
    // SImgFrame_WIC
    SImgFrame_WIC::SImgFrame_WIC( IWICBitmapSource *pFrame ):m_pFrame(pFrame)
    {

    }
    
    void SImgFrame_WIC::SetWICBitmpaSource( IWICBitmapSource *pFrame )
    {
        m_pFrame=pFrame;
    }

    BOOL SImgFrame_WIC::GetSize( UINT *pWid,UINT *pHei )
    {
        ASSERT(m_pFrame);
        return S_OK == m_pFrame->GetSize(pWid,pHei);
    }

    BOOL SImgFrame_WIC::CopyPixels(const RECT *prc, UINT cbStride, UINT cbBufferSize, BYTE *pbBuffer )
    {
        ASSERT(m_pFrame);
        if(!prc)
        {
            return S_OK==m_pFrame->CopyPixels(NULL,cbStride,cbBufferSize,pbBuffer);
        }
        else
        {
            WICRect rc={prc->left,prc->top,prc->right-prc->left,prc->bottom-prc->top};
            return S_OK==m_pFrame->CopyPixels(&rc,cbStride,cbBufferSize,pbBuffer);
        }
    }



    //////////////////////////////////////////////////////////////////////////

    SImgDecoder_WIC::SImgDecoder_WIC(void):m_pImgArray(NULL),m_uImgCount(0)
    {
    }

    SImgDecoder_WIC::~SImgDecoder_WIC(void)
    {
        if(m_pImgArray) delete []m_pImgArray;
        m_pImgArray = NULL;
    }

    int SImgDecoder_WIC::DecodeFromMemory(void *pBuf,size_t bufLen )
    {
        ASSERT(m_pImgArray == NULL);

        IWICImagingFactory*    factory    = SImgDecoderFactory::s_wicImgFactory;
        CAutoRefPtr<IWICBitmapDecoder>     decoder;
        CAutoRefPtr<IWICStream> stream ;

        if(FAILED(factory->CreateStream(&stream))) return 0;

        if(FAILED(stream->InitializeFromMemory((BYTE*)pBuf,bufLen))) return 0;

        if(FAILED(factory->CreateDecoderFromStream(stream,NULL,WICDecodeMetadataCacheOnDemand,&decoder))) return 0;

        return _DoDecode(decoder);
    }

    int SImgDecoder_WIC::DecodeFromFile( LPCWSTR pszFileName )
    {
        ASSERT(m_pImgArray == NULL);
        IWICImagingFactory*    factory    = SImgDecoderFactory::s_wicImgFactory;

        CAutoRefPtr<IWICBitmapDecoder>     decoder;

        HRESULT hr = factory->CreateDecoderFromFilename(
            pszFileName,
            NULL,
            GENERIC_READ,
            WICDecodeMetadataCacheOnLoad,
            &decoder
            );

        if(FAILED(hr)) return 0;

        return _DoDecode(decoder);
    }

    int SImgDecoder_WIC::DecodeFromFile( LPCSTR pszFileName )
    {
        wchar_t wszFileName[MAX_PATH+1];
        MultiByteToWideChar(CP_ACP,0,pszFileName,-1,wszFileName,MAX_PATH);
        if(GetLastError()==ERROR_INSUFFICIENT_BUFFER) return 0;
        return DecodeFromFile(wszFileName);
    }

    int SImgDecoder_WIC::_DoDecode( IWICBitmapDecoder * pDecoder )
    {
        ASSERT(m_uImgCount == 0);
        
        IWICImagingFactory*    factory    = SImgDecoderFactory::s_wicImgFactory;
        CAutoRefPtr<IWICFormatConverter> converter;
        if(FAILED(factory->CreateFormatConverter(&converter))) 
            return 0;

        if(FAILED(pDecoder->GetFrameCount(&m_uImgCount)))  
            return 0;

        m_pImgArray = new SImgFrame_WIC[m_uImgCount];
        for(UINT i = 0; i< m_uImgCount ;i++)
        {
            CAutoRefPtr<IWICBitmapFrameDecode> frame;
            if(SUCCEEDED(pDecoder->GetFrame(i,&frame)))
            {
                converter->Initialize(frame,
                    GUID_WICPixelFormat32bppPBGRA,
                    WICBitmapDitherTypeNone,NULL,
                    0.f,WICBitmapPaletteTypeCustom);
                CAutoRefPtr<IWICBitmapSource> bmp;
                converter->QueryInterface(IID_PPV_ARGS(&bmp));
                m_pImgArray[i].SetWICBitmpaSource(bmp);
            }
        }
        return m_uImgCount;
    }

    //////////////////////////////////////////////////////////////////////////
    CAutoRefPtr<IWICImagingFactory> SImgDecoderFactory::s_wicImgFactory;

    SImgDecoderFactory::SImgDecoderFactory()
    {
        HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory,NULL,
            CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&s_wicImgFactory));
        (hr);
        ASSERT(SUCCEEDED(hr));
    }

    SImgDecoderFactory::~SImgDecoderFactory()
    {
        if(s_wicImgFactory) s_wicImgFactory = NULL;
    }

    IImgDecoder * SImgDecoderFactory::CreateImgDecoder()
    {
        return new SImgDecoder_WIC;
    }
    
    //////////////////////////////////////////////////////////////////////////
    BOOL CreateImgDecoderFactory( IImgDecoderFactory **pImgDecoderFactory )
    {
        *pImgDecoderFactory = new SImgDecoderFactory;
        return TRUE;
    }

}//end of namespace SOUI
