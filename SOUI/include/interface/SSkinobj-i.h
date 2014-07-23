#pragma once

#include "sobject.h"
#include "unknown/obj-ref-i.h"
#include "unknown/obj-ref-impl.hpp"

namespace SOUI
{

class SOUI_EXP ISkinObj : public SObject,public TObjRefImpl2<IObjRef,ISkinObj>
{
public:
    ISkinObj():m_dwOwnerID(0)
    {
    }
    virtual ~ISkinObj()
    {
    }

    void SetOwnerID(DWORD dwOwnerID){m_dwOwnerID=dwOwnerID;}
    DWORD GetOwnerID(){return m_dwOwnerID;}

    virtual void Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha=0xFF)=0;

    virtual SIZE GetSkinSize()
    {
        SIZE ret = {0, 0};

        return ret;
    }

    virtual BOOL IgnoreState()
    {
        return TRUE;
    }

    virtual int GetStates()
    {
        return 1;
    }
protected:
    DWORD   m_dwOwnerID;
};



}//namespace SOUI
