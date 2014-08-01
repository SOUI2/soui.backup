#pragma once
#include "core/SSingletonMap.h"
#include "interface/Sskinobj-i.h"
#include <unknown/obj-ref-i.h>
#include <unknown/obj-ref-impl.hpp>

#define GETSKIN(p1) SSkinPoolMgr::getSingleton().GetSkin(p1)
#define GETBUILTINSKIN(p1) SSkinPoolMgr::getSingleton().GetBuiltinSkin(p1)

namespace SOUI
{
    //系统内置皮肤名称
    enum SYS_SKIN
    {
        SKIN_SYS_CHECKBOX=0,            //L"_skin.sys.checkbox",
        SKIN_SYS_RADIO,                 //L"_skin.sys.radio",
        SKIN_SYS_FOCUSCHECKBOX,         //L"_skin.sys.focuscheckbox",
        SKIN_SYS_FOCUSRADIO,            //L"_skin.sys.focusradio",
        SKIN_SYS_BTN_NORMAL,            //L"_skin.sys.btn.normal",
        SKIN_SYS_SCROLLBAR,             //L"_skin.sys.scrollbar",
        SKIN_SYS_BORDER,                //L"_skin.sys.border",
        SKIN_SYS_DROPBTN,               //L"_skin.sys.dropbtn",
        SKIN_SYS_TREE_TOGGLE,           //L"_skin.sys.tree.toggle",
        SKIN_SYS_TREE_CHECKBOX,         //L"_skin.sys.tree.checkbox"
        SKIN_SYS_TAB_PAGE,              //L"_skin.sys.tab.page",
        SKIN_SYS_HEADER,                //L"_skin.sys.header"
        SKIN_SYS_SPLIT_VERT,            //L"_skin.sys.split.vert",
        SKIN_SYS_SPLIT_HORZ,            //L"_skin.sys.split.horz",
        SKIN_SYS_PROG_BKGND,            //L"_skin.prog.bkgnd",
        SKIN_SYS_PROG_BAR,              //L"_skin.prog.bar",
        SKIN_SYS_SLIDER_THUMB,          //L"_skin.slider.thumb",

        SKIN_SYS_BTN_CLOSE,             //L"_skin.sys.btn.close",
        SKIN_SYS_BTN_MINIMIZE,          //L"_skin.sys.btn.minimize",
        SKIN_SYS_BTN_MAXMIZE,           //L"_skin.sys.btn.maxmize",
        SKIN_SYS_BTN_RESTORE,           //L"_skin.sys.btn.restore",

        SKIN_SYS_MENU_CHECK,            //L"_skin.sys.menu.check",
        SKIN_SYS_MENU_SEP,              //L"_skin.sys.menu.sep",
        SKIN_SYS_MENU_BORDER,           //L"_skin.sys.menu.border",
        SKIN_SYS_MENU_SKIN,             //L"_skin.sys.menu.skin",
        SKIN_SYS_ICONS,                 //L"_skin.sys.icons",
        SKIN_SYS_WND_BKGND,             //L"_skin.sys.wnd.bkgnd",
        
        SKIN_SYS_COUNT,
    };

typedef ISkinObj * SSkinPtr;
class SOUI_EXP SSkinPool :public SCmnMap<SSkinPtr,SStringW>, public TObjRefImpl2<IObjRef,SSkinPool>
{
public:
    SSkinPool();

    virtual ~SSkinPool();

    ISkinObj* GetSkin(LPCWSTR strSkinName);

    int LoadSkins(pugi::xml_node xmlNode);
   

protected:
    static void OnKeyRemoved(const SSkinPtr & obj);
};

class SOUI_EXP SSkinPoolMgr : public SSingleton<SSkinPoolMgr> 
{
public:
    SSkinPoolMgr();
    ~SSkinPoolMgr();

    ISkinObj* GetSkin(LPCWSTR strSkinName);
    
    void PushSkinPool(SSkinPool *pSkinPool);

    SSkinPool * PopSkinPool();

    ISkinObj * GetBuiltinSkin(SYS_SKIN uID);
    
    SSkinPool * GetBuiltinSkinPool(){return m_bulitinSkinPool;}
protected:
    SList<SSkinPool *> m_lstSkinPools;
    CAutoRefPtr<SSkinPool> m_bulitinSkinPool;

};


}//namespace SOUI