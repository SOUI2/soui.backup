#include "souistd.h"
#include "core/SimpleWnd.h"

namespace SOUI
{
    CSimpleWndHelper * CSimpleWndHelper::s_Instance=NULL;

    CSimpleWndHelper::CSimpleWndHelper(HINSTANCE hInst,LPCTSTR pszClassName)
        :m_hInst(hInst)
        ,m_sharePtr(NULL)
    {
        InitializeCriticalSection(&m_cs);
        m_hHeap=HeapCreate(HEAP_CREATE_ENABLE_EXECUTE,0,0);
        m_atom=CSimpleWnd::RegisterSimpleWnd(hInst,pszClassName);
    }

    CSimpleWndHelper::~CSimpleWndHelper()
    {
        if(m_hHeap) HeapDestroy(m_hHeap);
        DeleteCriticalSection(&m_cs);
        if(m_atom) UnregisterClass((LPCTSTR)m_atom,m_hInst);
    }

    CSimpleWndHelper* CSimpleWndHelper::GetInstance()
    {
        return s_Instance;
    }

    BOOL CSimpleWndHelper::Init(HINSTANCE hInst,LPCTSTR pszClassName)
    {
        if(s_Instance) return FALSE;
        s_Instance=new CSimpleWndHelper(hInst,pszClassName);
        return s_Instance!=NULL;
    }

    void CSimpleWndHelper::Destroy()
    {
        if(s_Instance) delete s_Instance;
        s_Instance=NULL;
    }

    void CSimpleWndHelper::LockSharePtr(void *p)
    {
        EnterCriticalSection(&m_cs);
        m_sharePtr=p;
    }

    void CSimpleWndHelper::UnlockSharePtr()
    {
        LeaveCriticalSection(&m_cs);
    }

//////////////////////////////////////////////////////////////////////////
CSimpleWnd::CSimpleWnd(HWND hWnd)
    :m_bDestoryed(FALSE)
    ,m_pCurrentMsg(NULL)
    ,m_hWnd(hWnd)
    ,m_pfnSuperWindowProc(::DefWindowProc)
    ,m_pThunk(NULL)
{
}

CSimpleWnd::~CSimpleWnd(void)
{
}

ATOM CSimpleWnd::RegisterSimpleWnd( HINSTANCE hInst,LPCTSTR pszSimpleWndName )
{
    WNDCLASSEX wcex = {sizeof(WNDCLASSEX),0};
    wcex.cbSize            = sizeof(WNDCLASSEX);
    wcex.style            = CS_HREDRAW | CS_VREDRAW |CS_DBLCLKS;
    wcex.lpfnWndProc    = StartWindowProc; // 第一个处理函数
    wcex.hInstance        = hInst;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszClassName    = pszSimpleWndName;
    return ::RegisterClassEx(&wcex);
}

HWND CSimpleWnd::Create(LPCTSTR lpWindowName, DWORD dwStyle,DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent,LPVOID lpParam )
{
    CSimpleWndHelper::GetInstance()->LockSharePtr(this);

    m_pThunk=(tagThunk*)HeapAlloc(CSimpleWndHelper::GetInstance()->GetHeap(),HEAP_ZERO_MEMORY,sizeof(tagThunk));
    // 在::CreateWindow返回之前会去调StarWindowProc函数
    HWND hWnd= ::CreateWindowEx(dwExStyle,(LPCTSTR)CSimpleWndHelper::GetInstance()->GetSimpleWndAtom(), lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, 0, CSimpleWndHelper::GetInstance()->GetAppInstance(), lpParam);
    CSimpleWndHelper::GetInstance()->UnlockSharePtr(); 
    if(!hWnd)
    {
        HeapFree(CSimpleWndHelper::GetInstance()->GetHeap(),0,m_pThunk);
        m_pThunk=NULL;
    }
    return hWnd;
}


void CSimpleWnd::OnFinalMessage( HWND hWnd )
{
    if(m_pThunk)
    {
        HeapFree(CSimpleWndHelper::GetInstance()->GetHeap(),0,m_pThunk);
        m_pThunk=NULL;
    }
}

LRESULT CALLBACK CSimpleWnd::WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CSimpleWnd* pThis = (CSimpleWnd*)hWnd; // 强转为对象指针
    MSG msg= {pThis->m_hWnd, uMsg, wParam, lParam};
    const MSG* pOldMsg = pThis->m_pCurrentMsg;
    pThis->m_pCurrentMsg = &msg;
    // pass to the message map to process
    LRESULT lRes;
    BOOL bRet = pThis->ProcessWindowMessage(pThis->m_hWnd, uMsg, wParam, lParam, lRes, 0);
    // restore saved value for the current message
    ASSERT(pThis->m_pCurrentMsg == &msg);

    // do the default processing if message was not handled
    if(!bRet)
    {
        if(uMsg != WM_NCDESTROY)
            lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
        else
        {
            // unsubclass, if needed
            LONG_PTR pfnWndProc = ::GetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC);
            lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
            if(pThis->m_pfnSuperWindowProc != ::DefWindowProc && ::GetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC) == pfnWndProc)
                ::SetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC, (LONG_PTR)pThis->m_pfnSuperWindowProc);
            // mark window as destryed
            pThis->m_bDestoryed=TRUE;
        }

    }
    if((pThis->m_bDestoryed) && pOldMsg== NULL)
    {
        // clear out window handle
        HWND hWndThis = pThis->m_hWnd;
        pThis->m_hWnd = NULL;
        pThis->m_bDestoryed =FALSE;
        // clean up after window is destroyed
        pThis->m_pCurrentMsg = pOldMsg;
        pThis->OnFinalMessage(hWndThis);
    }
    else
    {
        pThis->m_pCurrentMsg = pOldMsg;
    }
    return lRes;
}

LRESULT CALLBACK CSimpleWnd::StartWindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CSimpleWnd* pThis=(CSimpleWnd*)CSimpleWndHelper::GetInstance()->GetSharePtr();

    pThis->m_hWnd=hWnd;
    // 初始化Thunk，做了两件事:1、mov指令替换hWnd为对象指针，2、jump指令跳转到WindowProc
    pThis->m_pThunk->Init((DWORD_PTR)WindowProc, pThis);

    // 得到Thunk指针
    WNDPROC pProc = (WNDPROC)pThis->m_pThunk->GetCodeAddress();
    // 调用下面的语句后，以后消息来了，都由pProc处理
    ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pProc);

    return pProc(hWnd, uMsg, wParam, lParam);
}

BOOL CSimpleWnd::SubclassWindow( HWND hWnd )
{
    ASSERT(::IsWindow(hWnd));
    // Allocate the thunk structure here, where we can fail gracefully.
    m_pThunk=(tagThunk*)HeapAlloc(CSimpleWndHelper::GetInstance()->GetHeap(),HEAP_ZERO_MEMORY,sizeof(tagThunk));
    m_pThunk->Init((DWORD)WindowProc, this);
    WNDPROC pProc = (WNDPROC)m_pThunk->GetCodeAddress();
    WNDPROC pfnWndProc = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pProc);
    if(pfnWndProc == NULL)
    {
        HeapFree(CSimpleWndHelper::GetInstance()->GetHeap(),0,m_pThunk);
        m_pThunk=NULL;
        return FALSE;
    }
    m_pfnSuperWindowProc = pfnWndProc;
    m_hWnd = hWnd;
    return TRUE;
}


HWND CSimpleWnd::UnsubclassWindow( BOOL bForce /*= FALSE*/ )
{
    ASSERT(m_hWnd != NULL);

    WNDPROC pOurProc = (WNDPROC)m_pThunk->GetCodeAddress();
    WNDPROC pActiveProc = (WNDPROC)::GetWindowLongPtr(m_hWnd, GWLP_WNDPROC);

    HWND hWnd = NULL;
    if (bForce || pOurProc == pActiveProc)
    {
        if(!::SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_pfnSuperWindowProc))
            return NULL;

        m_pfnSuperWindowProc = ::DefWindowProc;
        hWnd = m_hWnd;
        m_hWnd = NULL;
    }
    return hWnd;
}

LRESULT CSimpleWnd::ForwardNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lResult = 0;
    switch(uMsg)
    {
    case WM_COMMAND:
    case WM_NOTIFY:
    case WM_PARENTNOTIFY:
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
    case WM_COMPAREITEM:
    case WM_DELETEITEM:
    case WM_VKEYTOITEM:
    case WM_CHARTOITEM:
    case WM_HSCROLL:
    case WM_VSCROLL:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
        lResult = ::SendMessage(GetParent(m_hWnd),uMsg, wParam, lParam);
        break;
    default:
        bHandled = FALSE;
        break;
    }
    return lResult;
}

LRESULT CSimpleWnd::ReflectNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HWND hWndChild = NULL;

    switch(uMsg)
    {
    case WM_COMMAND:
        if(lParam != NULL)    // not from a menu
            hWndChild = (HWND)lParam;
        break;
    case WM_NOTIFY:
        hWndChild = ((LPNMHDR)lParam)->hwndFrom;
        break;
    case WM_PARENTNOTIFY:
        switch(LOWORD(wParam))
        {
        case WM_CREATE:
        case WM_DESTROY:
            hWndChild = (HWND)lParam;
            break;
        default:
            hWndChild = GetDlgItem(m_hWnd,HIWORD(wParam));
            break;
        }
        break;
    case WM_DRAWITEM:
        if(wParam)    // not from a menu
            hWndChild = ((LPDRAWITEMSTRUCT)lParam)->hwndItem;
        break;
    case WM_MEASUREITEM:
        if(wParam)    // not from a menu
            hWndChild = GetDlgItem(m_hWnd,((LPMEASUREITEMSTRUCT)lParam)->CtlID);
        break;
    case WM_COMPAREITEM:
        if(wParam)    // not from a menu
            hWndChild =  ((LPCOMPAREITEMSTRUCT)lParam)->hwndItem;
        break;
    case WM_DELETEITEM:
        if(wParam)    // not from a menu
            hWndChild =  ((LPDELETEITEMSTRUCT)lParam)->hwndItem;

        break;
    case WM_VKEYTOITEM:
    case WM_CHARTOITEM:
    case WM_HSCROLL:
    case WM_VSCROLL:
        hWndChild = (HWND)lParam;
        break;
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
        hWndChild = (HWND)lParam;
        break;
    default:
        break;
    }

    if(hWndChild == NULL)
    {
        bHandled = FALSE;
        return 1;
    }

    ASSERT(::IsWindow(hWndChild));
    return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
}

BOOL CSimpleWnd::DefaultReflectionHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
{
    switch(uMsg)
    {
    case OCM_COMMAND:
    case OCM_NOTIFY:
    case OCM_PARENTNOTIFY:
    case OCM_DRAWITEM:
    case OCM_MEASUREITEM:
    case OCM_COMPAREITEM:
    case OCM_DELETEITEM:
    case OCM_VKEYTOITEM:
    case OCM_CHARTOITEM:
    case OCM_HSCROLL:
    case OCM_VSCROLL:
    case OCM_CTLCOLORBTN:
    case OCM_CTLCOLORDLG:
    case OCM_CTLCOLOREDIT:
    case OCM_CTLCOLORLISTBOX:
    case OCM_CTLCOLORMSGBOX:
    case OCM_CTLCOLORSCROLLBAR:
    case OCM_CTLCOLORSTATIC:
        lResult = ::DefWindowProc(hWnd, uMsg - OCM__BASE, wParam, lParam);
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

LRESULT CSimpleWnd::DefWindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return ::CallWindowProc(m_pfnSuperWindowProc,m_hWnd, uMsg, wParam, lParam);
}

}//namespace SOUI
