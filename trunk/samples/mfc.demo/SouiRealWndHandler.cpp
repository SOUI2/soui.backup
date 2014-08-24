#include "StdAfx.h"
#include "SouiRealWndHandler.h"

namespace SOUI
{
    CSouiRealWndHandler::CSouiRealWndHandler(void)
    {
    }

    CSouiRealWndHandler::~CSouiRealWndHandler(void)
    {
    }

    HWND CSouiRealWndHandler::OnRealWndCreate( SRealWnd *pRealWnd )
    {
        const SRealWndParam &param=pRealWnd->GetRealWndParam();
        if(param.m_strClassName==_T("button"))
        {
            CButton *pbtn=new CButton;
            pbtn->Create(param.m_strWindowName,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,::CRect(0,0,0,0),CWnd::FromHandle(pRealWnd->GetContainer()->GetHostHwnd()),pRealWnd->GetID());
            pRealWnd->SetData(pbtn);
            return pbtn->m_hWnd;
        }else
        {
            return 0;
        }
    }

    void CSouiRealWndHandler::OnRealWndDestroy( SRealWnd *pRealWnd )
    {
        const SRealWndParam &param=pRealWnd->GetRealWndParam();
        if(param.m_strClassName==_T("button"))
        {
            CButton *pbtn=(CButton*) pRealWnd->GetData();
            if(pbtn)
            {
                pbtn->DestroyWindow();
                delete pbtn;
            }
        }
    }

    void CSouiRealWndHandler::OnRealWndSize( SRealWnd *pRealWnd )
    {
        if(::IsWindow(pRealWnd->GetRealHwnd(FALSE)))
        {
            CRect rcWnd;
            pRealWnd->GetWindowRect(&rcWnd);
            ::SetWindowPos(pRealWnd->GetRealHwnd(FALSE),0,rcWnd.left,rcWnd.top,rcWnd.Width(),rcWnd.Height(),SWP_NOZORDER);
        }
    }
}
