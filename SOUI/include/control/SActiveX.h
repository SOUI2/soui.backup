#pragma once
#include "core/SWnd.h"
#include "activex/flash10t.tlh"
#include "activex/wmp.tlh"
#include <mshtmhst.h>
//#import "C:\\windows\\system32\\WMP.dll" no_function_mapping //使用这行代码来生成wmp.tlh and wmp.tli

namespace SOUI
{

    class SOUI_EXP SActiveX : public SWindow
    {
        friend class SAxContainerImpl;
    public:
        SOUI_CLASS_NAME(SActiveX, L"activex")
        explicit SActiveX();
        virtual ~SActiveX();

        IUnknown * GetIUnknown();
    protected:
        virtual void OnAxActivate(IUnknown *pUnknwn){}

        int OnCreate(LPVOID);
        void OnSize(UINT nType, CSize size);
        void OnPaint(IRenderTarget *pRT);
        LRESULT OnMouseEvent(UINT uMsg,WPARAM wp,LPARAM lp);
        LRESULT OnKeyEvent(UINT uMsg,WPARAM wp,LPARAM lp);
        void OnShowWindow(BOOL bShow, UINT nStatus);

        virtual UINT OnGetDlgCode(){return SC_WANTALLKEYS;}

        virtual BOOL IsFocusable(){return TRUE;}

        HRESULT OnAttrClsid(const SStringW & strValue,BOOL bLoading);
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_PAINT_EX(OnPaint)
            MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseEvent)
            MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST,WM_KEYLAST,OnKeyEvent)
            MSG_WM_CREATE(OnCreate)
            MSG_WM_SIZE(OnSize)
            MSG_WM_SHOWWINDOW(OnShowWindow)
        SOUI_MSG_MAP_END()

        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"clsID",OnAttrClsid)
            ATTR_DWORD(L"clscText",m_clsCtx,FALSE)
            ATTR_UINT(L"delay",m_bDelayInit,FALSE)
        SOUI_ATTRS_END()

        virtual void OnInitActiveXFinished(){}

        BOOL InitActiveX();
        void SetActiveXVisible(BOOL bVisible);
        void SetExternalUIHandler(IDocHostUIHandler *pUiHandler);
    protected:
        SAxContainerImpl * m_axContainer;
        CLSID    m_clsid;
        DWORD    m_clsCtx;
        BOOL        m_bDelayInit;
    };

    class SOUI_EXP SFlashCtrl : public SActiveX
    {
    public:
        SOUI_CLASS_NAME(SFlashCtrl, L"flash")
        SFlashCtrl();

        ShockwaveFlashObjects::IShockwaveFlash* GetFlashInterface()  const
        {
            return flash_;
        }

        BOOL Play(LPCWSTR pszUrl);
    protected:
        virtual void OnAxActivate(IUnknown *pUnknwn);

        HRESULT OnAttrUrl(const SStringW & strValue,BOOL bLoading);
        
        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"url",OnAttrUrl)
        SOUI_ATTRS_END()

        SStringW m_strUrl;

        SComQIPtr<ShockwaveFlashObjects::IShockwaveFlash> flash_;
    };


    class SOUI_EXP SMediaPlayer :public SActiveX
    {
    public:
        SOUI_CLASS_NAME(SMediaPlayer, L"mediaplayer")
        SMediaPlayer();

        WMPLib::IWMPPlayer4* GetMediaPlayerInterface()  const
        {
            return wmp_;
        }
        bool Play(LPCWSTR pszUrl);

    protected:
        virtual void OnInitActiveXFinished(){
            if(!m_strUrl.IsEmpty() && wmp_)
            {
                Play(m_strUrl);
            }
        }

        virtual void OnAxActivate(IUnknown *pUnknwn);

        SOUI_ATTRS_BEGIN()
            ATTR_STRINGW(L"url",m_strUrl,FALSE)
        SOUI_ATTRS_END()

        SStringW m_strUrl;
        SComQIPtr<WMPLib::IWMPPlayer4> wmp_;
    };

}

