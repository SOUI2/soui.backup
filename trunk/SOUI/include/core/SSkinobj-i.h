#pragma once

#include "sobject.h"
#include "unknown/obj-ref-i.h"
#include "unknown/obj-ref-impl.hpp"

namespace SOUI
{

class SOUI_EXP ISkinObj : public SObject,public TObjRefImpl2<IObjRef,ISkinObj>
{
public:
    ISkinObj()
    {
    }
    virtual ~ISkinObj()
    {
    }

    void SetOwner(SStringW strOwner)
    {
        m_strOwner=strOwner;
    }

    SStringW GetOwner()
    {
        return m_strOwner;
    }

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
    SStringW	m_strOwner;
};



}//namespace SOUI
