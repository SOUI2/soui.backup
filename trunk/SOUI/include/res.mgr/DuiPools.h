#pragma once

#include "res.mgr/DuiSkinPool.h"
#include "res.mgr/DuiCSS.h"
#include "res.mgr/duistringpool.h"
#include "res.mgr/DuiStylePool.h"

namespace SOUI
{
    class SOUI_EXP SPools : public DuiSkinPool
                            , public DuiStringPool
                            , public DuiCSS
                            , public DuiStylePool
    {
    public:
        SPools();
        virtual ~SPools(void);

    public:
        void Init(LPCTSTR pszInitXml,LPCTSTR pszResType);
        
        void Init(pugi::xml_node xmlNode);

        void Clear();
    };

}//end of namespace
