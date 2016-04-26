#include "souistd.h"
#include "helper/SDIBHelper.h"

namespace SOUI
{
    enum{Red,Green,Blue};

    struct DIBINFO{
        LPBYTE  pBits;
        int     nWid;
        int     nHei;
    };

	// ------------------------------------------------------------
	// 有太多的算法需要用某种方式(map)变换位图的每个像素的颜色，比如
	// 彩色转换为灰度图，gamma校正，颜色空间转换,hsl调整.所以写一个模板做为参数调用的通用算法
	// ------------------------------------------------------------
	template <class Mode, class Param>
	bool ColorTransform(DIBINFO* pDib, Mode mode, const Param &param)
	{
		if (NULL == pDib||NULL == pDib->pBits)
		{
			return false;
		}
        
        int nPixels = pDib->nWid * pDib->nHei;
        LPBYTE pBit = pDib->pBits;
        for(int i=0;i<nPixels;i++, pBit+=4)
        {
            mode(pBit[2], pBit[1], pBit[0], param);
        }

		return true;
	}

	// 灰度 = 0.299 * red + 0.587 * green + 0.114 * blue 
	inline void MaptoGray(BYTE & red, BYTE & green, BYTE & blue, const int &)
	{
		red   = (red * 77 + green * 150 + blue * 29 + 128) / 256;
		green = red;
		blue  = red;
	}

    struct HSL32PARAM
    {
        int degHue;
        int perSaturation;
        int perLuminosity;
    };
    
    inline void AdjustPixelHSL(BYTE & red, BYTE & green, BYTE & blue,const HSL32PARAM & param)
    {
        float H = 0.0f;
        float S = 0.0f;
        float L = 0.0f;	

        SDIBHelper::RGBtoHSL(red,green,blue,
            H, S, L);

        H += param.degHue;
        S = (S*(float)param.perSaturation/100.0f);
        L = (L*(float)param.perLuminosity/100.0f);

        SDIBHelper::HSLtoRGB(H, S, L, red,green,blue);
    }
    
	bool SDIBHelper::AdjustHSL32(IBitmap * pBmp,int degHue,int perSaturation,int perLuminosity)
	{
        DIBINFO di={(LPBYTE)pBmp->LockPixelBits(),pBmp->Width(),pBmp->Height()};

		bool bRet = false;
		do 
		{
			if (perSaturation < 0||perLuminosity < 0)
			{
				break;
			}

			if (degHue == 0 && perSaturation == 100 && perLuminosity == 100)
			{
				bRet = true;// 未作调整，直接返回
				break;
			}
            
            HSL32PARAM param={degHue,perSaturation,perLuminosity};
            bRet =ColorTransform(&di, AdjustPixelHSL,param);
            
		} while (false);
		
		pBmp->UnlockPixelBits(di.pBits);
		return bRet;
	}

    bool SDIBHelper::AdjustHue(IBitmap *pBmp,COLORREF cr)
    {
        float h,s,l;
        BYTE r=GetRValue(cr),g=GetGValue(cr),b=GetBValue(cr);
        RGBtoHSL(r,g,b,h,s,l);
        return AdjustHSL32(pBmp,(int)h,100,100);
    }

	bool SDIBHelper::GrayImage(IBitmap * pBmp)
	{
	    DIBINFO di={(LPBYTE)pBmp->LockPixelBits(),pBmp->Width(),pBmp->Height()};
		bool bRet =ColorTransform(&di, MaptoGray,0);
		pBmp->UnlockPixelBits(di.pBits);
		return bRet;
	}
	

	void SDIBHelper::RGBtoHSL(const BYTE &red, const BYTE &green, const BYTE &blue,
		                      float &hue, float &saturation, float &lightness)
	{
		float mn  = 0.0f;
		float mx  = 0.0f; 
		int	major = Red;

		if (red < green)
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

		if (blue < mn)
		{
			mn = blue;
		}
		else if (blue > mx)
		{
			mx = blue;
			major = Blue;
		}

		if (mn == mx) 
		{
			lightness    = mn/255.0f;
			saturation   = 0;
			hue          = 0; 
		}   
		else 
		{ 
			lightness = (mn+mx) / 510.0f;

			if (lightness <= 0.5f)
			{
				saturation = (mx-mn) / (mn+mx); 
			}
			else
			{
				saturation = (mx-mn) / (510.0f-mn-mx);
			}

			switch (major)
			{
			case Red:
				hue = (green-blue) * 60 / (mx-mn) + 360; 
				break;

			case Green: 
				hue = (blue-red) * 60  / (mx-mn) + 120;  
				break;

			case Blue : hue = (red-green) * 60 / (mx-mn) + 240;
				break;
			}

			if (hue >= 360.0f)
			{
				hue = hue - 360.0f;
			}
		}
	}

	unsigned char Value(float m1, float m2, float h)
	{
		while (h >= 360.0f) 
		{
			h -= 360.0f;
		}
		while (h < 0) 
		{
			h += 360.0f;
		}

		if (h < 60.0f) 
		{
			m1 = m1 + (m2 - m1) * h / 60;   
		}
		else if (h < 180.0f) 
		{
			m1 = m2;
		}
		else if (h < 240.0f) 
		{
			m1 = m1 + (m2 - m1) * (240 - h) / 60;  
		}

		return (unsigned char)(m1 * 255);
	}


	void SDIBHelper::HSLtoRGB(const float &hue, const float &_saturation, const float &_lightness,
		                      BYTE &red, BYTE &green, BYTE &blue)
	{
		float lightness = min(1.0f, _lightness);
		float saturation = min(1.0f, _saturation);

		if (saturation == 0)
		{
			red = green = blue = (unsigned char) (lightness*255);
		}
		else
		{
			float m1, m2;

			if (lightness <= 0.5f)
			{
				m2 = lightness + lightness * saturation;  
			}
			else       
			{
				m2 = lightness + saturation - lightness * saturation;
			}

			m1 = 2 * lightness - m2;   

			red   = Value(m1, m2, hue + 120);   
			green = Value(m1, m2, hue);
			blue  = Value(m1, m2, hue - 120);
		}
	}

}//namespace SOUI