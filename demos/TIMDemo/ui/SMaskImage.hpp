#pragma once
#include <core/swnd.h>

namespace SOUI
{
    
class SMaskImage : public SWindow
{
	SOUI_CLASS_NAME(SMaskImage, L"maskimg")   //

public:
	SMaskImage()
		: m_pSkin(NULL)
	{

	}
	~SMaskImage()
	{

	}
protected:
    //SWindow的虚函数
	virtual CSize GetDesiredSize(LPCRECT pRcContainer)
	{
		CSize szRet;
		if(NULL != m_pSkin) 
			szRet = m_pSkin->GetSkinSize();

		return szRet;
	}


protected:
	void OnPaint(IRenderTarget *pRT)
	{
		CRect rcClient = GetClientRect();
		if(NULL == m_bmpCache )
		{
			if(NULL != m_pSkin)
			{				
				m_pSkin->Draw(pRT, rcClient, 0);
			}
			return ;
		}

		
		CRect rcCache(CPoint(0, 0), m_bmpCache->Size());
		pRT->DrawBitmapEx(&rcClient, m_bmpCache, &rcCache, EM_STRETCH);
	}

	HRESULT OnAttrMask(const SStringW & strValue, BOOL bLoading)
	{
		SStringW strChannel = strValue.Right(2);
		m_iMaskChannel = -1;
		if(strChannel == L".a")
			m_iMaskChannel = 3;
		else if(strChannel == L".r")
			m_iMaskChannel =0;
		else if(strChannel == L".g")
			m_iMaskChannel = 1;
		else if(strChannel == L".b")
			m_iMaskChannel = 2;

		IBitmap* pImg = NULL;
		if(m_iMaskChannel==-1)
		{//use alpha channel as default
			m_iMaskChannel = 0;
			pImg = LOADIMAGE2(strValue);
		}else
		{
			pImg = LOADIMAGE2(strValue.Left(strValue.GetLength()-2));
		}
		if(!pImg)
		{
			return E_FAIL;
		}
		m_bmpMask = pImg;
		pImg->Release();

		m_bmpCache = NULL;
		GETRENDERFACTORY->CreateBitmap(&m_bmpCache);
		m_bmpCache->Init(m_bmpMask->Width(),m_bmpMask->Height());

		return S_OK;
	}

    HRESULT OnAttrSkin(const SStringW & strValue, BOOL bLoading)
	{
		m_pSkin = GETSKIN(strValue, GetScale());
		if(NULL != m_bmpCache)
		{			
			MakeCacheApha();
		}
		
		return bLoading ? S_OK : S_FALSE;
	}

	void MakeCacheApha()
	{
		SASSERT(m_bmpMask && m_bmpCache);
		CAutoRefPtr<IRenderTarget> pRTDst;
		GETRENDERFACTORY->CreateRenderTarget(&pRTDst, 0, 0);
		CAutoRefPtr<IRenderObj> pOldBmp;
		pRTDst->SelectObject(m_bmpCache, &pOldBmp);
		
		if(NULL != m_pSkin)
		{
			CRect rc(CPoint(0, 0), m_bmpCache->Size());
			m_pSkin->Draw(pRTDst, &rc, 0);
		}
		

		pRTDst->SelectObject(pOldBmp);

		//从mask的指定channel中获得alpha通道
		LPBYTE pBitCache = (LPBYTE)m_bmpCache->LockPixelBits();
		LPBYTE pBitMask = (LPBYTE)m_bmpMask->LockPixelBits();
		LPBYTE pDst = pBitCache;
		LPBYTE pSrc = pBitMask + m_iMaskChannel;
		int nPixels = m_bmpCache->Width()*m_bmpCache->Height();
		for(int i=0;i<nPixels;i++)
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

	SOUI_ATTRS_BEGIN()
		ATTR_CUSTOM(L"mask", OnAttrMask)//image.a
		ATTR_CUSTOM(L"skin", OnAttrSkin)
	SOUI_ATTRS_END()

    //SOUI控件消息映射表
	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)    //窗口绘制消息
	SOUI_MSG_MAP_END()

protected:
	ISkinObj*						m_pSkin;  /**< ISkinObj对象 */
	CAutoRefPtr<IBitmap>    m_bmpCache;
	CAutoRefPtr<IBitmap>    m_bmpMask;
	int									m_iMaskChannel;
};

}