#pragma once

#include "SWnd.h"

namespace SOUI
{
    class SOUI_EXP SRootWindow : public SWindow
    {
    public:
        SRootWindow(void);
        ~SRootWindow(void);
        
        virtual BOOL InitFromXml(pugi::xml_node xmlNode);

        SOUI_ATTRS_BEGIN()
            ATTR_STRINGW(L"title",m_strTitle,FALSE)
             ATTR_SIZE(L"size",m_szInit,FALSE)
             ATTR_INT(L"width",m_szInit.cx,FALSE)
             ATTR_INT(L"height",m_szInit.cy,FALSE)
             ATTR_RECT(L"margin",m_rcMargin,FALSE)
             ATTR_SIZE(L"minsize",m_szMin,FALSE)
             ATTR_DWORD(L"wndstyle",m_dwStyle,FALSE)
             ATTR_DWORD(L"wndstyleex",m_dwExStyle,FALSE)
             ATTR_INT(L"resizable",m_bResizable,FALSE)
             ATTR_INT(L"translucent",m_bTranslucent,FALSE)
             ATTR_INT(L"appwnd",m_bAppWnd,FALSE)
             ATTR_INT(L"toolwindow",m_bToolWnd,FALSE)
        SOUI_ATTRS_QUIT()//不把属性交给SWindow处理
        
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_DESTROY(OnDestroy)
        SOUI_MSG_MAP_END()
    protected:
        void OnDestroy();
        void Cleanup();
        
        CRect m_rcMargin;
        CSize m_szMin;
        CSize m_szInit;
        
        BOOL m_bResizable;
        BOOL m_bAppWnd;
        BOOL m_bToolWnd;
        BOOL m_bTranslucent;    //窗口的半透明属性
        
        DWORD m_dwStyle;
        DWORD m_dwExStyle;
        
        SStringW m_strTitle;
    };

}
