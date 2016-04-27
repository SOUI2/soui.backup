#pragma once

#include "interface/render-i.h"

namespace SOUI
{
	/// <summary>
	///		简洁版本，简版去掉了对8、16、24位的支持,
	/// </summary>
	class SOUI_EXP SDIBHelper
	{
    public:
		/// <summary>
		///     H:-180~180（度）,=0不作调整, S/L:0~200(建议，最大值可>200),=100不作调整
		/// </summary>
		/// <remarks>
		///     HSL变换,HSL操作建议是在非预乘像素阵列下进行，但为了更快的效率，目前采用预乘像素阵列
		///     HSL变换应该在整个像素阵列不再变化时开始做调整.不然原始像素阵列就不再准确
		/// </remarks>
		static bool AdjustHSL32(IBitmap * pBmp, int H, int S, int L);
		
		static bool AdjustHue(IBitmap *pBmp,COLORREF crRef);
		
        static bool AdjustHue(COLORREF & crTarget,COLORREF crRef);

		static bool GrayImage(IBitmap * pBmp); 
        
        //计算图片的平均色
        //IBitmap *pBmp:图片源
        //int nPercent:有效值百分比，90代表最高和最低5%的值会丢掉，不参与平均。
        //int int nBlockSize:分块大小, 每次计算一个块的颜色平均值。
        static COLORREF CalcAvarageColor(IBitmap *pBmp,int nPercent=90,int nBlockSize=5);
	public:// 辅助
		static void RGBtoHSL(const BYTE &R, const BYTE &G, const BYTE &B, float &H, float &S, float &L);
		static void HSLtoRGB(const float &H, const float &S, const float &L, BYTE &R, BYTE &G, BYTE &B);
    };

}//namespace SOUI