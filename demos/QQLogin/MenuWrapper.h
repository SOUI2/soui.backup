#pragma once
#include <vector>
#include "string\tstring.h"
#include "pugixml\pugixml.hpp"

namespace SOUI
{

    class MenuItemWrapper
    {
    public:

        MenuItemWrapper(pugi::xml_node node);

        void SetText(const SStringW& str);
        void SetId(int id);
        void SetEnable(BOOL enable);
        void SetCheck(BOOL check);
        void SetFont(const SStringW& font);
        int  GetId();

    protected:

        pugi::xml_attribute AppendItemAttr(LPCWSTR lpszAttr);

    private:
        pugi::xml_node  _menuItemNode;
    };

    class MenuWrapper
    {
    public:

        MenuWrapper(const SStringW& xml, const SStringW& type);
        ~MenuWrapper();

        int ShowMenu(int op, int x, int y, HWND hHost);
        MenuItemWrapper* AddMenu(const SStringW& text, int id, BOOL bEnable = TRUE, BOOL bCheck = FALSE);
        void AddSeperator();
        MenuItemWrapper* GetMenuItemById(int id);
        void RemoveItem(int id);
        void SetItemSkin(const SStringW& skin);
        void SetCheckSkin(const SStringW& skin);
        void SetSepSkin(const SStringW& skin);
        void SetArrowSkin(const SStringW& skin);
        void SetItemHeight(int height);
        void SetIconPos(int pos);
        void SetMargin(const CRect& rc);
        void SetIconBarWidht(int width);
        void SetTextOffset(int offset);
        void SetMinWidth(int width);

    protected:

        pugi::xml_node GetNodeById(pugi::xml_node node, int id);

    private:

        typedef std::vector<MenuItemWrapper> VecMenuItems;

        pugi::xml_document  _menuDoc;
        pugi::xml_node      _menuRoot;
        VecMenuItems        _menuItems;
    };

}; // namespace SOUI
