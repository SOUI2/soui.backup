#include "stdafx.h"
#include "SDesignerRoot.h"


long g_nUIElmIndex = 0;

namespace SOUI{

	SUIWindow::SUIWindow(void)
	{
		
	}

	SWindow * SUIWindow::CreateChild(LPCWSTR classname)
	{
		SStringT wndClassname = classname;
		if (wndClassname.CompareNoCase(_T("realwnd")) == 0)
			wndClassname = _T("ui_window");
		
		SWindow *pChild = SApplication::getSingleton().CreateWindowByName(wndClassname);
		if (!pChild)
		{
			pChild = SApplication::getSingleton().CreateWindowByName(_T("ui_window"));
		}
		
		if (pChild)
		{
			pChild->SetUserData((ULONG_PTR)(GetUIElmIndex()));
		}
		return pChild;
	}

	//////////////////////////////////////////////////////////////////////////

	SDesignerRoot::SDesignerRoot(void)
	{
	}

	SDesignerRoot::~SDesignerRoot(void)
	{
	}

	void SDesignerRoot::BeforePaint(IRenderTarget *pRT, SPainter &painter)
	{
		pRT->SelectObject(m_defFont,(IRenderObj**)&painter.oldFont);
	}

	void SDesignerRoot::AfterPaint(IRenderTarget *pRT, SPainter &painter)
	{
		pRT->SelectObject(painter.oldFont);
	}

	

}
