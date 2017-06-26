/*
 * RichEditUnitConverter.h
 * RichEdit的字体大小单位是 twpis
 * 该文件提供一些pt,px,twips之间的转换工具函数
*/

#pragma once

class RichEditUintConverter
{
public:

    static BOOL GetDPI(UINT &dpi, BOOL bIsHeightPx = TRUE);
    static BOOL PointToPixel(FLOAT pt, FLOAT &px);
    static void PointToPixel(FLOAT pt, UINT dpi, FLOAT &px);
    static BOOL PixelToPoint(FLOAT px, FLOAT &pt);
    static void PixelToPoint(FLOAT px, UINT dpi, FLOAT &pt);
    static BOOL PixelToTwips(FLOAT px, FLOAT &twips);
    static void PixelToTwips(FLOAT px, UINT dpi, FLOAT &twips);
};
