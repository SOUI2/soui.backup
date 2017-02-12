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
        {//只实现了button的创建
            //分配一个MFC CButton对象
            CButton *pbtn=new CButton;
            //创建CButton窗口,注意使用pRealWnd->GetContainer()->GetHostHwnd()作为CButton的父窗口
            //把pRealWnd->GetID()作为真窗口的ID
            pbtn->Create(param.m_strWindowName,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,::CRect(0,0,0,0),CWnd::FromHandle(pRealWnd->GetContainer()->GetHostHwnd()),pRealWnd->GetID());
            //把pbtn的指针放到SRealWnd的Data中保存，以便在窗口destroy时释放pbtn对象。
            pRealWnd->SetData(pbtn);
            //返回成功创建后的窗口句柄
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
        {//销毁真窗口，释放窗口占用的内存
            CButton *pbtn=(CButton*) pRealWnd->GetData();
            if(pbtn)
            {
                pbtn->DestroyWindow();
                delete pbtn;
            }
        }
    }
    
    //不处理，返回FALSE
    BOOL CSouiRealWndHandler::OnRealWndSize( SRealWnd *pRealWnd )
    {
        return FALSE;
    }

    //不处理，返回FALSE
    BOOL CSouiRealWndHandler::OnRealWndInit( SRealWnd *pRealWnd )
    {
        return FALSE;
    }
}
