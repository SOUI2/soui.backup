#include "StdAfx.h"
#include "SouiRealWndHandler.h"
#include "ScintillaWnd.h"

namespace SOUI
{
	CSouiRealWndHandler::CSouiRealWndHandler(void)
	{
	}

	CSouiRealWndHandler::~CSouiRealWndHandler(void)
	{
	}

	HWND CSouiRealWndHandler::OnRealWndCreate(SRealWnd *pRealWnd)
	{
		if (pRealWnd->GetRealWndParam().m_strClassName == _T("scintilla"))
		{
			CScintillaWnd *pWnd = new CScintillaWnd;
			BOOL bOK = pWnd->Create(pRealWnd->GetRealWndParam().m_strWindowName, WS_CHILD, CRect(0, 0, 0, 0), pRealWnd->GetContainer()->GetHostHwnd(), pRealWnd->GetID(), SApplication::getSingleton().GetInstance());
			if (!bOK)
			{
				SASSERT(FALSE);
				delete pWnd;
				return 0;
			}
			pRealWnd->SetUserData((ULONG_PTR)pWnd);
			return pWnd->m_hWnd;
		}
		else
		{
			return 0;
		}
	}

	void CSouiRealWndHandler::OnRealWndDestroy(SRealWnd *pRealWnd)
	{
		if (pRealWnd->GetRealWndParam().m_strClassName == _T("scintilla"))
		{
			CScintillaWnd *pWnd = (CScintillaWnd *)pRealWnd->GetUserData();
			if (pWnd)
			{
				pWnd->DestroyWindow();
				delete pWnd;
			}
		}
	}

	//不处理，返回FALSE
	BOOL CSouiRealWndHandler::OnRealWndSize(SRealWnd *pRealWnd)
	{
		return FALSE;
	}

	//不处理，返回FALSE
	BOOL CSouiRealWndHandler::OnRealWndInit(SRealWnd *pRealWnd)
	{
		return FALSE;
	}
}
