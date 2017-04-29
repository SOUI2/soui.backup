#include "souistd.h"
#include "interface/render-i.h"

namespace SOUI
{
	HRESULT IBitmap::Clone(IBitmap **ppClone) const 
	{
		HRESULT hr = E_UNEXPECTED;
		BOOL bOK = GetRenderFactory()->CreateBitmap(ppClone);
		if(bOK)
		{
			hr=(*ppClone)->Init(Width(),Height(),GetPixelBits());
			if(S_OK != hr)
			{
				(*ppClone)->Release();
				(*ppClone) = NULL;
			}
		}
		return hr;
	}

	HRESULT IBitmap::Scale(IBitmap **pOutput,int nScale,FilterLevel filterLevel)
	{
		if(nScale == 100)
		{
			return Clone(pOutput);
		}
		HRESULT hr = E_UNEXPECTED;
		BOOL bOK = GetRenderFactory()->CreateBitmap(pOutput);
		if(bOK)
		{
			int wid = Width()*nScale/100;
			int hei = Height()*nScale/100;
			IRenderTarget *pRT=NULL;
			if(GetRenderFactory()->CreateRenderTarget(&pRT,wid,hei))
			{
				RECT rcSrc = {0,0,Width(),Height()};
				RECT rcDst ={0,0,wid,hei};
				hr = pRT->DrawBitmapEx(&rcDst,this,&rcSrc,MAKELONG(EM_STRETCH,filterLevel));
				if(hr == S_OK)
				{
					*pOutput = (IBitmap*)pRT->GetCurrentObject(OT_BITMAP);
					(*pOutput)->AddRef();
				}
				pRT->Release();
			}else
			{
				hr = E_OUTOFMEMORY;
			}
		}
		return hr;
	}

	HRESULT IBitmap::Save(LPCWSTR pszFileName,const LPVOID pFormat)
	{
		return GetRenderFactory()->GetImgDecoderFactory()->SaveImage(this,pszFileName,pFormat);
	}
}