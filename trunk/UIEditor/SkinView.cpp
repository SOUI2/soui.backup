#include "StdAfx.h"
#include "SkinView.h"
namespace SOUI
{

	CSkinView_Base::CSkinView_Base(void):m_crSep(RGBA(0xff,0,0,0xff))
	{
        GETRENDERFACTORY->CreateBitmap(&m_img);
	}

	CSkinView_Base::~CSkinView_Base(void)
	{
	}

	void CSkinView_Base::OnPaint(IRenderTarget *pRT )
	{
		CRect rcWnd;
		GetClientRect(&rcWnd);
		CRect rcState=rcWnd;
		int nStates=GetSkin()->GetStates();
		rcState.right=rcState.left+rcState.Width()/nStates;
		for(int i=0;i<nStates;i++)
		{
			GetSkin()->Draw(pRT,rcState,i);
			rcState.OffsetRect(rcState.Width(),0);
		}

		//draw seperate line
		rcState=rcWnd;
		rcState.right=rcState.left+rcState.Width()/nStates;

        CPoint pts[2];
        pts[0] = CPoint(rcState.right,rcState.top);
        pts[1] = CPoint(rcState.right,rcState.bottom);
        CAutoRefPtr<IPen>  pPen ,pOldPen;
        pRT->CreatePen(PS_DASHDOT,m_crSep,1,&pPen);
        pRT->SelectObject(pPen,(IRenderObj**)&pOldPen);
		for(int i=0;i<nStates-1;i++)
		{

            pRT->DrawLines(pts,2);
            pts[0].x+=rcState.Width();
            pts[1].x+=rcState.Width();
		}
        pRT->SelectObject(pOldPen);
	}


	BOOL CSkinView_Base::SetImageFile( LPCTSTR pszFileName )
	{
		BOOL bOK= S_OK == m_img->LoadFromFile(pszFileName);
		Invalidate();
		return bOK;
	}

	//////////////////////////////////////////////////////////////////////////
	CSkinView_ImgList::CSkinView_ImgList()
	{
		m_skin=new SSkinImgList;
		m_skin->SetImage(m_img);
	}

	//////////////////////////////////////////////////////////////////////////

	void CSkinView_ImgFrame::OnPaint(IRenderTarget *pRT)
	{
		CSkinView_ImgList::OnPaint(pRT);
		CRect rcMargin=m_skin->GetMargin();
		CRect rcWnd;
		GetClientRect(&rcWnd);
		
		CRect rcState=rcWnd;
		int nStates=GetSkin()->GetStates();
		rcState.right=rcState.left+rcState.Width()/nStates;
		
        CAutoRefPtr<IPen>  pPen ,pOldPen;
        pRT->CreatePen(PS_DASHDOT,m_crSep,1,&pPen);
        pRT->SelectObject(pPen,(IRenderObj**)&pOldPen);

        CPoint pts[2];
		for(int i=0;i<nStates;i++)
		{
			if(rcMargin.left!=0)
			{
                pts[0] = CPoint(rcState.left+rcMargin.left,rcState.top);
                pts[1] = CPoint(rcState.left+rcMargin.left,rcState.bottom);
                pRT->DrawLines(pts,2);
			}
			if(rcMargin.right!=0)
			{
                pts[0] = CPoint(rcState.right-rcMargin.right,rcState.top);
                pts[1] = CPoint(rcState.right-rcMargin.right,rcState.bottom);
                pRT->DrawLines(pts,2);
			}
			if(rcMargin.top!=0)
			{
                pts[0] = CPoint(rcState.left,rcState.top+rcMargin.top);
                pts[1] = CPoint(rcState.right,rcState.top+rcMargin.top);
                pRT->DrawLines(pts,2);
			}
			if(rcMargin.bottom!=0)
			{
                pts[0] = CPoint(rcState.left,rcState.bottom-rcMargin.bottom);
                pts[1] = CPoint(rcState.right,rcState.bottom-rcMargin.bottom);
                pRT->DrawLines(pts,2);
			}
			rcState.OffsetRect(rcState.Width(),0);
		}
        pRT->SelectObject(pOldPen);
	}
}

