#pragma once
#include <richedit.h>
#include "wtl.mini\souimisc.h"

class IRichEditObjHost
{
public:
    virtual ISwndContainer * GetHostContainer() = 0;
    virtual CRect   GetHostRect() = 0;
    virtual CRect   GetAdjustedRect() = 0;
    virtual int     GetContentLength() = 0;
    virtual void    DirectDraw(const CRect& rc) = 0;
    virtual void    DelayDraw(const CRect& rc) = 0;
    virtual HRESULT SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pRet=NULL) = 0;
    virtual ITextDocument* GetTextDoc() = 0;
};
