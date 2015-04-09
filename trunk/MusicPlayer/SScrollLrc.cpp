#include "stdafx.h"
#include "SScrollLrc.h"

namespace SOUI
{
    SScrollLrc::SScrollLrc(void):m_nSpeed(100)
		,bTurnFlag(true)
		,pParen(NULL)
	    ,pCallSetlrc(NULL)
    {
		int i=0;
	}

    SScrollLrc::~SScrollLrc(void)
    {
		int i=0;
    }

	SScrollLrc * SScrollLrc::GetInstance()
	{
		static SScrollLrc _Instance;
		return &_Instance;
	}


    void SScrollLrc::OnTimer(char cTimer)
    {
		if (100==cTimer)
		{
			static int i= 0;
			CRect rect; 
			CPoint ptNew;
			GetClientRect(&rect);
			CSize size=GetViewSize();

			ptNew.x=size.cx;
			if (bTurnFlag)
			{		
				i++;
			}else
			{		
				i--;
			}
			ptNew.y=i;
			SetViewOrigin(ptNew);
			if(i>(size.cy-rect.Height()))
			{
				bTurnFlag=false;
			}
			if (i<0)
			{
				bTurnFlag=true;
			}
			if (!(i%60))//滚完一行高亮一行
			{
				pCallSetlrc(pParen);
			}
			
		}
    }

    void SScrollLrc::OnSize(UINT nType, CSize size)
    {
        __super::OnSize(nType,size);
		//KillTimer(1);
		//SetTimer(1,m_nSpeed);
    }

    void SScrollLrc::SetWindowText(const SStringT & strText)
    {
        m_strText = strText;
    }
	void SScrollLrc::LoadLrc(const SStringT strLrc)  
	{

	}

	void SScrollLrc::StarsRollLrc()  
	{
		KillTimer(100);
		SetTimer(100,m_nSpeed);
	}

	BOOL SScrollLrc::GetSetLrcFun(pCallBackLrc funCall,void *pUser)
	{
		if (funCall != NULL)
		{
			pParen=pUser;
			pCallSetlrc= funCall;
			return TRUE;
		}
		return FALSE;
	}


}
