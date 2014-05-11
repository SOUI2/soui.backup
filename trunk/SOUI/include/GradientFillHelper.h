#pragma once

namespace SOUI
{

SOUI_EXP void GradientFillRectV(HDC hdc,const CRect &rcFill, COLORREF crTop, COLORREF crBottom,BYTE byAlpha);

SOUI_EXP void GradientFillRectH(HDC hdc,const CRect &rcFill, COLORREF crLeft, COLORREF crRight,BYTE byAlpha);
SOUI_EXP void GradientFillRect(HDC hdc, const CRect &rcFill, COLORREF cr1, COLORREF cr2,BOOL bVert,BYTE byAlpha);
}//namespace SOUI