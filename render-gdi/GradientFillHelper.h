#pragma once

namespace SOUI
{

void GradientFillRectV(HDC hdc,const RECT *rcFill, COLORREF crTop, COLORREF crBottom,BYTE byAlpha);

void GradientFillRectH(HDC hdc,const RECT *rcFill, COLORREF crLeft, COLORREF crRight,BYTE byAlpha);
void GradientFillRect(HDC hdc, const RECT *rcFill, COLORREF cr1, COLORREF cr2,BOOL bVert,BYTE byAlpha);
}//namespace SOUI