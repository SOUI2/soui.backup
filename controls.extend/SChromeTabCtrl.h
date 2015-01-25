#pragma once

#include <core/Swnd.h>
#include "SAnimator.h"

namespace SOUI
{
    
#define EVT_CHROMETAB_BEGIN (EVT_EXTERNAL_BEGIN + 400)
#define EVT_CHROMETAB_NEW           (EVT_CHROMETAB_BEGIN + 0)
#define EVT_CHROMETAB_CLOSE         (EVT_CHROMETAB_BEGIN + 1)
#define EVT_CHROMETAB_SELCHANGED    (EVT_CHROMETAB_BEGIN + 2)

    class EventChromeTabNew : public EventArgs
    {
    public:
        EventChromeTabNew(SWindow *pSender):EventArgs(pSender)
        {

        }

        virtual UINT GetEventID(){return EVT_CHROMETAB_NEW;}

        SWindow * pNewTab;

        int       iNewTab;
    };

    class EventChromeTabClose : public EventArgs
    {
    public:
        EventChromeTabClose(SWindow *pSender):EventArgs(pSender)
        {

        }

        virtual UINT GetEventID(){return EVT_CHROMETAB_CLOSE;}

        SWindow * pCloseTab;

        int       iCloseTab;
    };

    class EventChromeTabSelChanged : public EventArgs
    {
    public:
        EventChromeTabSelChanged(SWindow *pSender):EventArgs(pSender)
        {

        }

        virtual UINT GetEventID(){return EVT_CHROMETAB_CLOSE;}

        int         iOldSel;
        int         iNewSel;
    };

    class SChromeTab;
    class EventTabNew : public EventArgs
    {
    public:
        EventTabNew(SWindow *_pNewTab,SWindow *pSender):EventArgs(pSender),pNewTab(_pNewTab)
        {

        }

        virtual UINT GetEventID(){return EVT_CHROMETAB_NEW;}

        SWindow * pNewTab;
    };

    class SChromeTabCtrl : public SWindow, public ITimelineHandler
    {
        SOUI_CLASS_NAME(SChromeTabCtrl,L"chromeTabCtrl")
    public:
        enum TABDIR{
            TDIR_HORZ,
            TDIR_VERT,
        };

        SChromeTabCtrl(void);
        ~SChromeTabCtrl(void);
        
        BOOL InsertTab(LPCTSTR pszTitle,int iPos = -1);

    protected:
        virtual BOOL CreateChildren(pugi::xml_node xmlNode);
        virtual void UpdateChildrenPosition();
        
        virtual void OnNextFrame();

        bool OnBtnNewClick(EventArgs *pEvt);
        bool OnBtnCloseTabClick(EventArgs *pEvt);
        bool OnTabClick(EventArgs *pEvt);
    
        int GetTabIndex(const SChromeTab* pTab)const;

        int OnCreate(LPVOID);
        void OnDestroy();

        SOUI_MSG_MAP_BEGIN()
            MSG_WM_CREATE(OnCreate)
            MSG_WM_DESTROY(OnDestroy)
        SOUI_MSG_MAP_END()

        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"tabDesiredSize",m_nDesiredSize,FALSE)
            ATTR_ENUM_BEGIN(L"tabAlign",TABDIR,FALSE)
                ATTR_ENUM_VALUE(L"vertical",TDIR_VERT)
                ATTR_ENUM_VALUE(L"horizontal",TDIR_HORZ)
            ATTR_ENUM_END(m_tabAlign)
        SOUI_ATTRS_END()

        int     m_nDesiredSize;
        TABDIR  m_tabAlign;
        SArray<SChromeTab*> m_lstTab;

        SChromeTab *        m_pBtnNew;

        pugi::xml_document  m_xmlStyle;

        int                 m_iCurSel;
    };

}
