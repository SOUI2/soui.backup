#pragma once

#include "res.mgr/SSkinPool.h"
#include "res.mgr/SObjDefAttr.h"
#include "res.mgr/Sstringpool.h"
#include "res.mgr/SStylePool.h"

namespace SOUI
{
    class SOUI_EXP SPools : public SSkinPool
                            , public SStringPool
                            , public SObjDefAttr
                            , public SStylePool
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
