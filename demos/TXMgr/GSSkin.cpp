#include "stdafx.h"
#include "GSSkin.h"
#include <GdiPlus.h>  
#include "helper/SplitString.h"

//////////////////////////////////////////////////////////////////////////
// GSSkinImgList
GSSkinImgList::GSSkinImgList()
{
	m_nSizeX = 0;
	m_nSizeY = 0;

	fOffsetX = 0;
	fOffsetY = 0;

	m_bmpMask = NULL;
	m_bmpAttrImg = NULL;

	m_nSkinState = 0;
}

GSSkinImgList::~GSSkinImgList()
{
}

SIZE GSSkinImgList::GetSkinSize()
{
	SIZE ret = { 0, 0 };

	if (m_arrBitMap.GetCount() > 0)
		ret = m_arrBitMap.GetAt(0)->Size();
	else
	{
		if (m_pImg)
			ret = m_pImg->Size();

		if (m_bVertical)
		{
			if (m_nSizeY)	ret.cy = m_nSizeY;
			else            ret.cy /= m_nStates;

		}
		else
		{
			if (m_nSizeX)
				ret.cx = m_nSizeX;
			else
				ret.cx /= m_nStates;
		}
	}

		
	return ret;
}

SIZE GSSkinImgList::GetOffsetSize(DWORD dwState)
{
	SIZE ret = { 0, 0 };

	if (m_arrBitMap.GetCount())
		return ret;

	SIZE imgSize = m_pImg->Size();
	if (m_bVertical)
	{
		ret.cy = (imgSize.cy - m_nSizeY) / m_nStates * dwState;
	}
	else
	{
		ret.cx = (imgSize.cx - m_nSizeX) / m_nStates * dwState;
	}

	return ret;
}


IBitmap* GSSkinImgList::GetImage(DWORD dwState)
{
	if (m_arrBitMap.GetCount() > 0 && m_arrBitMap.GetCount() > dwState)
	{
		return m_arrBitMap.GetAt(dwState);
	}

	return m_arrBitMap.GetCount() > 0 ? m_arrBitMap.GetAt(0) : m_pImg;
}

void GSSkinImgList::_Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState, BYTE byAlpha)
{
	if (!m_pImg && m_arrBitMap.GetCount() == 0) return;
	SIZE sz = GetSkinSize();
	RECT rcSrc = { 0, 0, sz.cx, sz.cy };
	SIZE szoff = GetOffsetSize(dwState);	

	OffsetRect(&rcSrc, szoff.cx, szoff.cy);

	if (m_nSkinState != 0 && dwState == 0)
	{
		dwState = m_nSkinState;
	}

	IBitmap* pImg = GetImage(dwState);

	if (!m_bmpMask)
	{
		if (m_bTile && (pImg->Height() > (rcDraw->bottom - rcDraw->top) || pImg->Width() > (rcDraw->right - rcDraw->left)) && (fOffsetX != 0 || fOffsetY != 0))
		{
			rcSrc.left += pImg->Width() * fOffsetX;
			rcSrc.top += pImg->Height() * fOffsetY;
		}

		pRT->DrawBitmap9Patch(rcDraw, pImg, &rcSrc, &m_rcMargin, GetExpandMode(), byAlpha);	
	}
	else
	{

		_DrawAttrImg(pRT, rcDraw, rcSrc, byAlpha);

		RECT rcSrcEx = { 0, 0, sz.cx, sz.cy };
		pRT->DrawBitmap9Patch(rcDraw, m_bmpAttrImg, &rcSrcEx, &m_rcMargin, GetExpandMode(), byAlpha);

	}
	
}

BOOL GSSkinImgList::_DrawAttrImg(IRenderTarget *pRT, LPCRECT rcDraw, const RECT& rcSrc, BYTE byAlpha)
{
	if (m_bmpMask == NULL)
	{
		return FALSE;
	}

	SIZE sz = GetSkinSize();
	CRect rect(0,0,sz.cx,sz.cy);

	CAutoRefPtr<IRenderTarget> pRTDst;
	GETRENDERFACTORY->CreateRenderTarget(&pRTDst, sz.cx, sz.cy);
	pRTDst->DrawBitmapEx(rect, m_pImg, &rcSrc, GetExpandMode());

	CAutoRefPtr<IBitmap> pBmp = (IBitmap*)pRTDst->GetCurrentObject(OT_BITMAP);
	if (!pBmp)
	{
		return FALSE;
	}

	//从mask的指定channel中获得alpha通道，与cache按位运算
	LPBYTE pBitCache = (LPBYTE)pBmp->LockPixelBits();
	LPBYTE pBitMask = (LPBYTE)m_bmpMask->LockPixelBits();

	// 重新计算宽高，使用两个图片像素的最小值
	int cx = m_bmpMask->Width() < pBmp->Width() ? m_bmpMask->Width() : pBmp->Width();
	int cy = m_bmpMask->Height() < pBmp->Height() ? m_bmpMask->Height() : pBmp->Height();
	for (int y = 0; y < cy; ++y)
	{
		LPBYTE pDst = pBitCache + y * pBmp->Width() * 4;
		LPBYTE pSrc = pBitMask + y * m_bmpMask->Width() * 4;

		for (int x = 0; x < cx; ++x)
		{
			BYTE byAlpha = *(pSrc + 3);
 			if (byAlpha == 0)
 			{
				*pDst++ = ((*pDst) * byAlpha) >> 8;
				*pDst++ = ((*pDst) * byAlpha) >> 8;
				*pDst++ = ((*pDst) * byAlpha) >> 8;
				*pDst++ = byAlpha;

				pDst -= 4;
			}

			pDst += 4;
			pSrc += 4;
		}
	}

	pBmp->UnlockPixelBits(pBitCache);
	m_bmpMask->UnlockPixelBits(pBitMask);

	m_bmpAttrImg = pBmp;
	
	return TRUE;
}

HRESULT GSSkinImgList::OnAttrMask(const SStringW& strValue, BOOL bLoading)
{
	IBitmap *pImg = NULL;
	pImg = LOADIMAGE2(strValue);

	if (!pImg)
	{
		return E_FAIL;
	}
	m_bmpMask = pImg;
	pImg->Release();

	return S_OK;
}

static const TCHAR  KImgListPropSeprator = _T(';');   //字体属性之间的分隔符，不再支持其它符号。
HRESULT GSSkinImgList::OnSetImgList(const SStringW & strValue, BOOL bLoading)
{
	for (int i = 0; i < m_arrBitMap.GetCount(); i++)
	{
		m_arrBitMap.GetAt(i)->Release();
	}
	m_arrBitMap.RemoveAll();

	SStringTList fontProp;
	SplitString(strValue, KImgListPropSeprator, fontProp);

	for (int i = 0;i < fontProp.GetCount(); i++)
	{
		SStringW strt = fontProp.GetAt(i);
		SOUI::IBitmap *pImg = LOADIMAGE2(fontProp.GetAt(i));
		if (pImg)	m_arrBitMap.Add(pImg);
	}

	if (m_arrBitMap.GetCount())
	{
		m_nStates = m_arrBitMap.GetCount();
	}

	return S_OK;
}

HRESULT GSSkinImgList::OnSetImgPath(const SStringW & strValue, BOOL bLoading)
{
	for (int i = 0; i < m_arrBitMap.GetCount(); i++)
	{
		m_arrBitMap.GetAt(i)->Release();
	}
	m_arrBitMap.RemoveAll();

	for (int i = 0; i < 200; i++)
	{
		SStringW strFile = strValue;
		strFile.AppendFormat(L"\\%d.png", i);

		SOUI::IBitmap *pImg = LOADIMAGE(L"file", strFile);
		if (pImg)			m_arrBitMap.Add(pImg);
		else    break;
	}

	if (m_arrBitMap.GetCount())
	{
		m_nStates = m_arrBitMap.GetCount();
	}

	return S_OK;
}

HRESULT GSSkinImgList::OnAttrOffset(const SStringW & strValue, BOOL bLoading)
{
	float fx, fy;
	if (2 != swscanf(strValue, L"%f,%f", &fx, &fy))
	{
		return E_FAIL;
	}
	fOffsetX = fx;
	fOffsetY = fy;
	return S_OK;
}

