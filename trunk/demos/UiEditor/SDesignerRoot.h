#pragma once

namespace SOUI
{
	class SDesignerRoot : public SWindow
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


