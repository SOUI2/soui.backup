#pragma once

namespace SOUI
{

void GradientFillRectV(HDC hdc,const RECT *rcFill, COLORREF crTop, COLORREF crBottom);

void GradientFillRectH(HDC hdc,const RECT *rcFill, COLORREF crLeft, COLORREF crRight);
void GradientFillRect(HDC hdc, const RECT *rcFill, COLORREF cr1, COLORREF cr2,BOOL bVert);
}//namespace SOUI