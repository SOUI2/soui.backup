#include "duistd.h"
#include "SHostDialog.h"

namespace SOUI
{
    SHostDialog::SHostDialog(LPCTSTR pszXmlName):SHostWnd(pszXmlName),m_nRetCode(IDOK)
    {
        SApplication::getSingleton().PushMessageLoop(this);
    }

    SHostDialog::~SHostDialog(void)
    {
        SApplication::getSingleton().PopMessageLoop();
    }

    INT_PTR SHostDialog::DoModal(HWND hParent/*=NULL*/)
    {
        if(!hParent)
        {
            hParent = SThreadActiveWndMgr::GetActive();
            if (!hParent)  hParent = ::GetActiveWindow();
        }

        BOOL bEnableParent = FALSE;
        if (hParent && hParent != ::GetDesktopWindow() && ::IsWindowEnabled(hParent))
        {
            ::EnableWindow(hParent, FALSE);
            bEnableParent = TRUE;
        }
        
        if(!IsWindow())
        {//进入DoModal前已经创建窗口对象时不再执行Create
            if(!Create(hParent, 0,0,0,0))
                return 0;
        }

        HWND hWndLastActive = SThreadActiveWndMgr::SetActive(m_hWnd);

        CSimpleWnd::SendMessage(WM_INITDIALOG, (WPARAM)m_hWnd);
        
        if(GetExStyle()&WS_EX_TOOLWINDOW)
            ::ShowWindow(m_hWnd,SW_SHOWNOACTIVATE);
        else
            ::ShowWindow(m_hWnd,SW_SHOWNORMAL);

        Run();

        // From MFC
        // hide the window before enabling the parent, etc.

        if ( IsWindow() )
        {
            CSimpleWnd::SetWindowPos(
                NULL, 0, 0, 0, 0,
                SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
        }

        if (bEnableParent)
        {
            ::EnableWindow(hParent, TRUE);
        }

        if (hParent != NULL && ::GetActiveWindow() == m_hWnd)
            ::SetActiveWindow(hParent);

        SThreadActiveWndMgr::SetActive(hWndLastActive);

        if ( IsWindow() )
            DestroyWindow();

        return m_nRetCode;
    }

    void SHostDialog::EndDialog( INT_PTR nResult )
    {
        m_nRetCode = nResult;
        PostMessage(WM_QUIT);
    }

    void SHostDialog::OnOK()
    {
        EndDialog(IDOK);
    }

    void SHostDialog::OnCancel()
    {
        EndDialog(IDCANCEL);
    }

}
