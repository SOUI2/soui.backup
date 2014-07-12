#pragma once
#include <wke.h>
namespace SOUI
{
	const char TM_TICKER = 1;
	class SWkeWebkit : public SWindow, protected wkeBufHandler , protected IIdleHandler
	{
		SOUI_CLASS_NAME(SWkeWebkit, L"wkeWebkit")
	public:
		SWkeWebkit(void);
		~SWkeWebkit(void);
		static void WkeWebkit_Init();
		static void WkeWebkit_Shutdown();

		wkeWebView	GetWebView(){return m_pWebView;}
	protected:
		virtual void onBufUpdated (const HDC hdc,int x, int y, int cx, int cy);
        virtual BOOL OnIdle();
	protected:
		int OnCreate(void *);
		void OnDestroy();
		void OnPaint(IRenderTarget *pRT);
		void OnSize(UINT nType, CSize size);
		LRESULT OnMouseEvent(UINT uMsg, WPARAM wParam,LPARAM lParam); 
		LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam,LPARAM lParam); 
		LRESULT OnKeyDown(UINT uMsg, WPARAM wParam,LPARAM lParam);
		LRESULT OnKeyUp(UINT uMsg, WPARAM wParam,LPARAM lParam);
		LRESULT OnChar(UINT uMsg, WPARAM wParam,LPARAM lParam);
		LRESULT OnImeStartComposition(UINT uMsg, WPARAM wParam,LPARAM lParam);
		void OnSetDuiFocus();
		void OnKillDuiFocus();
		void OnTimer(char cTimerID);

		virtual BOOL OnSetCursor(const CPoint &pt);

		BOOL OnAttrUrl(SStringW strValue, BOOL bLoading);
	protected:
		SOUI_ATTRS_BEGIN()
			ATTR_CUSTOM(L"url",OnAttrUrl)
		SOUI_ATTRS_END()

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_CREATE(OnCreate)
			MSG_WM_DESTROY(OnDestroy)
			MSG_WM_SIZE(OnSize)
			MSG_WM_TIMER_EX(OnTimer)
			MSG_WM_SETFOCUS_EX(OnSetDuiFocus)
			MSG_WM_KILLFOCUS_EX(OnKillDuiFocus)
			MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,0x209,OnMouseEvent)
			MESSAGE_HANDLER_EX(WM_MOUSEWHEEL,OnMouseWheel)
			MESSAGE_HANDLER_EX(WM_KEYDOWN,OnKeyDown)
			MESSAGE_HANDLER_EX(WM_KEYUP,OnKeyUp)
			MESSAGE_HANDLER_EX(WM_CHAR,OnChar)
			MESSAGE_HANDLER_EX(WM_IME_STARTCOMPOSITION,OnImeStartComposition)
		SOUI_MSG_MAP_END()

		wkeWebView m_pWebView;
		SStringW m_strUrl;
	};
}
