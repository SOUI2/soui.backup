#include "duistd.h"
#include "res.mgr/DuiStylePool.h"

namespace SOUI
{

    // Get style object from pool by class name
    BOOL DuiStylePool::GetStyle(LPCSTR lpszName, DuiStyle& style)
    {
        if(!HasKey(lpszName)) return FALSE;
        style=GetKeyObject(lpszName);
        return TRUE;
    }

    // Load style-pool from xml tree
    BOOL DuiStylePool::Init(pugi::xml_node xmlStyleRoot)
    {
        DUIASSERT(xmlStyleRoot);

        if (strcmp(xmlStyleRoot.name(), "style") != 0)
        {
            DUIASSERT(FALSE);
            return FALSE;
        }

        LPCSTR lpszClassName = NULL;

        for (pugi::xml_node xmlChild=xmlStyleRoot.child("class"); xmlChild; xmlChild=xmlChild.next_sibling("class"))
        {
            lpszClassName = xmlChild.attribute("name").value();
            if (!lpszClassName)
                continue;

            GetKeyObject(lpszClassName).Load(xmlChild);
        }
        return TRUE;
    }
}//end of namespace SOUI
