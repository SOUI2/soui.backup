#include "stdafx.h"
#include "SGifPlayer.h"

namespace SOUI
{

SGifPlayer::SGifPlayer() :m_pgif(NULL), m_iCurFrame(0)
{

}

SGifPlayer::~SGifPlayer()
{
}

void SGifPlayer::OnTimer(char cTimerID)
{	
	KillTimer(1);	

	if(m_pgif)
	{
		int nStates=m_pgif->GetStates();
		m_iCurFrame++;
		m_iCurFrame%=nStates;
		Invalidate();
        
        if(m_pgif->GetFrameDelay()==0)
            SetTimer(1,90);
        else
            SetTimer(1, m_pgif->GetFrameDelay()*10);	
	}
}

void SGifPlayer::OnPaint( IRenderTarget *pRT )
{	
	__super::OnPaint(pRT);
	if(m_pgif)
	{		
		m_pgif->Draw(pRT, m_rcWindow,m_iCurFrame,m_byAlpha);
	}
}

void SGifPlayer::OnShowWindow( BOOL bShow, UINT nStatus )
{
	__super::OnShowWindow(bShow,nStatus);
	if(!bShow)
	{
		KillTimer(1);
	}else if(m_pgif)
	{
        if(m_pgif->GetFrameDelay()==0)
            SetTimer(1,90);
        else
		    SetTimer(1, m_pgif->GetFrameDelay()*10);					
	}
}

HRESULT SGifPlayer::OnAttrGif( const SStringW & strValue, BOOL bLoading )
{
	ISkinObj *pSkin = SSkinPool::getSingleton().GetSkin(strValue);
	if(!pSkin) return E_FAIL;
	if(!pSkin->IsClass(SSkinGif::GetClassName())) return S_FALSE;
	m_pgif=static_cast<SSkinGif*>(pSkin);
	return bLoading?S_OK:S_FALSE;
}

CSize SGifPlayer::GetDesiredSize( LPRECT pRcContainer )
{
	CSize sz;
	if(m_pgif) sz=m_pgif->GetSkinSize();
	return sz;
}

BOOL SGifPlayer::PlayGifFile( LPCTSTR pszFileName )
{
    SStringW key=S_CT2W(pszFileName);
    if(SSkinPool::getSingleton().HasKey(key))
    {
        ISkinObj *pSkin = GETSKIN(key);
        if(!pSkin->IsClass(SSkinGif::GetClassName())) return FALSE;
        m_pgif=static_cast<SSkinGif*>(pSkin);
        return TRUE;
    }
    SSkinGif *pGifSkin = (SSkinGif*)SApplication::getSingleton().CreateSkinByName(SSkinGif::GetClassName());
    if(!pGifSkin) return FALSE;
    if(0==pGifSkin->LoadFromFile(pszFileName))
    {
        pGifSkin->Release();
        return FALSE;
    }
    
    SSkinPool::getSingleton().AddKeyObject(key,pGifSkin);//将创建的skin交给skinpool管理
    m_pgif = pGifSkin;
    if(m_layout.IsFitContent())
    {
        GetParent()->UpdateChildrenPosition();
    }
    return TRUE;
}

}
