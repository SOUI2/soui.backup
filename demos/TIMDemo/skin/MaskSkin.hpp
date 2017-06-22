#ifndef __MASK_SKIN_HPP_
#define __MASK_SKIN_HPP_

#include "core/SSkin.h"

//************************************
// 这个是 mask  遮罩 皮肤  头像  在skin.xml 里配置 需要 3个值 
// src 和 imglist 一样 
// mask_a 设置透明值 的rgb a // .a=3 .r=0 .g=1 .b=2 
// mask 设置遮罩 图片 
// <masklist name="default" src="image:default" mask_a="1" mask="image:mask_42" />
// 还提供了 
//************************************
class SSkinMask: public SSkinImgList
{
	SOUI_CLASS_NAME(SSkinMask, L"masklist")

public:
	SSkinMask()
		: m_bmpMask(NULL)
		, m_bmpCache(NULL)
		, m_iMaskChannel(0)
	{			
		
	}
	virtual ~SSkinMask()
	{
		
	}
public:			// 

protected:
	virtual void _Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState, BYTE byAlpha)
	{
		if(!m_pImg) return;
		SIZE sz = GetSkinSize();
		RECT rcSrc={0,0,sz.cx,sz.cy};
		if(m_bVertical) 
			OffsetRect(&rcSrc,0, dwState * sz.cy);
		else
			OffsetRect(&rcSrc, dwState * sz.cx, 0);

		if(m_bmpCache)
		{
			RECT rcSrcEx = { 0, 0, m_bmpCache->Size().cx, m_bmpCache->Size().cy };
			pRT->DrawBitmapEx(rcDraw, m_bmpCache, &rcSrcEx, GetExpandMode(), byAlpha);
		}
		else
		{
			RECT rcSrcEx = { 0, 0, m_pImg->Size().cx, m_pImg->Size().cy };
			pRT->DrawBitmapEx(rcDraw, m_pImg, &rcSrcEx, GetExpandMode(), byAlpha);
		}
		//MakeCacheApha(pRT, rcDraw, rcSrc, byAlpha);
	}

private:
	CAutoRefPtr<IBitmap>    m_bmpMask;
	CAutoRefPtr<IBitmap>    m_bmpCache;
	int									m_iMaskChannel;				// 对应 mask  的rgb a // .a=3 .r=0 .g=1 .b=2
protected:
	SOUI_ATTRS_BEGIN()
		ATTR_CUSTOM(L"src", OnAttrSrc)
		ATTR_INT(L"mask_a", m_iMaskChannel, FALSE)		// 要先设置这个  不然就用默认
		ATTR_CUSTOM(L"mask", OnAttrMask)	  //image.a		
	SOUI_ATTRS_END()
protected:
	HRESULT OnAttrSrc(const SStringW& strValue, BOOL bLoading)
	{
		m_pImg = LOADIMAGE2(strValue);
		if(NULL == m_pImg) 
			return E_FAIL; 

		if(NULL != m_bmpMask)
			MakeCacheApha();

		return bLoading ? S_OK : S_FALSE;
	}

	HRESULT OnAttrMask(const SStringW& strValue, BOOL bLoading)
	{
		IBitmap* pImg = NULL;
		pImg = LOADIMAGE2(strValue);

		if (!pImg)
		{
			return E_FAIL;
		}

		m_bmpMask = pImg;
		pImg->Release();

		m_bmpCache = NULL;
		GETRENDERFACTORY->CreateBitmap(&m_bmpCache);
		m_bmpCache->Init(m_bmpMask->Width(),m_bmpMask->Height());

		if(NULL != m_pImg)
			MakeCacheApha();

		return S_OK;
	}

	void MakeCacheApha()
	{
		SASSERT(m_bmpMask && m_bmpCache);

		CAutoRefPtr<IRenderTarget> pRTDst;
		GETRENDERFACTORY->CreateRenderTarget(&pRTDst, 0, 0);
		
		CAutoRefPtr<IRenderObj> pOldBmp;
		pRTDst->SelectObject(m_bmpCache, &pOldBmp);
		CRect rc(CPoint(0, 0), m_bmpCache->Size());
		
		CRect rcSrc(CPoint(0, 0), m_pImg->Size());

		pRTDst->DrawBitmapEx(rc, m_pImg, &rcSrc, GetExpandMode());
		
		pRTDst->SelectObject(pOldBmp);

		//从mask的指定channel中获得alpha通道
		LPBYTE pBitCache = (LPBYTE)m_bmpCache->LockPixelBits();
		LPBYTE pBitMask = (LPBYTE)m_bmpMask->LockPixelBits();
		LPBYTE pDst = pBitCache;
		LPBYTE pSrc = pBitMask + m_iMaskChannel;
		int nPixels = m_bmpCache->Width()*m_bmpCache->Height();
		for(int i=0; i<nPixels; i++)
		{
			BYTE byAlpha = *pSrc;
			pSrc += 4;
			//源半透明，mask不透明时使用源的半透明属性
			if(pDst[3] == 0xff || (pDst[3]!=0xFF &&byAlpha == 0))
			{//源不透明,或者mask全透明
				*pDst++ = ((*pDst) * byAlpha)>>8;//做premultiply
				*pDst++ = ((*pDst) * byAlpha)>>8;//做premultiply
				*pDst++ = ((*pDst) * byAlpha)>>8;//做premultiply
				*pDst++ = byAlpha;
			}
		}
		m_bmpCache->UnlockPixelBits(pBitCache);
		m_bmpMask->UnlockPixelBits(pBitMask);
		
	}
};
//////////////////////////////////////////////////////////////////////////
#endif // __WINFILE_ICON_SKIN_HPP_

