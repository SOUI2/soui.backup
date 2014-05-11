#include "StdAfx.h"
#include "DuiSkinGif.h"
#pragma comment(lib,"gdiplus.lib")

GUID CDuiSkinGif::ms_Guid = Gdiplus::FrameDimensionTime;

void CDuiSkinGif::OnSetImage()
{
	if(!m_pDuiImg)
	{
		m_nFrames=0;
		if(m_pFrameDelay)
		{
			delete []m_pFrameDelay;
			m_pFrameDelay=NULL;
		}
	}else
	{
		CDuiImgX *pImgX=static_cast<CDuiImgX *>(m_pDuiImg);
		if(!pImgX) m_pDuiImg=NULL;
		else{
			Gdiplus::Image * m_pImage=pImgX->GetImage();
			UINT nCount = m_pImage->GetFrameDimensionsCount();
			if (nCount == 0)
			{
				m_pDuiImg=NULL;
				m_nFrames=0;
				if(m_pFrameDelay)
				{
					delete []m_pFrameDelay;
					m_pFrameDelay=NULL;
				}
				return;
			}

			GUID* pDimensionIDs = new GUID[nCount];
			if (pDimensionIDs != NULL)
			{
				m_pImage->GetFrameDimensionsList(pDimensionIDs, nCount);
				m_nFrames = m_pImage->GetFrameCount(&pDimensionIDs[0]);
				delete pDimensionIDs;
			}
			if(m_nFrames>1)
			{
				UINT nSize = m_pImage->GetPropertyItemSize(PropertyTagFrameDelay);
				DUIASSERT (nSize);

				Gdiplus::PropertyItem * pPropertyItem = (Gdiplus::PropertyItem *)malloc(nSize);
				if (pPropertyItem != NULL)
				{
					m_pImage->GetPropertyItem(PropertyTagFrameDelay, nSize, pPropertyItem);

					m_pFrameDelay = new long[m_nFrames];
					if (m_pFrameDelay != NULL)
					{
						for (int i = 0; i < (int)m_nFrames; i++)
						{
							m_pFrameDelay[i] = ((long*)pPropertyItem->value)[i] * 10;
							if (m_pFrameDelay[i] < 100)
								m_pFrameDelay[i] = 100;
						}
					}
					free(pPropertyItem);
				}
			}
		}
	}
}

void CDuiSkinGif::OnAttributeChanged(const CDuiStringA & strAttrName,BOOL bLoading,HRESULT hRet )
{
	if(strAttrName=="src") 
		OnSetImage();
}

void CDuiSkinGif::Draw( HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha/*=0xFF*/ )
{
	if(dwState!=-1) SelectActiveFrame(dwState);
	CDuiImgX *pImgX=static_cast<CDuiImgX *>(m_pDuiImg);
	Gdiplus::Image *pImg=pImgX->GetImage();
	Gdiplus::Graphics graphics(dc);
	graphics.DrawImage(pImg,Gdiplus::Rect(rcDraw.left, rcDraw.top, rcDraw.Width(), rcDraw.Height()));
}

long CDuiSkinGif::GetFrameDelay( int iFrame )
{
	if(iFrame==-1) iFrame=m_iFrame;
	long nRet=-1;
	if(m_nFrames>1 && iFrame>=0 && iFrame<m_nFrames)
	{
		nRet=m_pFrameDelay[iFrame];
	}
	return nRet;
}

void CDuiSkinGif::ActiveNextFrame()
{
	if(m_nFrames>1)
	{
		m_iFrame++;
		if(m_iFrame==m_nFrames) m_iFrame=0;
		SelectActiveFrame(m_iFrame);
	}
}

void CDuiSkinGif::SelectActiveFrame( int iFrame )
{
	if(m_nFrames>1 && iFrame<m_nFrames)
	{
		CDuiImgX *pImgX=static_cast<CDuiImgX *>(m_pDuiImg);
		Gdiplus::Image *pImg=pImgX->GetImage();
		Gdiplus::Status status=pImg->SelectActiveFrame(&ms_Guid, iFrame);
	}
}