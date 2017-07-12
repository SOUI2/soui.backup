#pragma once

extern long g_nUIElmIndex;			//用来给设计器中的控件一个索引, 以便查找

namespace SOUI
{
	inline long GetUIElmIndex()
	{
		return ++g_nUIElmIndex;
	}

	class SUIWindow : public SWindow
	{
		SOUI_CLASS_NAME(SUIWindow, L"ui_window")
	public:
		SUIWindow(void);

		/*SWindow* CreateChild(LPCWSTR classname);*/
	protected:

	};
	
	class SDesignerRoot : public SUIWindow
	{
		SOUI_CLASS_NAME(SDesignerRoot,L"designerRoot")
	public:
		SDesignerRoot(void);
		~SDesignerRoot(void);

		void SetRootFont(IFontPtr defFont){m_defFont = defFont;}
	protected:
		virtual void BeforePaint(IRenderTarget *pRT, SPainter &painter);
		virtual void AfterPaint(IRenderTarget *pRT, SPainter &painter);

		CAutoRefPtr<IFont> m_defFont;
	};
}


