#include "StdAfx.h"
#include "SkinView.h"
namespace SOUI
{

	CSkinView_Base::CSkinView_Base(void):m_crSep(255)
	{
	}

	CSkinView_Base::~CSkinView_Base(void)
	{
	}

	void CSkinView_Base::OnPaint( CDCHandle dc )
	{
		CRect rcWnd;
		GetClient(&rcWnd);
		CRect rcState=rcWnd;
		int nStates=GetSkin()->GetStates();
		rcState.right=rcState.left+rcState.Width()/nStates;
		for(int i=0;i<nStates;i++)
		{
			GetSkin()->Draw(dc,rcState,i);
			rcState.OffsetRect(rcState.Width(),0);
		}

		//draw seperate line
		rcState=rcWnd;
		rcState.right=rcState.left+rcState.Width()/nStates;
		for(int i=0;i<nStates-1;i++)
		{
			CGdiAlpha::DrawLine(dc,rcState.right,rcState.top,rcState.right,rcState.bottom,m_crSep,PS_DASHDOTDOT);
			rcState.OffsetRect(rcState.Width(),0);
		}
	}


	BOOL CSkinView_Base::SetImageFile( LPCTSTR pszFileName )
	{
		m_img.Clear();
		BOOL bOK=m_img.LoadFromFile(pszFileName);
		NotifyInvalidate();
		return bOK;
	}

	//////////////////////////////////////////////////////////////////////////
	CSkinView_ImgLst::CSkinView_ImgLst()
	{
		m_skin=new CDuiSkinImgList;
		m_skin->SetImage(&m_img);
	}

	void CSkinView_ImgLst::OnPaint( CDCHandle dc )
	{
		CRect rcWnd;
		GetClient(&rcWnd);
		CRect rcState=rcWnd;
		int nStates=GetSkin()->GetStates();
		rcState.right=rcState.left+rcState.Width()/nStates;
		for(int i=0;i<nStates;i++)
		{
			GetSkin()->Draw(dc,rcState,i);
			rcState.OffsetRect(rcState.Width(),0);
		}
		//draw seperate line
		rcState=rcWnd;
		rcState.right=rcState.left+rcState.Width()/nStates;
		for(int i=0;i<nStates-1;i++)
		{
			CGdiAlpha::DrawLine(dc,rcState.right,rcState.top,rcState.right,rcState.bottom,m_crSep,PS_DASHDOTDOT);
			rcState.OffsetRect(rcState.Width(),0);
		}

	}


	//////////////////////////////////////////////////////////////////////////

	void CSkinView_ImgFrame::OnPaint( CDCHandle dc )
	{
		__super::OnPaint(dc);
		CRect rcMargin=m_skin->GetMargin();
		CRect rcWnd;
		GetClient(&rcWnd);
		
		CRect rcState=rcWnd;
		int nStates=GetSkin()->GetStates();
		rcState.right=rcState.left+rcState.Width()/nStates;
		

		for(int i=0;i<nStates;i++)
		{
			if(rcMargin.left!=0)
			{
				CGdiAlpha::DrawLine(dc,
					rcState.left+rcMargin.left,rcState.top,
					rcState.left+rcMargin.left,rcState.bottom,
					m_crFrame,PS_DASHDOT);
			}
			if(rcMargin.right!=0)
			{
				CGdiAlpha::DrawLine(dc,
					rcState.right-rcMargin.right,rcState.top,
					rcState.right-rcMargin.right,rcState.bottom,
					m_crFrame,PS_DASHDOT);
			}
			if(rcMargin.top!=0)
			{
				CGdiAlpha::DrawLine(dc,
					rcState.left,rcState.top+rcMargin.top,
					rcState.right,rcState.top+rcMargin.top,
					m_crFrame,PS_DASHDOT);
			}
			if(rcMargin.bottom!=0)
			{
				CGdiAlpha::DrawLine(dc,
					rcState.left,rcState.bottom-rcMargin.bottom,
					rcState.right,rcState.bottom-rcMargin.bottom,
					m_crFrame,PS_DASHDOT);
			}

			rcState.OffsetRect(rcState.Width(),0);
		}
	}
}

