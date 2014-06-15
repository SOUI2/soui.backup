#include "duistd.h"
#include "DuiMenu.h"
#include "DuiSystem.h"
#include "gdialpha.h"
#include "mybuffer.h"

namespace SOUI
{

CDuiMenuAttr::CDuiMenuAttr()
    :m_pItemSkin(NULL)
    ,m_pIconSkin(NULL)
    ,m_pSepSkin(NULL)
    ,m_pCheckSkin(NULL)
    ,m_hFont(0)
    ,m_nItemHei(0)
    ,m_nIconMargin(2)
    ,m_nTextMargin(5)
    ,m_szIcon(CX_ICON,CY_ICON)
{
    m_crTxtNormal=GetSysColor(COLOR_MENUTEXT);
    m_crTxtSel=GetSysColor(COLOR_HIGHLIGHTTEXT);
    m_crTxtGray=GetSysColor(COLOR_GRAYTEXT);
}

void CDuiMenuAttr::OnAttributeFinish( pugi::xml_node xmlNode )
{
    DUIASSERT(m_pItemSkin);
    if(m_nItemHei==0) m_nItemHei=m_pItemSkin->GetSkinSize().cy;
    if(!m_hFont) m_hFont=DuiFontPool::getSingleton().GetFont(DUIF_DEFAULTFONT);
}
//////////////////////////////////////////////////////////////////////////

CDuiMenuODWnd::CDuiMenuODWnd(HWND hMenuOwner):m_hMenuOwner(hMenuOwner)
{

}

void CDuiMenuODWnd::OnInitMenu( HMENU menu )
{
    ::SendMessage(m_hMenuOwner,WM_INITMENU,(WPARAM)menu,0);
}

void CDuiMenuODWnd::OnInitMenuPopup( HMENU menuPopup, UINT nIndex, BOOL bSysMenu )
{
    ::SendMessage(m_hMenuOwner,WM_INITMENUPOPUP,(WPARAM)menuPopup,MAKELPARAM(nIndex,bSysMenu));
}

void CDuiMenuODWnd::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
    CRect rcItem=lpDrawItemStruct->rcItem;
    DuiMenuItemData *pdmmi=(DuiMenuItemData*)lpDrawItemStruct->itemData;

    CDCHandle dc(lpDrawItemStruct->hDC);
    CDCHandle dcMem;
    dcMem.CreateCompatibleDC(dc);
    CBitmap      bmp=CGdiAlpha::CreateBitmap32(dc,rcItem.Width(),rcItem.Height());
    CBitmapHandle hOldBmp=dcMem.SelectBitmap(bmp);
    dcMem.BitBlt(0,0,rcItem.Width(),rcItem.Height(),dc,rcItem.left,rcItem.top,SRCCOPY);
    rcItem.MoveToXY(0,0);

    if(pdmmi)
    {
        MENUITEMINFO mii= {sizeof(MENUITEMINFO),MIIM_FTYPE,0};
        HMENU menuPopup=pdmmi->hMenu;
        GetMenuItemInfo(menuPopup,pdmmi->nID,FALSE,&mii);

        BOOL bDisabled = lpDrawItemStruct->itemState & ODS_GRAYED;
        BOOL bSelected = lpDrawItemStruct->itemState & ODS_SELECTED;
        BOOL bChecked = lpDrawItemStruct->itemState & ODS_CHECKED;
        BOOL bRadio = mii.fType&MFT_RADIOCHECK;

        m_pItemSkin->Draw(dcMem,rcItem,bSelected?1:0);    //draw back

        //draw icon
        CRect rcIcon;
        rcIcon.left=rcItem.left+m_nIconMargin;
        rcIcon.right=rcIcon.left+m_szIcon.cx;
        rcIcon.top=rcItem.top+(rcItem.Height()-m_szIcon.cy)/2;
        rcIcon.bottom=rcIcon.top+m_szIcon.cy;
        if(bChecked)
        {
            if(m_pCheckSkin)
            {
                if(bRadio) m_pCheckSkin->Draw(dcMem,rcIcon,1);
                else m_pCheckSkin->Draw(dcMem,rcIcon,0);
            }
        }
        else if(pdmmi->itemInfo.iIcon!=-1 && m_pIconSkin)
        {
            m_pIconSkin->Draw(dcMem,rcIcon,pdmmi->itemInfo.iIcon);
        }
        rcItem.left=rcIcon.right+m_nIconMargin;

        //draw text
        CRect rcTxt=rcItem;
        rcTxt.DeflateRect(m_nTextMargin,0);
        dcMem.SetBkMode(TRANSPARENT);

        COLORREF crOld=dcMem.SetTextColor(bDisabled?m_crTxtGray:(bSelected?m_crTxtSel:m_crTxtNormal));


        HFONT hOldFont=0;
        hOldFont=dcMem.SelectFont(m_hFont);
        dcMem.DrawText(pdmmi->itemInfo.strText,pdmmi->itemInfo.strText.GetLength(),&rcTxt,DT_SINGLELINE|DT_VCENTER|DT_LEFT);
        dcMem.SelectFont(hOldFont);

        dcMem.SetTextColor(crOld);

        if(bSelected && m_pItemSkin->GetStates()>2)
        {
            //draw select mask
            CRect rcItem=lpDrawItemStruct->rcItem;
            rcItem.MoveToXY(0,0);
            m_pItemSkin->Draw(dcMem,rcItem,2);
        }
    }
    else  //if(strcmp("sep",pXmlItem->Value())==0)
    {
        m_pItemSkin->Draw(dcMem,rcItem,0);    //draw back
        if(m_pIconSkin)
        {
            rcItem.left += m_pIconSkin->GetSkinSize().cx+m_nIconMargin*2;
        }

        if(m_pSepSkin)
            m_pSepSkin->Draw(dcMem,&rcItem,0);
        else
        {
            CGdiAlpha::DrawLine(dcMem, rcItem.left, rcItem.top, rcItem.right, rcItem.top, RGB(196,196,196), PS_SOLID);
            CGdiAlpha::DrawLine(dcMem, rcItem.left, rcItem.top+1, rcItem.right, rcItem.top+1, RGB(255,255,255), PS_SOLID);
        }
    }
    rcItem=lpDrawItemStruct->rcItem;
    dc.BitBlt(rcItem.left,rcItem.top,rcItem.Width(),rcItem.Height(),dcMem,0,0,SRCCOPY);
    dcMem.SelectBitmap(hOldBmp);
    dcMem.DeleteDC();
    bmp.DeleteObject();
}

void CDuiMenuODWnd::MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct )
{
    if(lpMeasureItemStruct->CtlType != ODT_MENU) return;

    DuiMenuItemData *pdmmi=(DuiMenuItemData*)lpMeasureItemStruct->itemData;
    if(pdmmi)
    {
        //menu item
        lpMeasureItemStruct->itemHeight = m_nItemHei;
        lpMeasureItemStruct->itemWidth = m_szIcon.cx+m_nIconMargin*2;

        CDCHandle dc=::GetDC(NULL);
        HFONT hOldFont=0;
        hOldFont=dc.SelectFont(m_hFont);
        SIZE szTxt;
        dc.GetTextExtent(pdmmi->itemInfo.strText,pdmmi->itemInfo.strText.GetLength(),&szTxt);
        lpMeasureItemStruct->itemWidth += szTxt.cx+m_nTextMargin*2;
        dc.SelectFont(hOldFont);
        ::ReleaseDC(NULL,dc);
    }
    else
    {
        // separator
        lpMeasureItemStruct->itemHeight = m_pSepSkin?m_pSepSkin->GetSkinSize().cy:3;
        lpMeasureItemStruct->itemWidth=0;
    }

}

void CDuiMenuODWnd::OnMenuSelect( UINT nItemID, UINT nFlags, HMENU menu )
{
    ::SendMessage(m_hMenuOwner,WM_MENUSELECT,MAKEWPARAM(nItemID,nFlags),(LPARAM)menu);
}

//////////////////////////////////////////////////////////////////////////

CDuiMenu::CDuiMenu():m_pParent(NULL),m_hMenu(0)
{
}

CDuiMenu::CDuiMenu( const CDuiMenu & src )
{
    m_pParent=src.m_pParent;
    m_hMenu=src.m_hMenu;
    m_menuSkin=src.m_menuSkin;    
}

CDuiMenu::~CDuiMenu(void)
{
    DestroyMenu();
}

BOOL CDuiMenu::LoadMenu( LPCTSTR pszResName )
{
    if(::IsMenu(m_hMenu)) return FALSE;

    pugi::xml_document xmlDoc;
    if(!LOADXML(xmlDoc,pszResName,DUIRES_XML_TYPE)) return FALSE;

    pugi::xml_node xmlMenu=xmlDoc.child("menu");
    if(!xmlMenu)  return FALSE;

    return LoadMenu(xmlMenu);
}


BOOL CDuiMenu::LoadMenu( pugi::xml_node xmlMenu )
{
    m_hMenu=CreatePopupMenu();
    if(!m_hMenu) return FALSE;

    m_menuSkin.Load(xmlMenu);
    DUIASSERT(m_menuSkin.m_pItemSkin);

    BuildMenu(m_hMenu,xmlMenu);

    return TRUE;
}

CDuiMenu CDuiMenu::GetSubMenu(int nPos)
{
    HMENU hSubMenu=::GetSubMenu(m_hMenu,nPos);
    CDuiMenu ret;
    ret.m_pParent=this;
    ret.m_hMenu=hSubMenu;
    ret.m_menuSkin=m_menuSkin;
    return ret;
}

BOOL CDuiMenu::InsertMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem,LPCTSTR strText, int iIcon)
{
    nFlags|=MF_OWNERDRAW;
    if(nFlags&MF_SEPARATOR)
    {
        return ::InsertMenu(m_hMenu,nPosition,nFlags,(UINT_PTR)0,(LPCTSTR)NULL);
    }

    DuiMenuItemData *pMenuData=new DuiMenuItemData;
    pMenuData->hMenu=m_hMenu;
    pMenuData->itemInfo.iIcon=iIcon;
    pMenuData->itemInfo.strText=strText;

    if(nFlags&MF_POPUP)
    {
        //插入子菜单，
        CDuiMenu *pSubMenu=(CDuiMenu*)(LPVOID)nIDNewItem;
        DUIASSERT(pSubMenu->m_pParent==NULL);
        pMenuData->nID=(UINT_PTR)pSubMenu->m_hMenu;
    }
    else
    {
        pMenuData->nID=nIDNewItem;
    }

    if(!::InsertMenu(m_hMenu,nPosition,nFlags,pMenuData->nID,(LPCTSTR)pMenuData))
    {
        delete pMenuData;
        return FALSE;
    }

    CDuiMenu *pRootMenu=this;
    while(pRootMenu->m_pParent) pRootMenu=pRootMenu->m_pParent;
    //将分配的内存放到根菜单的内存节点列表中
    pRootMenu->m_arrDmmi.Add(pMenuData);

    if(nFlags&MF_POPUP)
    {
        //对子菜单还需要做数据迁移处理
        CDuiMenu *pSubMenu=(CDuiMenu*)(LPVOID)nIDNewItem;
        for(UINT i=0; i<pSubMenu->m_arrDmmi.GetCount(); i++)
            pRootMenu->m_arrDmmi.Add(pSubMenu->m_arrDmmi[i]);
        pSubMenu->m_arrDmmi.RemoveAll();
        pSubMenu->m_pParent=this;
    }
    return TRUE;
}

UINT CDuiMenu::TrackPopupMenu(
    UINT uFlags,
    int x,
    int y,
    HWND hWnd,
    LPCRECT prcRect
)
{
    DUIASSERT(IsMenu(m_hMenu));

    CDuiMenuODWnd menuOwner(hWnd);
    *(static_cast<CDuiMenuAttr*>(&menuOwner))=m_menuSkin;
    menuOwner.Create(NULL,WS_POPUP,WS_EX_NOACTIVATE,0,0,0,0,NULL,NULL);
    UINT uNewFlags=uFlags|TPM_RETURNCMD;
    UINT uRet=::TrackPopupMenu(m_hMenu,uNewFlags,x,y,0,menuOwner.m_hWnd,prcRect);
    menuOwner.DestroyWindow();
    if(uRet && !(uFlags&TPM_RETURNCMD)) ::SendMessage(hWnd,WM_COMMAND,uRet,0);
    return uRet;
}

void CDuiMenu::BuildMenu( HMENU menuPopup,pugi::xml_node xmlNode )
{
    pugi::xml_node xmlItem=xmlNode.first_child();

    while(xmlItem)
    {
        if(strcmp("item",xmlItem.name())==0)
        {
            DuiMenuItemData *pdmmi=new DuiMenuItemData;
            pdmmi->hMenu=menuPopup;
            pdmmi->itemInfo.iIcon=xmlItem.attribute("icon").as_int(-1);
            pdmmi->itemInfo.strText=DUI_CA2T(xmlItem.text().get(),CP_UTF8);

            int nID=xmlItem.attribute("id").as_int(0);
            BOOL bCheck=xmlItem.attribute("check").as_bool(false);
            BOOL bRadio=xmlItem.attribute("radio").as_bool(false);
            BOOL bDisable=xmlItem.attribute("disable").as_bool(false);


            pugi::xml_writer_buff writer;
            xmlItem.print(writer);
            CDuiStringW str=DUI_CA2W(CDuiStringA(writer.buffer(),writer.size()),CP_UTF8);

            pugi::xml_node xmlChild=xmlItem.first_child();
            while(xmlChild && xmlChild.type()==pugi::node_pcdata) xmlChild=xmlChild.next_sibling();


            if(!xmlChild)
            {
                pdmmi->nID=nID;
                UINT uFlag=MF_OWNERDRAW;
                if(bCheck) uFlag|=MF_CHECKED;
                if(bDisable) uFlag |= MF_GRAYED;
                if(bRadio) uFlag |= MFT_RADIOCHECK|MF_CHECKED;
                AppendMenu(menuPopup,uFlag,(UINT_PTR)pdmmi->nID,(LPCTSTR)pdmmi);
            }
            else
            {
                HMENU hSubMenu=::CreatePopupMenu();
                pdmmi->nID=(UINT_PTR)hSubMenu;
                UINT uFlag=MF_OWNERDRAW|MF_POPUP;
                if(bDisable) uFlag |= MF_GRAYED;
                AppendMenu(menuPopup,uFlag,(UINT_PTR)hSubMenu,(LPCTSTR)pdmmi);
                BuildMenu(hSubMenu,xmlItem);//build sub menu
            }
            m_arrDmmi.Add(pdmmi);
        }
        else if(strcmp("sep",xmlItem.name())==0)
        {
            AppendMenu(menuPopup,MF_SEPARATOR|MF_OWNERDRAW,(UINT_PTR)0,(LPCTSTR)NULL);
        }
        xmlItem=xmlItem.next_sibling();
    }
}

void CDuiMenu::DestroyMenu()
{
    if(!m_pParent)
    {
        if(m_hMenu) ::DestroyMenu(m_hMenu);
        for(UINT i=0; i<m_arrDmmi.GetCount(); i++) 
            delete m_arrDmmi[i];
        m_arrDmmi.RemoveAll();
    }
}

}//namespace SOUI