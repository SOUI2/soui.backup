#include "drawtext-skia.h"


static size_t breakTextEx(const SkPaint *pPaint, const wchar_t* textD, size_t length, SkScalar maxWidth,
                          SkScalar* measuredWidth) 
{
    size_t nLineLen=pPaint->breakText(textD,length*sizeof(wchar_t),maxWidth,measuredWidth,SkPaint::kForward_TextBufferDirection);
    if(nLineLen==0) return 0;
    nLineLen/=sizeof(wchar_t);

    const wchar_t * p=textD;
    for(size_t i=0;i<nLineLen;i++, p++)
    {
        if(*p == L'\r')
        {
            if(i<nLineLen-1 && p[1]==L'\n') return i+2;
            else return i;
        }else if(*p == L'\n')
        {
            return i+1;
        }
    }
    return nLineLen;
}


int CountLines(const wchar_t *text, int len, const SkPaint& paint, SkScalar width)
{
    int   count = 0;
    const wchar_t* stop = text + len;

    if (width <=0) return 0;
    do {
         count += 1;
         size_t line_len = breakTextEx(&paint,text, stop - text, width,0);
         text = text + line_len;
    } while (text < stop);
    return count;
}

void DrawText_Skia(SkCanvas* canvas,const wchar_t *text,int len,SkRect box,const SkPaint& paint,int vAlign,int xFlag)
{
	if(len<0)	len = wcslen(text);
    if(len == 0) return;

    const wchar_t* textStop = text + len;
    float maxWidth = (float)(box.width()+0.5);
	if(maxWidth <= 0) return;

	bool eeFlag = (xFlag & 0x01) != 0;
	bool wbFlag = (xFlag & 0x02) != 0;
	bool slFlag = (xFlag & 0x04) != 0;
	bool ecFlag = (xFlag & 0x08) != 0; //½ûÖ¹°ëÐÐ
	bool peFlag = (xFlag & 0x10) != 0;
	bool shadow = false;

	if(slFlag) wbFlag = 0;
	if(!slFlag) vAlign = 0;

    float  x, y;
	float  fontHeight,textHeight,height;
    SkPaint::FontMetrics metrics;

    switch (paint.getTextAlign()) 
	{
    case SkPaint::kLeft_Align:
       x = 0;
       break;
    case SkPaint::kCenter_Align:
       x = SkScalarHalf(maxWidth);
       break;
    default:
       x = maxWidth-1;
       break;
    }
    x += box.fLeft;

    paint.getFontMetrics(&metrics);
	fontHeight = metrics.fDescent-metrics.fAscent;
	textHeight = fontHeight;
    height = box.height();
	float lineSpan = metrics.fBottom-metrics.fTop;

    switch (vAlign) 
	{
    case 0:
	   y = 0;
       break;
	case 1:
       y = (float)((height - textHeight)/2-0.5);
       break;
    default:
       y = height - textHeight;
       break;
    }
	y = y + box.fTop - metrics.fAscent;

    while(text<textStop)
    {
        len = breakTextEx(&paint,text, textStop-text, maxWidth, NULL);
        if (y + metrics.fDescent + metrics.fLeading > 0)
        {
            canvas->drawText(text,len*sizeof(wchar_t), x,y, paint);
        }
        text += len;
        y += lineSpan;
        if (y + metrics.fAscent >= box.fBottom) break;
    }
}

