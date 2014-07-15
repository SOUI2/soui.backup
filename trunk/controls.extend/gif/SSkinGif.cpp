#include "StdAfx.h"
#include "SSkinGif.h"
#include <helper/SplitString.h>
#include <interface/imgdecoder-i.h>
#include <interface/render-i.h>

namespace SOUI
{

    LRESULT SSkinGif::OnAttrSrc( const SStringW &strValue,BOOL bLoading )
    {
        SStringWList strLst;
        size_t nSegs=SplitString(strValue,L':',strLst);
        CAutoRefPtr<IImgX> pImgX;
        if(nSegs == 2)
        {
            pImgX = GETRESPROVIDER->LoadImgX(strLst[0],strLst[1]);
        }else
        {//自动从GIF资源类型里查找资源
            pImgX = GETRESPROVIDER->LoadImgX(L"gif",strLst[0]);
        }
        LoadFromImgX(pImgX);
        return S_OK;
    }

void SSkinGif::Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha )
{
    if(m_nFrames == 0 || !m_pFrames) return;
	if(dwState!=-1) SelectActiveFrame(dwState);
    CRect rcSrc(CPoint(0,0),GetSkinSize());
    pRT->DrawBitmapEx(rcDraw,m_pFrames[m_iFrame].pBmp,rcSrc,EM_STRETCH,byAlpha);
}

long SSkinGif::GetFrameDelay( int iFrame )
{
	if(iFrame==-1) iFrame=m_iFrame;
	long nRet=-1;
	if(m_nFrames>1 && iFrame>=0 && iFrame<m_nFrames)
	{
		nRet=m_pFrames[iFrame].nDelay;
	}
	return nRet;
}

void SSkinGif::ActiveNextFrame()
{
	if(m_nFrames>1)
	{
		m_iFrame++;
		if(m_iFrame==m_nFrames) m_iFrame=0;
		SelectActiveFrame(m_iFrame);
	}
}

void SSkinGif::SelectActiveFrame( int iFrame )
{
	if(m_nFrames>1 && iFrame<m_nFrames)
	{
        m_iFrame = iFrame;
	}
}

int SSkinGif::LoadFromFile( LPCTSTR pszFileName )
{
    CAutoRefPtr<IImgX> pImgX;
    GETRENDERFACTORY->GetImgDecoderFactory()->CreateImgX(&pImgX);
    pImgX->LoadFromFile(pszFileName);
    return LoadFromImgX(pImgX);
}

int SSkinGif::LoadFromMemory( LPVOID pBits,size_t szData )
{
    CAutoRefPtr<IImgX> pImgX;
    GETRENDERFACTORY->GetImgDecoderFactory()->CreateImgX(&pImgX);
    pImgX->LoadFromMemory(pBits,szData);
    return LoadFromImgX(pImgX);
}

int SSkinGif::LoadFromImgX( IImgX *pImgX )
{
    if(!pImgX) return 0;
    if(m_pFrames) delete []m_pFrames;
    m_nFrames = pImgX->GetFrameCount();
    m_iFrame =0;

    m_pFrames = new SGifFrame[m_nFrames];
    for(int i=0; i< m_nFrames;i++)
    {
        GETRENDERFACTORY->CreateBitmap(&m_pFrames[i].pBmp);
        m_pFrames[i].pBmp->Init(pImgX->GetFrame(i));
        m_pFrames[i].nDelay=pImgX->GetFrame(i)->GetDelay();
    }
    return m_nFrames;
}

}//end of namespace SOUI