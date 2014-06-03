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

#define DT_ELLIPSIS (DT_PATH_ELLIPSIS|DT_END_ELLIPSIS|DT_WORD_ELLIPSIS)
#define CH_ELLIPSIS L"..."
 
class SkTextLayoutEx {
public:
    //not support for DT_PREFIXONLY
    void init(const wchar_t text[], size_t length,SkRect rc, const SkPaint &paint,UINT uFormat)
    {
        if(uFormat & DT_NOPREFIX)
        {
            fText.setCount(length);
            memcpy(fText.begin(),text,length*sizeof(wchar_t));
        }else
        {
            fPrefix.deleteAll();
            SkTDArray<wchar_t> tmp;
            tmp.setCount(length);
            memcpy(tmp.begin(),text,length*sizeof(wchar_t));
            for(int i=0;i<tmp.count();i++)
            {
                if(tmp[i]==L'&')
                {
                    tmp.remove(i,1);
                    if(i<tmp.count()-1) fPrefix.push(i);
                    if(i<tmp.count()-1 && tmp[i+1]==L'&') i++;  //skip the next "&"
                }
            }
            fText.copy(tmp);
        }
        
        fPaint=paint;
        fBounds=rc;
        
        buildLines();   
    }

    void draw(SkCanvas* canvas)
    {
        float  fontHeight,textHeight;
        SkPaint::FontMetrics metrics;
        const SkPaint paint=fPaint;
        
        paint.getFontMetrics(&metrics);
        fontHeight = metrics.fDescent-metrics.fAscent;
        textHeight = fontHeight;
        
        float lineSpan = metrics.fBottom-metrics.fTop;

        float  x;
        switch (paint.getTextAlign()) 
        {
        case SkPaint::kCenter_Align:
            x = SkScalarHalf(maxWidth);
            break;
        case SkPaint::kRight_Align:
            x = fBounds.width()-1;
            break;
        default://SkPaint::kLeft_Align:
            x = 0;
            break;
        }
        x += fBounds.fLeft;

        canvas->save();

        canvas->clipRect(fBounds);

        float height = fBounds.height();
        float y=fBounds.fTop - metrics.fAscent;
        if(fFormat & DT_SINGLELINE)
        {//单行显示
            if(fFormat & DT_VCENTER) 
            {
                y += (height - textHeight)/2.0f;
            }
            if(fFormat & DT_ELLIPSIS)
            {//只支持在行尾增加省略号
                drawLineWithEndEllipsis(canvas,x,y,0,fText.count(),fontHeight,fBounds.width());
            }else
            {
                drawLine(canvas,x,y,0,fText.count(),fontHeight);
            }
        }else
        {//多行显示
            int iLine = 0;
            while(iLine<fLines.count())
            {
                if(y + lineSpan + metrics.fAscent >= fBounds.fBottom) 
                    break;  //the last visible line
                drawLine(canvas,x,y,fLines[iLine],fLines[iLine+1],fontHeight);
                y += lineSpan;
                iLine ++;
            }
            if(iLine<fLines.count())
            {//draw the last visible line
                int iBegin=fLines[iLine];
                int iEnd = iLine<(fLines.count()-1)?fLines[iLine+1]:fText.count();
                if(fFormat & DT_ELLIPSIS)
                {//只支持在行尾增加省略号
                    drawLineWithEndEllipsis(canvas,x,y,iBegin,iEnd,fontHeight,fBounds.width());
                }else
                {
                    drawLine(canvas,x,y,iBegin,iEnd,fontHeight);
                }
                y += lineSpan;
            }
            
            fBounds.fBottom = fBounds.fTop + y; //获得真实的限制矩形，方便返回大小
        }
        canvas->restore();
    }
    
private:
    SkScalar drawLineWithEndEllipsis(SkCanvas *canvas, SkScalar x, SkScalar y, int iBegin,int iEnd,SkScalar fontHei,SkScalar maxWidth)
    {
        SkScalar widReq=fPaint.measureText(fText.begin()+iBegin,(iEnd-iBegin)*size_t(wchar_t));
        if(widReq<fBounds.width())
        {
            return drawLine(canvas,x,y,0,fText.count(),fontHei);
        }else
        {
            SkScalar fEllipsisWid=fPaint.measureText(CH_ELLIPSIS,sizeof(CH_ELLIPSIS)-sizeof(wchar_t));            SkScalar fWid=fEllipsisWid;            int i=0;            const wchar_t *text=fText.begin()+iBegin;
            while(i<(iEnd-iBegin))            {                fWid += fPaint.measureText(text,sizeof(wchar_t));                if(fWid > maxWidth) break;                i++;            }            drawLine(canvas,x,y,0,i,fontHei);            if((!fFormat & DT_CALCRECT)) canvas->drawText(CH_ELLIPSIS,sizeof(CH_ELLIPSIS)-sizeof(wchar_t),x+fWid-fEllipsisWid,y,fPaint);            return fWid;        }
    }
    
    SkScalar drawLine(SkCanvas *canvas, SkScalar x, SkScalar y, int iBegin,int iEnd,SkScalar fontHei)
    {
        const wchar_t *text=fText.begin()+iBegin;
                
        if((!fFormat & DT_CALCRECT))
        {
            canvas->drawText(text,(iEnd-iBegin)*sizeof(wchar_t),x,y,fPaint);
            int i=0;
            while(i<fPrefix.count())
            {
                if(fPrefix[i]>=iBegin)
                    break;
                i++;
            }
            while(i<fPrefix.count() && fPrefix[i]<iEnd)
            {
                SkScalar x1 = x + fPaint.measureText(text,(fPrefix[i]-iBegin)*sizeof(wchar_t));
                SkScalar x2 = x + fPaint.measureText(text,(fPrefix[i]-iBegin+1)*sizeof(wchar_t));
                canvas->drawLine(x1,y+fontHei,x2,y+fontHei,fPaint); //绘制下划线
                i++;
            }
        }
        return fPaint.measureText(text,(iEnd-iBegin)*sizeof(wchar_t));
    }
    
    void buildLines()
    {
        fLines.deleteAll();
        if (fBounds.width() <=0) return 0;
        
        if(fFormat & DT_SINGLELINE)
        {
            fLines.push(0);
        }else
        {
            const wchar_t *text = fText.begin();
            const wchar_t* stop = fText.begin() + fText.count();

            fLines.push(0);
            size_t lineHead=0;
            while(lineHead<fText.count())
            {
                fLines.push(lineHead);
                size_t line_len = breakTextEx(&fPaint,text, stop - text, fBounds.width(),0);
                text += line_len;
                lineHead += line_len;
            };
        }
    }
    
    SkTDArray<wchar_t> fText;   //文本内容
    SkTDArray<int>  fPrefix;    //前缀符索引
    SkTDArray<int> fLines;      //分行索引
    UINT            fFormat;    //显示标志
    SkRect          fBounds;    //限制矩形
    const SkPaint   fPaint;
};



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



void DrawText_Skia(SkCanvas* canvas,const wchar_t *text,int len,SkRect box,const SkPaint& paint,UINT uFormat)
{
	if(len<0)	len = wcslen(text);
    if(len == 0) return;

    const wchar_t* textStop = text + len;
    float maxWidth = box.width();
	if(maxWidth <= 0) return;

    float  fontHeight,textHeight,height;
    SkPaint::FontMetrics metrics;
    
    paint.getFontMetrics(&metrics);
    fontHeight = metrics.fDescent-metrics.fAscent;
    textHeight = fontHeight;
    height = box.height();
    float lineSpan = metrics.fBottom-metrics.fTop;

    float  x;
    switch (paint.getTextAlign()) 
    {
    case SkPaint::kCenter_Align:
        x = SkScalarHalf(maxWidth);
        break;
    case SkPaint::kRight_Align:
        x = maxWidth-1;
        break;
    default://SkPaint::kLeft_Align:
        x = 0;
        break;
    }
    x += box.fLeft;

    canvas->save();
    
    canvas->clipRect(box);
    
    float y=box.fTop - metrics.fAscent;
    if(uFormat & DT_SINGLELINE)
    {//单行显示
        if(uFormat & DT_VCENTER) 
        {
            y += (height - textHeight)/2.0f;
        }
        if(uFormat & DT_WORD_ELLIPSIS)
        {
            size_t lenVisible = paint.breakText(text,len*size_t(wchar_t),maxWidth,NULL);
            if(lenVisible==len)
            {
                canvas->drawText(text,len*sizeof(wchar_t),x,y,paint);
            }else
            {
                
            }
        }else
        {
            canvas->drawText(text,len*sizeof(wchar_t),x,y,paint);
        }
    }else
    {//多行显示
        int nLines=CountLines(text,len,paint,maxWidth);
        while(text<textStop)
        {
            size_t len = breakTextEx(&paint,text, textStop-text, maxWidth, NULL);
            if (y + metrics.fDescent + metrics.fLeading > 0)
            {
                canvas->drawText(text,len*sizeof(wchar_t), x,y, paint);
            }
            text += len;
            y += lineSpan;
            if (y + metrics.fAscent >= box.fBottom) break;
        }
    }
    canvas->restore();
}

