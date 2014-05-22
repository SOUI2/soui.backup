#pragma once

#include <wincodec.h>
#include "obj-ref-impl.hpp"

namespace SOUI
{

class SImgDecoder
{
public:
	SImgDecoder(void);
	~SImgDecoder(void);
	static BOOL InitImgDecoder();
	static void FreeImgDecoder();

	int DecodeFromMemory(void *pBuf,size_t bufLen);
	int DecodeFromFile(LPCWSTR pszFileName);

	IWICBitmapSource * GetFrame(UINT iFrame){
		if(iFrame >= m_uImgCount) return NULL;
		return m_pImgArray[iFrame];
	}
protected:
	int _DoDecode(IWICBitmapDecoder * pDecoder);
	
	CAutoRefPtr<IWICBitmapSource> * m_pImgArray;
	UINT					  m_uImgCount;

	static CAutoRefPtr<IWICImagingFactory> s_wicImgFactory;
};

}//
