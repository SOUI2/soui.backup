/// 
/// 实现Skia的图像操作
///
/// 文件名：SkBitmapOp.h
/// 作  者：汪荣
/// 日  期：2014-03-15
///  
/// ===============================================================
///

#ifndef _UISKBITMAPOP_H_
#define _UISKBITMAPOP_H_

#include "SkCanvas.h"
#include "SkBitmap.h"

class SkBitmapOp
{
public:

	SkBitmapOp();

	/// <summary>
	///  创建边缘模糊图像
	/// </summary>
	/// <param name="bitmap">创建的图像(作为结果返回)</param>
	/// <param name="w">图像宽度</param>
	/// <param name="h">图像高度</param>
	/// <param name="radius">模糊半径</param>
	/// <returns>成功返回true;否则false</returns>
	static bool CreateBlurBitmap(SkBitmap& bitmap, int w, int h, SkScalar radius);
};

#endif