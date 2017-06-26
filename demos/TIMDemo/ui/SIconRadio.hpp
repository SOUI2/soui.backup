#ifndef __ICONRADIO_H__
#define __ICONRADIO_H__

//#include "core/SWnd.h"
namespace SOUI
{

// 但图片的  单选框 
class SIconRadio : public SRadioBox
{
public:
	SOUI_CLASS_NAME(SIconRadio, L"iconradio")
	SIconRadio(void)
	//: m_pTabSkin(NULL)
	{
		m_style.m_byAlpha = (BYTE)125;
	}
	~SIconRadio(void){}

protected:
	virtual CSize GetDesiredSize(LPCRECT pRcContainer)
	{
		if(m_pSkin) return m_pSkin->GetSkinSize();
		else return SWindow::GetDesiredSize(pRcContainer);
	}
	void OnPaint(IRenderTarget *pRT)
	{
		if(NULL != m_pSkin)
		{			
			CRect rc;
			GetClientRect(&rc);
			DWORD dwState = GetState();
			int nState = 0;
			if(dwState & WndState_Check)
			{
				m_style.m_byAlpha = (BYTE)240;
				nState = 2;
			}
			else if(dwState & WndState_PushDown)
			{
				m_style.m_byAlpha = (BYTE)240;
				nState = 2;
			}
			else if(dwState & WndState_Hover) 
			{
				m_style.m_byAlpha = (BYTE)200;
				nState = 1;
			}
			else
				m_style.m_byAlpha = (BYTE)125;
			
			m_pSkin->Draw(pRT, rc, 0);

			 /*SIZE size = m_pSkin->GetSkinSize();
			 int n = (rc.Width() - size.cx) / 2;
			 CRect rcSk(rc);
			 rcSk.left += n / 2;
			 rcSk.right -= n / 2;
			 rcSk.bottom = rcSk.top + size.cy;
			 m_pSkin->Draw(pRT, rcSk, 0);*/
		 }
		 SWindow::OnPaint(pRT);
	 }

	 virtual void DrawFocus(IRenderTarget *pRT)
	 {
	 }
protected:
	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
	SOUI_MSG_MAP_END()

	SOUI_ATTRS_BEGIN() 
		
	SOUI_ATTRS_END()

	//ISkinObj *  m_pTabSkin;
};


}



#endif	//__PATHBAR_H__