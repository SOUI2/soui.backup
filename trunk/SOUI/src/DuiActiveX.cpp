#include "duistd.h"
#include "activex/DuiBStr.h"
#include "activex/DuiAxContainer.h"
#include "DuiActiveX.h"

namespace SOUI
{
//////////////////////////////////////////////////////////////////////////
    class CDuiAxContainerImpl : public CDuiAxContainer,public IAxHostDelegate
    {
    public:
        CDuiAxContainerImpl(CDuiActiveX *pOwner):m_pOwner(pOwner)
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
            m_pOwner->NotifyInvalidateRect(pRect);
        }

        virtual void OnAxSetCapture(BOOL fCapture)
        {
            if(fCapture)
            {
                m_pOwner->SetDuiCapture();
            }else
            {
                m_pOwner->ReleaseDuiCapture();
            }
        }
        
        virtual HRESULT OnAxGetDC(LPCRECT pRect, DWORD grfFlags, HDC *phDC)
        {
            return S_FALSE;
            *phDC=m_pOwner->GetDuiDC((const LPRECT)pRect,grfFlags);
            return S_OK;
        }

        virtual HRESULT OnAxReleaseDC(HDC hdc)
        {
            return S_FALSE;
            m_pOwner->ReleaseDuiDC(hdc);
            return S_OK;
        }

        CDuiActiveX *m_pOwner;
    };

//////////////////////////////////////////////////////////////////////////

    CDuiActiveX::CDuiActiveX() 
        : m_axContainer(new CDuiAxContainerImpl(this))
        ,m_clsid(CLSID_NULL)
        ,m_clsCtx(CLSCTX_ALL)
        ,m_bDelayInit(FALSE)
    {
    }

    CDuiActiveX::~CDuiActiveX() {
        delete m_axContainer;
    }


    BOOL CDuiActiveX::InitActiveX()
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

    void CDuiActiveX::OnPaint(CDCHandle dc)
    {
        m_axContainer->Draw(dc, m_rcWindow);
    }

    int CDuiActiveX::OnCreate( LPVOID )
    {
        if(IsEqualCLSID(m_clsid,CLSID_NULL)) return 0;

        if(!m_bDelayInit) InitActiveX();
        return 0;
    }

    void CDuiActiveX::OnSize( UINT nType, CSize size )
    {
        if(m_axContainer->GetActiveXControl())
        {
            m_axContainer->OnPosRectChange(m_rcWindow);        
        }
    }

    void CDuiActiveX::OnShowWindow( BOOL bShow, UINT nStatus )
    {
        __super::OnShowWindow(bShow, nStatus);

        if(bShow && m_bDelayInit)
        {
            InitActiveX();//窗口显示时才初始化
            m_bDelayInit=FALSE;
        }

        SetActiveXVisible(bShow);
    }

    LRESULT CDuiActiveX::OnMouseEvent( UINT uMsg,WPARAM wp,LPARAM lp )
    {
        if(!m_axContainer->GetActiveXControl()) return 0;
        if(uMsg==WM_LBUTTONDOWN) SetDuiFocus();
        return m_axContainer->OnWindowMessage(uMsg, wp, lp);
    }

    LRESULT CDuiActiveX::OnKeyEvent( UINT uMsg,WPARAM wp,LPARAM lp )
    {
        if(!m_axContainer->GetActiveXControl()) return 0;
        return m_axContainer->OnWindowMessage(uMsg, wp, lp);
    }

    HRESULT CDuiActiveX::OnAttrClsid(const CDuiStringA & strValue,BOOL bLoading)
    {
        CDuiStringW strValueW=DUI_CA2W(strValue,CP_UTF8);

        OLECHAR szCLSID[100] = { 0 };
        wcscpy(szCLSID,strValueW);

        HRESULT hr=E_FAIL;
        if( szCLSID[0] == L'{' ) hr=::CLSIDFromString(szCLSID, &m_clsid);
        else hr=::CLSIDFromProgID(szCLSID, &m_clsid);

        if(!SUCCEEDED(hr)) return S_FALSE;
        return S_OK;
    }

    void CDuiActiveX::SetActiveXVisible( BOOL bVisible )
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

    IUnknown * CDuiActiveX::GetIUnknow()
    {
        if(!m_axContainer) return NULL;
        return m_axContainer->GetActiveXControl();
    }

    void CDuiActiveX::SetExternalUIHandler( IDocHostUIHandler *pUiHandler )
    {
        if(m_axContainer) m_axContainer->SetExternalUIHandler(pUiHandler);
    }
    //////////////////////////////////////////////////////////////////////////
    CDuiFlashCtrl::CDuiFlashCtrl()
    {
        m_clsid=__uuidof(ShockwaveFlashObjects::ShockwaveFlash);
    }

    //////////////////////////////////////////////////////////////////////////

    CDuiMediaPlayer::CDuiMediaPlayer()
    {
        m_clsid=__uuidof(WMPLib::WindowsMediaPlayer);
    }

    void CDuiMediaPlayer::OnAxActivate(IUnknown *pUnknwn)
    {
        wmp_=pUnknwn;
        if(wmp_)
        {
            wmp_->put_windowlessVideo(VARIANT_TRUE);
        }
    }

    bool CDuiMediaPlayer::Play( LPCWSTR pszUrl )
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

