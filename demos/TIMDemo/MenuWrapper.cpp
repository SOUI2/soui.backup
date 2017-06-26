#include "stdafx.h"
#include "souistd.h"
#include "MenuWrapper.h"
#include "helper\smenuex.h"

namespace SOUI
{

    // ------------------------------------------------------------------------------
    //
    // impl MenuItemWrapper
    //
    // ------------------------------------------------------------------------------

    MenuItemWrapper::MenuItemWrapper(pugi::xml_node node) : _menuItemNode(node)
    {
    }

    void MenuItemWrapper::SetText(const SStringW& str)
    {
        if (_menuItemNode)
        {
            _menuItemNode.text().set((LPCWSTR)str);
        }
    }

    pugi::xml_attribute MenuItemWrapper::AppendItemAttr(LPCWSTR lpszAttr)
    {
        pugi::xml_attribute attr;

        if (_menuItemNode)
        {
            attr = _menuItemNode.attribute(lpszAttr);
            if (!attr)
            {
                attr = _menuItemNode.append_attribute(lpszAttr);
            }
        }

        return attr;
    }

    void MenuItemWrapper::SetId(int id)
    {
        AppendItemAttr(L"id").set_value(id);
    }

    void MenuItemWrapper::SetEnable(BOOL bEnable)
    {
        AppendItemAttr(L"enable").set_value(bEnable);
    }

    void MenuItemWrapper::SetCheck(BOOL bCheck)
    {
        AppendItemAttr(L"check").set_value(bCheck);
    }

    void MenuItemWrapper::SetFont(const SStringW& font)
    {
        AppendItemAttr(L"font").set_value((LPCWSTR)font);
    }

    int  MenuItemWrapper::GetId()
    {
        return AppendItemAttr(L"id").as_int(-1);
    }

    // ------------------------------------------------------------------------------
    //
    // impl MenuWrapper
    //
    // ------------------------------------------------------------------------------

    MenuWrapper::MenuWrapper(const SStringW& xml, const SStringW& type)
    {
        LOADXML(_menuDoc, xml, type);
        _menuRoot = _menuDoc.child(L"menuRoot");
    }

    MenuWrapper::~MenuWrapper()
    {
    }

    MenuItemWrapper* MenuWrapper::AddMenu(const SStringW& str,
        int id,
        BOOL bEnable, /*=TRUE*/
        BOOL bCheck)  /*=FALSE*/
    {
        if (!_menuRoot)
        {
            return NULL;
        }

        pugi::xml_node node = _menuRoot.append_child(L"menuItem");

        node.append_attribute(L"colorTextDisable").set_value(L"#c0c0c0");

        MenuItemWrapper menuItem(node);
        menuItem.SetText(str);
        menuItem.SetId(id);
        menuItem.SetEnable(bEnable);
        menuItem.SetCheck(bCheck);
        menuItem.SetFont(L"size:12");

        _menuItems.push_back(menuItem);
        VecMenuItems::iterator it = _menuItems.end() - 1;

        return &(*it);
    }

    MenuItemWrapper* MenuWrapper::GetMenuItemById(int id)
    {
        VecMenuItems::iterator it = _menuItems.begin();

        for (; it != _menuItems.end(); ++it)
        {
            if (it->GetId() == id)
            {
                return &(*it);
            }
        }

        if (_menuRoot)
        {
            pugi::xml_node node = GetNodeById(_menuRoot.first_child(), id);

            if (node)
            {
                MenuItemWrapper menuItem(node);
                _menuItems.push_back(menuItem);

                it = _menuItems.end() - 1;
                return &(*it);
            }
        }

        return NULL;
    }

    void MenuWrapper::RemoveItem(int id)
    {
        VecMenuItems::iterator it = _menuItems.begin();

        for (; it != _menuItems.end(); ++it)
        {
            if (it->GetId() == id)
            {
                _menuItems.erase(it);
                break;
            }
        }

        pugi::xml_node node = GetNodeById(_menuRoot.first_child(), id);
        if (node)
        {
            _menuRoot.remove_child(node);
        }
    }

    void MenuWrapper::SetItemSkin(const SStringW& skin)
    {

    }

    void MenuWrapper::SetCheckSkin(const SStringW& skin)
    {

    }

    int MenuWrapper::ShowMenu(int op, int x, int y, HWND hHost)
    {
        SMenuEx menu;
        menu.LoadMenu(_menuRoot);
        return menu.TrackPopupMenu(op, x, y, hHost);
    }

    pugi::xml_node MenuWrapper::GetNodeById(pugi::xml_node node, int id)
    {
        if (!node)
        {
            return pugi::xml_node();
        }

        if (node.attribute(L"id").as_int(-1) == id)
        {
            return node;
        }

        pugi::xml_node matched;
        pugi::xml_node sibling;

        matched = GetNodeById(node.next_sibling(), id);
        if (matched)
        {
            return matched;
        }

        return GetNodeById(node.child(L"menuItem"), id);
    }

};
