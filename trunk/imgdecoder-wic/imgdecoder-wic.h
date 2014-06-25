// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IMGDECODERWIC_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IMGDECODERWIC_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef IMGDECODERWIC_EXPORTS
#define IMGDECODERWIC_API __declspec(dllexport)
#else
#define IMGDECODERWIC_API __declspec(dllimport)
#endif

#include <wincodec.h>
#include <unknown/obj-ref-impl.hpp>

#include <render/imgdecoder-i.h>

namespace SOUI
{
    
    class SImgFrame_WIC : public IImgFrame
    {
    public:
        SImgFrame_WIC(IWICBitmapSource *pFrame=NULL);
        void SetWICBitmpaSource(IWICBitmapSource *pFrame);
        
        virtual BOOL GetSize(UINT *pWid,UINT *pHei);
        virtual BOOL CopyPixels( 
            /* [unique][in] */ const RECT *prc,
            /* [in] */ UINT cbStride,
            /* [in] */ UINT cbBufferSize,
            /* [size_is][out] */ BYTE *pbBuffer);
    protected:
        CAutoRefPtr<IWICBitmapSource>  m_pFrame;    
    };
    
    class SImgDecoder_WIC : public TObjRefImpl<IImgDecoder>
    {
        friend class SImgDecoderFactory;
    public:

        int DecodeFromMemory(void *pBuf,size_t bufLen);
        int DecodeFromFile(LPCWSTR pszFileName);
        int DecodeFromFile(LPCSTR pszFileName);

        IImgFrame * GetFrame(UINT iFrame){
            if(iFrame >= m_uImgCount) return NULL;
            return m_pImgArray+iFrame;
        }
        virtual UINT GetFrameCount(){return m_uImgCount;}
    protected:
        SImgDecoder_WIC(void);
        ~SImgDecoder_WIC(void);
        
        int _DoDecode(IWICBitmapDecoder * pDecoder);

        SImgFrame_WIC *     m_pImgArray;
        UINT					  m_uImgCount;

    };

    class SImgDecoderFactory : public TObjRefImpl<IImgDecoderFactory>
    {
    friend class SImgDecoder_WIC;
    public:
        SImgDecoderFactory();
        ~SImgDecoderFactory();
        
        virtual IImgDecoder * CreateImgDecoder();
    protected:
        static CAutoRefPtr<IWICImagingFactory> s_wicImgFactory;
    };
    
    extern "C" IMGDECODERWIC_API BOOL CreateImgDecoderFactory(IImgDecoderFactory **pImgDecoderFactory);
}//end of namespace SOUI