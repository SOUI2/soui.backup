#include "duistd.h"
#include "activex/DuiBStr.h"
#include "../activex/DuiAxContainer.h"
#include "control/DuiActiveX.h"

namespace SOUI
{
//////////////////////////////////////////////////////////////////////////
    class SAxContainerImpl : public SAxContainer,public IAxHostDelegate
    {
    public:
        SAxContainerImpl(SActiveX *pOwner):m_pOwner(pOwner)
        {
            SetAxHost(this);
        }
    protected:
        virtual HWND GetAxHostWindow() const
        {
            return m_pOwner->GetContainer()->GetHostHwnd();
        }
        virtual void OnAxActivate(IUnknown *pCtrl)
        {
            m_pOwner->OnAxActivate(pCtrl);
        }

        virtual void OnAxInvalidate(LPCRECT pRect,BOOL bErase)
        {
            m_pOwner->InvalidateRect(pRect);
        }

        virtual void OnAxSetCapture(BOOL fCapture)
        {
            if(fCapture)
            {
                m_pOwner->SetCapture();
            }else
            {
                m_pOwner->ReleaseCapture();
            }
        }
        
        virtual HRESULT OnAxGetDC(LPCRECT pRect, DWORD grfFlags, HDC *phDC)
        {
            return S_FALSE;
        }

        virtual HRESULT OnAxReleaseDC(HDC hdc)
        {
            return S_FALSE;
        }

        SActiveX *m_pOwner;
    };

//////////////////////////////////////////////////////////////////////////

    SActiveX::SActiveX() 
        : m_axContainer(new SAxContainerImpl(this))
        ,m_clsid(CLSID_NULL)
        ,m_clsCtx(CLSCTX_ALL)
        ,m_bDelayInit(FALSE)
    {
    }

    SActiveX::~SActiveX() {
        delete m_axContainer;
    }


    BOOL SActiveX::InitActiveX()
    {
        BOOL bRet=m_axContainer->CreateControl(m_rcWindow,m_clsid,m_clsCtx);
        if(bRet)
        {
            m_axContainer->ActivateAx(NULL);
            SetActiveXVisible(IsVisible(TRUE));
        }
        OnInitActiveXFinished();
        return bRet;
    }

    void SActiveX::OnPaint(IRenderTarget *pRT)
    {
        HDC hdc=pRT->GetDC(0);
        m_axContainer->Draw(hdc, m_rcWindow);
        pRT->ReleaseDC(hdc);
    }

    int SActiveX::OnCreate( LPVOID )
    {
        if(IsEqualCLSID(m_clsid,CLSID_NULL)) return 0;

        if(!m_bDelayInit) InitActiveX();
        return 0;
    }

    void SActiveX::OnSize( UINT nType, CSize size )
    {
        if(m_axContainer->GetActiveXControl())
        {
            m_axContainer->OnPosRectChange(m_rcWindow);        
        }
    }

    void SActiveX::OnShowWindow( BOOL bShow, UINT nStatus )
    {
        __super::OnShowWindow(bShow, nStatus);

        if(bShow && m_bDelayInit)
        {
            InitActiveX();//窗口显示时才初始化
            m_bDelayInit=FALSE;
        }

        SetActiveXVisible(bShow);
    }

    LRESULT SActiveX::OnMouseEvent( UINT uMsg,WPARAM wp,LPARAM lp )
    {
        if(!m_axContainer->GetActiveXControl()) return 0;
        if(uMsg==WM_LBUTTONDOWN) SetFocus();
        return m_axContainer->OnWindowMessage(uMsg, wp, lp);
    }

    LRESULT SActiveX::OnKeyEvent( UINT uMsg,WPARAM wp,LPARAM lp )
    {
        if(!m_axContainer->GetActiveXControl()) return 0;
        return m_axContainer->OnWindowMessage(uMsg, wp, lp);
    }

    HRESULT SActiveX::OnAttrClsid(const SStringW & strValue,BOOL bLoading)
    {
        OLECHAR szCLSID[100] = { 0 };
        wcscpy(szCLSID,strValue);

        HRESULT hr=E_FAIL;
        if( szCLSID[0] == L'{' ) hr=::CLSIDFromString(szCLSID, &m_clsid);
        else hr=::CLSIDFromProgID(szCLSID, &m_clsid);

        if(!SUCCEEDED(hr)) return S_FALSE;
        return S_OK;
    }

    void SActiveX::SetActiveXVisible( BOOL bVisible )
    {
        if(m_axContainer->GetActiveXControl())
        {
            CDuiComQIPtr<IOleWindow> ole_window=m_axContainer->GetActiveXControl();
            if(!ole_window)
            {
                return ;
            }

            HWND window = NULL;
            ole_window->GetWindow(&window);
            if(!window)
            {
                return ;
            }

            ShowWindow(window, bVisible ? SW_SHOW : SW_HIDE);
        }
    }

    IUnknown * SActiveX::GetIUnknow()
    {
        if(!m_axContainer) return NULL;
        return m_axContainer->GetActiveXControl();
    }

    void SActiveX::SetExternalUIHandler( IDocHostUIHandler *pUiHandler )
    {
        if(m_axContainer) m_axContainer->SetExternalUIHandler(pUiHandler);
    }
    //////////////////////////////////////////////////////////////////////////
    SFlashCtrl::SFlashCtrl()
    {
        m_clsid=__uuidof(ShockwaveFlashObjects::ShockwaveFlash);
    }

    //////////////////////////////////////////////////////////////////////////

    SMediaPlayer::SMediaPlayer()
    {
        m_clsid=__uuidof(WMPLib::WindowsMediaPlayer);
    }

    void SMediaPlayer::OnAxActivate(IUnknown *pUnknwn)
    {
        wmp_=pUnknwn;
        if(wmp_)
        {
            wmp_->put_windowlessVideo(VARIANT_TRUE);
        }
    }

    bool SMediaPlayer::Play( LPCWSTR pszUrl )
    {
        if(!wmp_)
        {
            return false;
        }

        wmp_->close();
        wmp_->put_URL(CDuiBStr(pszUrl));
        return true;
    }

}//end of namespace SOUI

