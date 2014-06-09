#include "stdafx.h"
#include "img-decoder.h"

#pragma  comment(lib,"windowscodecs.lib")

namespace SOUI
{

CAutoRefPtr<IWICImagingFactory> SImgDecoder::s_wicImgFactory;


BOOL SImgDecoder::InitImgDecoder()
{
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory,NULL,
		CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&s_wicImgFactory));
	return SUCCEEDED(hr);
}

void SImgDecoder::FreeImgDecoder()
{
	if(s_wicImgFactory) s_wicImgFactory = NULL;
}


SImgDecoder::SImgDecoder(void):m_pImgArray(NULL),m_uImgCount(0)
{
}

SImgDecoder::~SImgDecoder(void)
{
	if(m_pImgArray) delete []m_pImgArray;
	m_pImgArray = NULL;
}

int SImgDecoder::DecodeFromMemory(void *pBuf,size_t bufLen )
{
	DUIASSERT(s_wicImgFactory);
	DUIASSERT(m_pImgArray == NULL);

	IWICImagingFactory*    factory    = s_wicImgFactory;
	CAutoRefPtr<IWICBitmapDecoder>     decoder;
	CAutoRefPtr<IWICStream> stream ;

	if(FAILED(factory->CreateStream(&stream))) return 0;

	if(FAILED(stream->InitializeFromMemory((BYTE*)pBuf,bufLen))) return 0;

	if(FAILED(factory->CreateDecoderFromStream(stream,NULL,WICDecodeMetadataCacheOnDemand,&decoder))) return 0;

	return _DoDecode(decoder);
}

int SImgDecoder::DecodeFromFile( LPCWSTR pszFileName )
{
	DUIASSERT(s_wicImgFactory);
	DUIASSERT(m_pImgArray == NULL);
	IWICImagingFactory*    factory    = s_wicImgFactory;

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

int SImgDecoder::_DoDecode( IWICBitmapDecoder * pDecoder )
{
	IWICImagingFactory*    factory    = s_wicImgFactory;
	CAutoRefPtr<IWICFormatConverter> converter;
	if(FAILED(factory->CreateFormatConverter(&converter))) 
		return 0;

	if(FAILED(pDecoder->GetFrameCount(&m_uImgCount)))  
		return 0;

	m_pImgArray = new CAutoRefPtr<IWICBitmapSource> [m_uImgCount];
	for(UINT i = 0; i< m_uImgCount ;i++)
	{
		CAutoRefPtr<IWICBitmapFrameDecode> frame;
		if(SUCCEEDED(pDecoder->GetFrame(i,&frame)))
		{
			converter->Initialize(frame,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,NULL,
				0.f,WICBitmapPaletteTypeCustom);

			converter->QueryInterface(IID_PPV_ARGS(&m_pImgArray[i]));
		}
	}
	return m_uImgCount;
}



}
