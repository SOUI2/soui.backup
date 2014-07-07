#pragma once

#ifndef WM_SYSTIMER
#define WM_SYSTIMER 0x0118   //(caret blink)
#endif//WM_SYSTIMER

namespace SOUI
{
    template<class T>
    BOOL RemoveElementFromArray(SArray<T> &arr, T ele)
    {
        for(size_t i=0;i<arr.GetCount();i++)
        {
            if(arr[i] == ele)
            {
                arr.RemoveAt(i);
                return TRUE;
            }
        }
        return FALSE;
    }
    
    struct IMessageFilter
    {
        virtual BOOL PreTranslateMessage(MSG* pMsg) = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////
    // CIdleHandler - Interface for idle processing

    struct IIdleHandler
    {
        virtual BOOL OnIdle() = 0;
    };

    class SOUI_EXP SMessageLoop
    {
    public:
        SArray<IMessageFilter*> m_aMsgFilter;
        SArray<IIdleHandler*> m_aIdleHandler;
        MSG m_msg;

        // Message filter operations
        BOOL AddMessageFilter(IMessageFilter* pMessageFilter)
        {
            return m_aMsgFilter.Add(pMessageFilter);
        }

        BOOL RemoveMessageFilter(IMessageFilter* pMessageFilter)
        {
            return RemoveElementFromArray(m_aMsgFilter,pMessageFilter);
        }

        // Idle handler operations
        BOOL AddIdleHandler(IIdleHandler* pIdleHandler)
        {
            return m_aIdleHandler.Add(pIdleHandler);
        }

        BOOL RemoveIdleHandler(IIdleHandler* pIdleHandler)
        {
            return RemoveElementFromArray(m_aIdleHandler,pIdleHandler);
        }

        static BOOL IsIdleMessage(MSG* pMsg)
        {
            // These messages should NOT cause idle processing
            switch(pMsg->message)
            {
            case WM_MOUSEMOVE:
            case WM_NCMOUSEMOVE:
            case WM_PAINT:
            case WM_SYSTIMER:	// WM_SYSTIMER (caret blink)
                return FALSE;
            }

            return TRUE;
        }

        // Overrideables
        // Override to change message filtering
        virtual BOOL PreTranslateMessage(MSG* pMsg)
        {
            // loop backwards
            for(int i = (int)m_aMsgFilter.GetCount() - 1; i >= 0; i--)
            {
                IMessageFilter* pMessageFilter = m_aMsgFilter[i];
                if(pMessageFilter != NULL && pMessageFilter->PreTranslateMessage(pMsg))
                    return TRUE;
            }
            return FALSE;   // not translated
        }

        // override to change idle processing
        virtual BOOL OnIdle(int /*nIdleCount*/)
        {
            for(size_t i = 0; i < m_aIdleHandler.GetCount(); i++)
            {
                IIdleHandler* pIdleHandler = m_aIdleHandler[i];
                if(pIdleHandler != NULL)
                    pIdleHandler->OnIdle();
            }
            return FALSE;   // don't continue
        }
        
        int Run()
        {
            BOOL bDoIdle = TRUE;
            int nIdleCount = 0;
            BOOL bRet;

            for(;;)
            {
                while(bDoIdle && !::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE))
                {
                    if(!OnIdle(nIdleCount++))
                        bDoIdle = FALSE;
                }

                bRet = ::GetMessage(&m_msg, NULL, 0, 0);

                if(bRet == -1)
                {
                    STRACE(_T("::GetMessage returned -1 (error)"));
                    continue;   // error, don't process
                }
                else if(!bRet)
                {
                    STRACE(_T("SMessageLoop::Run - exiting"));
                    break;   // WM_QUIT, exit message loop
                }

                if(!PreTranslateMessage(&m_msg))
                {
                    ::TranslateMessage(&m_msg);
                    ::DispatchMessage(&m_msg);
                }

                if(IsIdleMessage(&m_msg))
                {
                    bDoIdle = TRUE;
                    nIdleCount = 0;
                }
            }

            return (int)m_msg.wParam;
        }

    };

}//end of namespace SOUI
