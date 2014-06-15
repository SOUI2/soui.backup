#pragma once
#include "ColourPopup.h"

namespace SOUI
{
#define DUINM_COLORCHANGE	100

	typedef struct tagDUINMCOLORCHANGE
	{
		DUINMHDR hdr;
		COLORREF crSel;
	}DUINMCOLORCHANGE,*LPDUINMCOLORCHANGE;

class CDuiColorPicker :
	public CDuiButton ,
	public IColorPicker
{
	DUIOBJ_DECLARE_CLASS_NAME(CDuiColorPicker, "colorpicker")

public:
	CDuiColorPicker(void);
	~CDuiColorPicker(void);

	COLORREF GetColor(){return m_crCur;}
protected://IColorPicker
	virtual void OnColorChanged(COLORREF cr);
	virtual void OnColorEnd(BOOL bCancel,COLORREF cr);
protected:
	void OnPaint(CDCHandle dc);
	void OnLButtonUp(UINT nFlags,CPoint pt);

	DUIWIN_BEGIN_MSG_MAP()
		MSG_WM_PAINT(OnPaint)
		MSG_WM_LBUTTONUP(OnLButtonUp)
	DUIWIN_END_MSG_MAP()

	DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
		DUIWIN_COLOR_ATTRIBUTE("color",m_crCur=m_crDef,TRUE)
	DUIWIN_DECLARE_ATTRIBUTES_END()
	
	COLORREF	m_crDef;
	COLORREF	m_crCur;
};

}
