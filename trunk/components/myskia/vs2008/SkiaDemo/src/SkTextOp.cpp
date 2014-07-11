
#include <tchar.h>
#include "SkTextOp.h"

int SkTextOp::ComputeTextSize(int size)
{
	HDC hdc = ::GetDC(NULL);
    int iCaps = ::GetDeviceCaps(hdc,  LOGPIXELSY);
    int iSize = MulDiv(size, iCaps, 72);
    ::ReleaseDC(NULL, hdc);

	return iSize;
}

int SkTextOp::ComputeTextLineCount(SkPaint& paint, const wchar_t* text, int len)
{
	int iLineCount = 0;
	int iStartPos = 0;
	int i = 0;

	for (i = 0; i < len; ++i)
	{
		// 碰到换行符，直接断行
		if (text[i] == '\n')
		{
			iStartPos = i;
			++iLineCount;
		}
		else if (text[i] == '\r')
		{
			if (i < len - 1 && text[i + 1] == '\n')
			{
				++i;
			}

			iStartPos = i;
			++iLineCount;
		}
	}

	if (iStartPos < len)
	{
		++iLineCount;
	}

	return iLineCount;
}

int SkTextOp::ComputeWrapTextLineCount(SkPaint& paint, SkScalar w, const wchar_t* text, int len)
{
	// 记录上一个字符位置索引
	int iStartPos = 0;
	int iLineCount = 0;
	int i = 0;

	for (i = 0; i < len; ++i)
	{
		// 碰到换行符，直接断行
		if (text[i] == '\n')
		{
			iLineCount += ComputeWrapTextLines(paint, w, text + iStartPos, i - iStartPos) + 1;
			iStartPos = i + 1;
		}
		else if (text[i] == '\r')
		{
			iLineCount += ComputeWrapTextLines(paint, w, text + iStartPos, i - iStartPos) + 1;

			if (i < len - 1 && text[i + 1] == '\n')
			{
				++i;
			}

			iStartPos = i + 1;
		}
	}

	if (iStartPos < len)
	{
		iLineCount += ComputeWrapTextLines(paint, w, text + iStartPos, len - iStartPos) + 1;
	}

	return iLineCount;
}

int SkTextOp::ComputeWrapTextLines(SkPaint& paint, SkScalar w, const wchar_t* text, int len)
{
	int iLineCount = 0;
	len *= 2;

	if (len > 0)
	{
		int iCurrTextPos = 0;
		int iTextPos = paint.breakText(text, len, w);

		if (iTextPos % 2 != 0)
		{
			iTextPos -= 1;
		}

		iTextPos = iTextPos  /  2;

		while (iTextPos > 0)
		{
			iCurrTextPos += iTextPos ;
			iTextPos = paint.breakText(text + iCurrTextPos, len - iCurrTextPos * 2, w);

			if (iTextPos % 2 != 0)
			{
				iTextPos -= 1;
			}

			iTextPos = iTextPos  /  2;

			if (iTextPos > 0)
			{
				++iLineCount;
			}
		}
	}

	return iLineCount;
}

//=================================================================

void SkTextOp::DrawLimitText(SkCanvas* canvas, SkPaint& paint, SkScalar x, SkScalar w, const wchar_t* text, int len, SkScalar lineSpace, SkPoint& offset)
{
	len *= 2;
	offset.fX = 0;

	if (len > 0)
	{
		int iCurrTextPos = 0;
		int iTextPos = paint.breakText(text, len, w, &(offset.fX));

		if (iTextPos % 2 != 0)
		{
			iTextPos -= 1;
		}

		iTextPos = iTextPos  /  2;

		while (iTextPos > 0)
		{
			canvas->drawText(text + iCurrTextPos,  iTextPos * 2, x,  offset.fY,  paint);

			iCurrTextPos += iTextPos ;
			iTextPos = paint.breakText(text + iCurrTextPos, len - iCurrTextPos * 2, w, &(offset.fX));

			if (iTextPos % 2 != 0)
			{
				iTextPos -= 1;
			}

			iTextPos = iTextPos  /  2;

			if (iTextPos > 0)
			{
				offset.fY += lineSpace;
			}
		}
	}

	offset.fX += x;
}

void SkTextOp::CoereYCor(SkPaint& paint, SkScalar& y)
{
	SkPaint::FontMetrics fm;
	paint.getFontMetrics(&fm);
	y += fm.fDescent - fm.fAscent;
}

SkScalar SkTextOp::DrawSingle(SkCanvas* canvas, SkPaint& paint, SkScalar x, SkScalar y, SkScalar w, SkScalar h, const wchar_t* text, int len)
{
	SkScalar measureSize = 0;
	len *= 2;

	if (len > 0)
	{
		int iTextPos = paint.breakText(text, len, w, &measureSize);
		canvas->drawText(text, iTextPos,  x,  y,  paint);
	}

	return (x + measureSize);
}

SkScalar SkTextOp::DrawSingleText(SkCanvas* canvas, SkPaint& paint,const SkRect& rect, const wchar_t* text, int len)
{
	SkScalar pos = 0;

	pos = DrawSingle(canvas, paint, rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, text, len);

	return pos;
}

SkPoint SkTextOp::DrawLinesText(SkCanvas* canvas, SkPaint& paint, const SkRect& rect, const wchar_t* text, int len)
{
	SkPaint::FontMetrics fm;
	SkScalar lineSpace = paint.getFontMetrics(&fm);

	// 记录上一个字符位置索引
	int iStartPos = 0;
	int i = 0;
	SkPoint offset = SkPoint::Make(rect.fLeft, rect.fTop);

	SkScalar w = rect.width();
	SkScalar h = rect.height();
	SkScalar y = rect.fTop + fm.fDescent - fm.fAscent;
	SkScalar iStartY = rect.fTop;

	canvas->save();
	canvas->clipRect(rect);

	for (i = 0; i < len; ++i)
	{
		// 碰到换行符，直接断行
		if (text[i] == '\n')
		{
			offset.fX = DrawSingle(canvas, paint, rect.fLeft, y, w, h, text + iStartPos, i - iStartPos);
			y += lineSpace;
			iStartPos = i + 1;
		}
		else if (text[i] == '\r')
		{
			offset.fX = DrawSingle(canvas, paint, rect.fLeft, y, w, h, text + iStartPos, i - iStartPos);
			y += lineSpace;

			if (i < len - 1 && text[i + 1] == '\n')
			{
				++i;
			}

			iStartPos = i + 1;
		}

		//
		// 已经大于可见区域，直接退出
		//
		if (y - iStartY > h)
		{
			iStartPos = len;
			break;
		}
	}

	if (iStartPos < len)
	{
		offset.fX = DrawSingle(canvas, paint, rect.fLeft, y, w, h, text + iStartPos, len - iStartPos);
	}

	offset.fY = y - (fm.fDescent - fm.fAscent);

	canvas->restore();

	return offset;
}

SkPoint SkTextOp::DrawWrapText(SkCanvas* canvas, SkPaint& paint, const SkRect& rect, const wchar_t* text, int len)
{
	SkPaint::FontMetrics fm;
	SkScalar lineSpace = paint.getFontMetrics(&fm);

	// 记录上一个字符位置索引
	int iStartPos = 0;
	int i = 0;
	SkPoint offset = SkPoint::Make(rect.fLeft, rect.fTop + fm.fDescent - fm.fAscent);

	SkScalar w = rect.width();
	SkScalar h = rect.height();

	SkScalar iStartY = rect.fLeft;

	for (i = 0; i < len; ++i)
	{
		// 碰到换行符，直接断行
		if (text[i] == '\n')
		{
			DrawLimitText(canvas, paint, rect.fLeft, w, text + iStartPos, i - iStartPos, lineSpace, offset);
			offset.fY += lineSpace;
			iStartPos = i + 1;
		}
		else if (text[i] == '\r')
		{
			DrawLimitText(canvas, paint, rect.fLeft, w, text + iStartPos, i - iStartPos, lineSpace, offset);
			offset.fY += lineSpace;

			if (i < len - 1 && text[i + 1] == '\n')
			{
				++i;
			}

			iStartPos = i + 1;
		}

		//
		// 已经大于可见区域，直接退出
		//
		if (offset.fY - iStartY > h)
		{
			iStartPos = len;
			break;
		}
	}

	if (iStartPos < len)
	{
		DrawLimitText(canvas, paint, rect.fLeft, w, text + iStartPos, len - iStartPos, lineSpace, offset);
	}

	offset.fY -= fm.fDescent - fm.fAscent;

	return offset;
}

class A
{
public:

	A() {}

private:

	int i;
	friend class B;
};

class B
{
public:

	B() {}

private:

	class C
	{
	public:

		C() {}

		void SetValue(A* pA, int v)
		{
			pA->i = v;
		}
	};
};

//====================================================================

SkScalar SkTextOp::DrawEllipsisText(SkCanvas* canvas, SkPaint& paint, SkScalar x, SkScalar y, SkScalar w, const wchar_t* text, int len)
{
	SkPaint::FontMetrics fm;
	SkScalar lineSpace = paint.getFontMetrics(&fm);

	return InnerDrawEllipsisText(canvas, paint, x, y + fm.fDescent - fm.fAscent, w, text, len);
}

SkScalar SkTextOp::InnerDrawEllipsisText(SkCanvas* canvas, SkPaint& paint, SkScalar x, SkScalar y, SkScalar w, const wchar_t* text, int len)
{
	SkScalar iDrawSize = 0;
	const wchar_t* dotText = L"...";
	SkScalar dotSize = paint.measureText(dotText, 6);
	SkScalar realWid = w - dotSize;

	len = len * 2;

	if (realWid > 0)
	{
		SkScalar realMeasure = 0;
		int iTextPos = paint.breakText(text, len, w, &realMeasure);

		if (iTextPos % 2 != 0)
		{
			iTextPos -= 1;
		}

		//
		// 可视区域完全可以显示文本
		//
		if (iTextPos >= len)
		{
			iDrawSize = realMeasure;
			canvas->drawText(text, len, x, y, paint);
		}
		else if (realMeasure <= dotSize)
		{
			iDrawSize = DoDotTextOnly(canvas, paint, dotText, x, y, w, dotSize);
		}
		//
		// 文本实际长度大于给出的可视宽度
		//
		else
		{
			const int CharNum = 3;
			int i = 0;
			// 计算省略号所在位置
			SkScalar charWid[CharNum] = {0};
			SkScalar charSum = w - realMeasure;
			int charPos = iTextPos / 2 - CharNum;
			paint.getTextWidths(text + charPos, CharNum * 2, charWid);

			for (i = 1; i <= CharNum; ++i)
			{
				charSum += charWid[CharNum - i];
				if ((int)charSum >= dotSize)
				{
					break;
				}
			}

			iDrawSize = w - charSum;

			// 加上省略号
			canvas->drawText(text, iTextPos - i * 2, x, y, paint);
			canvas->drawText(dotText, 6, x + iDrawSize, y, paint);

			iDrawSize += dotSize;
		}
	}
	else
	{
		SkScalar txtSize = paint.measureText(text, len);
		if (txtSize <= w)
		{
			canvas->drawText(text, len, x, y, paint);
			iDrawSize = txtSize;
		}
		else
		{
			iDrawSize = DoDotTextOnly(canvas, paint, dotText, x, y, w, dotSize);
		}
	}

	return iDrawSize;
}

SkScalar SkTextOp::DrawPathEllipsisText(SkCanvas* canvas, SkPaint& paint, SkScalar x, SkScalar y, SkScalar w, const wchar_t* text, int len)
{
	SkPaint::FontMetrics fm;
	SkScalar lineSpace = paint.getFontMetrics(&fm);

	SkScalar iDrawSize = 0;
	const wchar_t* dotText = L"...";
	SkScalar dotSize = paint.measureText(dotText, 6);

	SkScalar realMeasure = 0;
	realMeasure = paint.measureText(text, len * 2);
	y += fm.fDescent - fm.fAscent;

	//
	// 文本实际长度小于给出的可视宽度
	// 
	if ((int)realMeasure <= w)
	{
		canvas->drawText(text, len * 2, x, y, paint);
	}
	else
	{
		int i = 0;
		int iLastCount = 0;
		int iLastSize = 0;
		int iFirstPos = -1;

		for (; i < len; ++i)
		{
			if (text[i] == _T('/') || text[i] == _T('\\'))
			{
				iFirstPos = i;
				break;
			}
		}

		// 不是路径格式字符串
		if (iFirstPos == -1)
		{
			return InnerDrawEllipsisText(canvas, paint, x, y, w, text, len);
		}

		for (i = len - 1; i >= 0; --i)
		{
			if (text[i] == _T('/') || text[i] == _T('\\'))
			{
				iLastCount = len - i;
				if (iLastCount > 0)
				{
					iLastSize = (int)paint.measureText(text + i + 1, iLastCount * 2);
				}
				--i;
				break;
			}
		}

		int iOldPos = i;
		int curSize = (int)realMeasure;

		for (; i >= 0; --i)
		{
			if (text[i] == _T('/') || text[i] == _T('\\'))
			{
				if (iFirstPos == i)
				{
					i = 0;
				}

				int iStart = i;
				int iSize = iOldPos - i;
				int iMeasureSize = 0;

				iMeasureSize = (int)paint.measureText(text + iStart, iSize * 2);

				if (curSize - iMeasureSize < w)
				{
					SkScalar endPos = InnerDrawEllipsisText(canvas, paint, x, y, w - iLastSize, text, iOldPos + 1);
					if (iLastCount > 0)
					{
						canvas->drawText(text + len - iLastCount, iLastCount * 2, endPos, y, paint);
					}
					return endPos + iLastSize;
				}

				curSize -= iMeasureSize;

				--i;
				iOldPos = i;
			}
		}

		if (w >= dotSize)
		{
			canvas->drawText(dotText, 6, x, y, paint);
			const wchar_t* strFile = text + len - iLastCount;
			int iTextPos = paint.breakText(strFile, iLastCount * 2, w - dotSize, &realMeasure);
			if (iTextPos > 0)
			{
				canvas->drawText(strFile, iTextPos, x + dotSize, y, paint);
			}

			realMeasure += dotSize;
		}
		else
		{
			realMeasure = DoDotTextOnly(canvas, paint, dotText, x, y, w, dotSize);
		}
	}

	return realMeasure;
}

SkScalar SkTextOp::DoDotTextOnly(SkCanvas* canvas, SkPaint& paint, const wchar_t* dotText, SkScalar x, SkScalar y, SkScalar w, SkScalar dotSize)
{
	SkScalar oneDot = dotSize / 3;
	SkScalar fPos = oneDot * 2;

	if (w >= fPos)
	{
		canvas->drawText(dotText, 4, x, y, paint);
	}
	else if (w >= oneDot)
	{
		fPos = oneDot;
		canvas->drawText(dotText, 2, x, y, paint);
	}
	else
	{
		fPos = 0;
	}

	return fPos;
}
