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
		
		static bool GrayImage(IBitmap * pBmp); 

	public:// 辅助
		static void RGBtoHSL(BYTE &R, BYTE &G, BYTE &B, float &H, float &S, float &L);
		static void HSLtoRGB(float &H, float &S, float &L, BYTE &R, BYTE &G, BYTE &B);
    };

}//namespace SOUI