#pragma once

#include <core/shostwnd.h>

namespace SOUI
{
    class SOUI_EXP SMenuEx : protected SHostWnd
    {
    friend class SMenuExItem;
    public:
        SMenuEx(void);
        ~SMenuEx(void);
        
        BOOL LoadMenu(LPCTSTR pszMenu);
        BOOL LoadMenu(pugi::xml_node xmlNode);
        
        UINT TrackPopupMenu(UINT flag,int x,int y,HWND hOwner);
        
    protected:
        int OnMouseActivate(HWND wndTopLevel, UINT nHitTest, UINT message);
        void OnTimer(UINT_PTR timeID);
        
        BEGIN_MSG_MAP_EX(SMenuEx)
            MSG_WM_MOUSEACTIVATE(OnMouseActivate)
            MSG_WM_TIMER(OnTimer)
            CHAIN_MSG_MAP(SHostWnd)
        END_MSG_MAP()
    protected:
        virtual BOOL _HandleEvent(EventArgs *pEvt);
        
        void ShowMenu(UINT uFlag,int x,int y);
        void HideMenu();
        void RunMenu(HWND hOwner);
        
        void PopupSubMenu(SMenuExItem * pItem);
        
        SMenuEx(SMenuEx *pParent);
        
        SMenuEx * m_pParent;
        SMenuExItem * m_pHoverItem;
    };
}
