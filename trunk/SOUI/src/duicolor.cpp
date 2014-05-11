//////////////////////////////////////////////////////////////////////////
//   File Name: bkcolor.h
// Description: HLS & RGB processor
//     Creator: Zhang Xiaoxuan
//     Version: 2009.6.10 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#include "duistd.h"
#include "duicolor.h"


namespace SOUI
{

DuiColor::DuiColor()
    : red(0)
    , green(0)
    , blue(0)
    , lightness(0)
    , saturation(0)
    , hue(0)
{
}

DuiColor::DuiColor(double h, double l, double s)
{
    hue		   = h;
    lightness  = l;
    saturation = s;

    ToRGB();
}

DuiColor::DuiColor (BYTE r, BYTE g, BYTE b)
{
    red   = r;
    green = g;
    blue  = b;

    ToHLS();
}

DuiColor::DuiColor(COLORREF color)
{
    red   = GetRValue(color);
    green = GetGValue(color);
    blue  = GetBValue(color);

    ToHLS();
}

// lightness  [0..1]
// saturation [0..1]
// hue		  [0..360)
void DuiColor::ToHLS(void)
{
    double mn, mx;
    int	   major;

    if ( red < green )
    {
        mn = red;
        mx = green;
        major = Green;
    }
    else
    {
        mn = green;
        mx = red;
        major = Red;
    }

    if ( blue < mn )
        mn = blue;
    else if ( blue > mx )
    {
        mx = blue;
        major = Blue;
    }

    if ( mn==mx )
    {
        lightness    = mn/255;
        saturation   = 0;
        hue          = 0;
    }
    else
    {
        lightness = (mn+mx) / 510;

        if ( lightness <= 0.5 )
            saturation = (mx-mn) / (mn+mx);
        else
            saturation = (mx-mn) / (510-mn-mx);

        switch ( major )
        {
        case Red  :
            hue = (green-blue) * 60 / (mx-mn) + 360;
            break;
        case Green:
            hue = (blue-red) * 60  / (mx-mn) + 120;
            break;
        case Blue :
            hue = (red-green) * 60 / (mx-mn) + 240;
        }

        if (hue >= 360)
            hue = hue - 360;
    }
}

void DuiColor::ToRGB(void)
{
    // lightness  [0..1]
    // saturation [0..1]
    // hue		  [0..360)

    lightness = max(0, min(1, lightness));
    saturation = max(0, min(1, saturation));
    hue = max(0, min(360, hue));
    if (360 == hue)
        hue = 0;

    if (saturation == 0)
    {
        red = green = blue = (unsigned char) (lightness*255);
    }
    else
    {
        double m1, m2;

        if ( lightness <= 0.5 )
            m2 = lightness + lightness * saturation;
        else
            m2 = lightness + saturation - lightness * saturation;

        m1 = 2 * lightness - m2;

        red   = _Value(m1, m2, hue + 120);
        green = _Value(m1, m2, hue);
        blue  = _Value(m1, m2, hue - 120);
    }
}

DuiColor::operator COLORREF() const throw()
{
    return RGB(red, green, blue);
}


unsigned char DuiColor::_Value(double m1, double m2, double h)
{
    while (h >= 360) h -= 360;
    while (h <    0) h += 360;

    if (h < 60)
        m1 = m1 + (m2 - m1) * h / 60;
    else if (h < 180)
        m1 = m2;
    else if (h < 240)
        m1 = m1 + (m2 - m1) * (240 - h) / 60;

    return (unsigned char)(m1 * 255);
}

}//end