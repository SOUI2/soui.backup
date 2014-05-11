//////////////////////////////////////////////////////////////////////////
//   File Name: bkcolor.h
// Description: HLS & RGB processor
//     Creator: Zhang Xiaoxuan
//     Version: 2009.6.10 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

namespace SOUI
{

class SOUI_EXP DuiColor
{
    typedef enum
    {
        Red,
        Green,
        Blue
    };

public:

    unsigned char red;
    unsigned char green;
    unsigned char blue;

    double lightness, saturation, hue;

    DuiColor();

    DuiColor(double h, double l, double s);

    DuiColor (BYTE r, BYTE g, BYTE b);

    DuiColor(COLORREF color);

    // lightness  [0..1]
    // saturation [0..1]
    // hue		  [0..360)
    void ToHLS(void);

    void ToRGB(void);

    operator COLORREF() const throw();

private:

    unsigned char _Value(double m1, double m2, double h);
};


}//end of namespace SOUI