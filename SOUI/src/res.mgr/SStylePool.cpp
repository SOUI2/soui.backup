#include "souistd.h"
#include "res.mgr/SStylePool.h"

namespace SOUI
{

    // Get style object from pool by class name
    BOOL SStylePool::GetStyle(LPCWSTR lpszName, SwndStyle& style)
    {
        if(!HasKey(lpszName)) return FALSE;
        style=GetKeyObject(lpszName);
        return TRUE;
    }

    // Load style-pool from xml tree
    BOOL SStylePool::Init(pugi::xml_node xmlStyleRoot)
    {
        ASSERT(xmlStyleRoot);

        if (wcscmp(xmlStyleRoot.name(), L"style") != 0)
        {
            ASSERT(FALSE);
            return FALSE;
        }

        LPCWSTR lpszClassName = NULL;

        for (pugi::xml_node xmlChild=xmlStyleRoot.child(L"class"); xmlChild; xmlChild=xmlChild.next_sibling(L"class"))
        {
            lpszClassName = xmlChild.attribute(L"name").value();
            if (!lpszClassName)
                continue;

            GetKeyObject(lpszClassName).InitFromXml(xmlChild);
        }
        return TRUE;
    }
}//end of namespace SOUI
