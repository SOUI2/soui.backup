#pragma once

#include <unknown/obj-ref-i.h>
#include "imgdecoder-i.h"

namespace SOUI
{
    struct IBrush;
    struct IPen;
    struct IFont;
    struct IBitmap;
    struct IRegion;
    struct IRenderTarget;
    struct IRenderFactory;

    /**
    * @struct     IRenderFactory
    * @brief      RenderFactory对象
    * 
    * Describe
    */
    struct IRenderFactory : public IObjRef
    {
        virtual IImgDecoderFactory * GetImgDecoderFactory()=0;
        virtual void SetImgDecoderFactory(IImgDecoderFactory *pImgDecoderFac)=0;
        virtual BOOL CreateRenderTarget(IRenderTarget ** ppRenderTarget,int nWid,int nHei)=0;
        virtual BOOL CreateFont(IFont ** ppFont, const LOGFONT &lf)=0;
        virtual BOOL CreateBitmap(IBitmap ** ppBitmap)=0;
        virtual BOOL CreateRegion(IRegion **ppRgn)=0;
    };

    enum OBJTYPE
    {
        OT_NULL=0,
        OT_PEN,
        OT_BRUSH,
        OT_BITMAP,
        OT_FONT,
        OT_RGN,
    };

    /**
    * @struct     IRenderObj
    * @brief      渲染对象基类
    * 
    * Describe    所有渲染对象全部使用引用计数管理生命周期
    */
    struct IRenderObj : public IObjRef
    {
        /**
         * ObjectType
         * @brief    查询对象类型
         * @return   const UINT 
         * Describe  
         */    
        virtual const OBJTYPE ObjectType() const = 0;

        /**
         * GetRenderFactory
         * @brief    获得创建该渲染对象的类厂
         * @return   IRenderFactory * -- 类厂 
         * Describe  
         */    
        virtual IRenderFactory * GetRenderFactory() const = 0;
    };


    /**
    * @struct     IBrush
    * @brief      画刷对象
    * 
    * Describe    
    */
    struct IBrush : public IRenderObj
    {
        virtual const OBJTYPE ObjectType() const
        {
            return OT_BRUSH;
        }
    };

    /**
    * @struct     IPen
    * @brief      画笔对象
    * 
    * Describe    
    */
    struct IPen: public IRenderObj
    {
        virtual const OBJTYPE ObjectType() const
        {
            return OT_PEN;
        }
    };

    /**
    * @struct     IBitmap
    * @brief      位图对象
    * 
    * Describe    
    */
    struct IBitmap: public IRenderObj
    {
        virtual const OBJTYPE ObjectType() const
        {
            return OT_BITMAP;
        }
        /**
         * Init
         * @brief    从32位的位图数据初始化IBitmap
         * @param    int nWid --  图片宽度
         * @param    int nHei --  图片高度
         * @param    const LPVOID pBits --  位图数据,以ARGB形式存储
         * @return   HRESULT -- 成功返回S_OK,失败返回错误代码
         * Describe  
         */    
        virtual HRESULT Init(int nWid,int nHei,const LPVOID pBits=NULL)=0;

        /**
         * Init
         * @brief    从IImgFrame初始化位图
         * @param    IImgFrame * pImgFrame --  IImgFrame指针
         * @return   HRESULT -- 成功返回S_OK,失败返回错误代码
         * Describe  
         */    
        virtual HRESULT Init(IImgFrame *pImgFrame) =0;

        /**
         * LoadFromFile
         * @brief    从文件中加载位图
         * @param    LPCTSTR pszFileName --  文件名
         * @return   HRESULT -- 成功返回S_OK,失败返回错误代码
         * Describe  
         */    
        virtual HRESULT LoadFromFile(LPCTSTR pszFileName)=0;

        /**
         * LoadFromMemory
         * @brief    从内存中加载位图
         * @param    LPBYTE pBuf --  内存地址
         * @param    size_t szLen --  内存大小
         * @return   HRESULT -- 成功返回S_OK,失败返回错误代码
         * Describe  
         */    
        virtual HRESULT LoadFromMemory(LPBYTE pBuf,size_t szLen)=0;

        /**
         * Width
         * @brief    获得图片宽度
         * @return   UINT -- 图片宽度
         * Describe  
         */    
        virtual UINT    Width() =0;

        /**
         * Height
         * @brief    获得图片高度
         * @return   UINT -- 图片高度
         * Describe  
         */    
        virtual UINT    Height() =0;

        /**
         * Size
         * @brief    获得图片高度及宽度
         * @return   SIZE -- 图片高度及宽度
         * Describe  
         */    
        virtual SIZE    Size() =0;

        /**
         * LockPixelBits
         * @brief    锁定位图的数据
         * @return   LPVOID -- 位图数据地址
         * Describe  
         */    
        virtual LPVOID  LockPixelBits() =0;

        /**
         * UnlockPixelBits
         * @brief    解除数据锁定
         * @param    LPVOID --  由LockPixelBits返回的位图数据地址
         * @return   void
         * Describe  与LockPixelBits配对使用
         */    
        virtual void    UnlockPixelBits(LPVOID) =0;
        
         /**
         * UnlockPixelBits
         * @brief    Save
         * @param    LPCWSTR pszFileName --  File name
         * @param    const LPVOID *pFormat --  image format
         * @return   HRESULT -- S_OK: succeed
         * Describe  
         */    
        virtual HRESULT Save(LPCWSTR pszFileName,const LPVOID pFormat)
        {
            return GetRenderFactory()->GetImgDecoderFactory()->SaveImage(this,pszFileName,pFormat);
        }
    };

    /**
    * @struct     IFont
    * @brief      字体对象
    * 
    * Describe    
    */
    struct IFont : public IRenderObj
    {
        virtual const OBJTYPE ObjectType() const
        {
            return OT_FONT;
        }

        /**
         * LogFont
         * @brief    获得字体的LOGFONT
         * @return   const LOGFONT * -- 包含字体信息的LOGFONT*
         * Describe  
         */    
        virtual const LOGFONT * LogFont() const =0;

        /**
         * FamilyName
         * @brief    获取字体名
         * @return   LPCTSTR -- 字体名
         * Describe  
         */    
        virtual LPCTSTR FamilyName()=0;

        /**
         * TextSize
         * @brief    获取字体大小
         * @return   int -- 字体大小
         * Describe  
         */    
        virtual int TextSize()=0;

        /**
         * IsBold
         * @brief    查询是否为粗体
         * @return   BOOL -- true为粗体，false正常
         * Describe  
         */    
        virtual BOOL IsBold()=0;

        /**
         * IsUnderline
         * @brief    查询字体下划线状态
         * @return   BOOL -- true有下划线，false正常
         * Describe  
         */    
        virtual BOOL IsUnderline()=0;

        /**
         * IsItalic
         * @brief    查询字体的斜体状态
         * @return   BOOL -- true为斜体，false正常
         * Describe  
         */    
        virtual BOOL IsItalic()=0;

        /**
         * StrikeOut
         * @brief    查询字体的删除线状态
         * @return   BOOL -- true有删除线
         * Describe  
         */    
        virtual BOOL IsStrikeOut() =0;
    };

    /**
    * @struct     IRegion
    * @brief      Region对象
    * 
    * Describe    
    */
    struct IRegion : public IRenderObj
    {
        virtual const OBJTYPE ObjectType() const
        {
            return OT_RGN;
        }

        /**
         * CombineRect
         * @brief    将一个矩形与this组合
         * @param    LPCRECT lprect --  要组合的矩形
         * @param    int nCombineMode --  组合模式
         * @return   void
         * Describe  组合模式同Win32 API CombineRect
         */    
        virtual void CombineRect(LPCRECT lprect,int nCombineMode )=0;

        /**
         * PtInRegion
         * @brief    检测一个点是否在region范围内
         * @param    POINT pt --  被检测的点
         * @return   BOOL -- true在region内
         * Describe  
         */    
        virtual BOOL PtInRegion(POINT pt)=0;

        /**
         * RectInRegion
         * @brief    检测一个矩形是否与this相交
         * @param    LPCRECT lprect --  被检测的的矩形。
         * @return   BOOL -- true在region内
         * Describe  
         */    
        virtual BOOL RectInRegion(LPCRECT lprect)=0;

        /**
         * GetRgnBox
         * @brief    获得this的外包矩形
         * @param [out] LPRECT lprect --  外包矩形
         * @return   void  
         * Describe  
         */    
        virtual void GetRgnBox(LPRECT lprect)=0;

        /**
         * IsEmpty
         * @brief    查询region是否为空
         * @return   BOOL -- true为空
         * Describe  
         */    
        virtual BOOL IsEmpty()=0;

        /**
         * Offset
         * @brief    将this整体平移
         * @param    POINT pt --  平移在x,y方向的的位移
         * @return   void 
         * Describe  
         */    
        virtual void Offset(POINT pt)=0;

        /**
         * Clear
         * @brief    清空region
         * @return   void
         * Describe  
         */    
        virtual void Clear()=0;
    };

    enum EXPEND_MODE
    {
        EM_NULL=0,      //不扩大
        EM_STRETCH,     //拉伸
        EM_TILE,        //平铺
    };

    /**
    * @struct     IRenderTarget
    * @brief      RenderTarget对象
    * 
    * Describe    实现各位渲染接口并创建设备相关资源
    */
    struct IRenderTarget: public IObjRef
    {
        virtual HRESULT CreateCompatibleRenderTarget(SIZE szTarget,IRenderTarget **ppRenderTarget)=0;
        virtual HRESULT CreatePen(int iStyle,COLORREF cr,int cWidth,IPen ** ppPen)=0;
        virtual HRESULT CreateSolidColorBrush(COLORREF cr,IBrush ** ppBrush)=0;
        virtual HRESULT CreateBitmapBrush( IBitmap *pBmp,IBrush ** ppBrush )=0;

        virtual HRESULT Resize(SIZE sz)=0;

        virtual HRESULT OffsetViewportOrg(int xOff, int yOff, LPPOINT lpPoint=NULL)=0;
        virtual HRESULT GetViewportOrg(LPPOINT lpPoint) =0;
        virtual HRESULT SetViewportOrg(POINT pt) =0;

        virtual HRESULT PushClipRect(LPCRECT pRect,UINT mode=RGN_AND)=0;
        virtual HRESULT PushClipRegion(IRegion *pRegion,UINT mode=RGN_AND)=0;
        virtual HRESULT PopClip()=0;

        virtual HRESULT ExcludeClipRect(LPCRECT pRc)=0;
        virtual HRESULT IntersectClipRect(LPCRECT pRc)=0;

        virtual HRESULT SaveClip(int *pnState)=0;
        virtual HRESULT RestoreClip(int nState=-1)=0;

        virtual HRESULT GetClipRegion(IRegion **ppRegion)=0;
        virtual HRESULT GetClipBox(LPRECT prc)=0;

        virtual HRESULT DrawText(LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat)=0;
        virtual HRESULT MeasureText(LPCTSTR pszText,int cchLen, SIZE *psz) =0;
        virtual HRESULT TextOut(int x,int y, LPCTSTR lpszString,int nCount) =0;

        virtual HRESULT DrawRectangle(LPCRECT pRect)=0;
        virtual HRESULT FillRectangle(LPCRECT pRect)=0;
        virtual HRESULT FillSolidRect(LPCRECT pRect,COLORREF cr)=0;
        virtual HRESULT DrawRoundRect(LPCRECT pRect,POINT pt)=0;
        virtual HRESULT FillRoundRect(LPCRECT pRect,POINT pt)=0;
        virtual HRESULT ClearRect(LPCRECT pRect,COLORREF cr)=0;
        virtual HRESULT DrawEllipse(LPCRECT pRect)=0;
        virtual HRESULT FillEllipse(LPCRECT pRect)=0;

        virtual HRESULT DrawArc(LPCRECT pRect,float startAngle,float sweepAngle,bool useCenter) =0;
        virtual HRESULT FillArc(LPCRECT pRect,float startAngle,float sweepAngle) =0;

        virtual HRESULT DrawLines(LPPOINT pPt,size_t nCount) =0;
        virtual HRESULT GradientFill(LPCRECT pRect,BOOL bVert,COLORREF crBegin,COLORREF crEnd,BYTE byAlpha=0xFF)=0;
        virtual HRESULT GradientFillEx( LPCRECT pRect,const POINT* pts,COLORREF *colors,float *pos,int nCount,BYTE byAlpha=0xFF )=0;

        virtual HRESULT DrawIconEx(int xLeft, int yTop, HICON hIcon, int cxWidth,int cyWidth,UINT diFlags)=0;
        virtual HRESULT DrawBitmap(LPCRECT pRcDest,IBitmap *pBitmap,int xSrc,int ySrc,BYTE byAlpha=0xFF)=0;
        virtual HRESULT DrawBitmapEx(LPCRECT pRcDest,IBitmap *pBitmap,LPCRECT pRcSrc,EXPEND_MODE expendMode, BYTE byAlpha=0xFF)=0;
        virtual HRESULT DrawBitmap9Patch(LPCRECT pRcDest,IBitmap *pBitmap,LPCRECT pRcSrc,LPCRECT pRcSourMargin,EXPEND_MODE expendMode,BYTE byAlpha=0xFF) =0;
        virtual HRESULT BitBlt(LPCRECT pRcDest,IRenderTarget *pRTSour,int xSrc,int ySrc,DWORD dwRop=SRCCOPY)=0;
        virtual HRESULT AlphaBlend(LPCRECT pRcDest,IRenderTarget *pRTSrc,LPCRECT pRcSrc,BYTE byAlpha) =0;
        virtual IRenderObj * GetCurrentObject(OBJTYPE uType) =0;
        //将指定的RenderObj恢复为默认状态
        virtual HRESULT SelectDefaultObject(OBJTYPE objType, IRenderObj ** pOldObj = NULL) =0;
        virtual HRESULT SelectObject(IRenderObj *pObj,IRenderObj ** pOldObj = NULL) =0;
        virtual COLORREF GetTextColor() =0;
        virtual COLORREF SetTextColor(COLORREF color)=0;

        //两个兼容GDI操作的接口
        virtual HDC GetDC(UINT uFlag=0)=0;
        virtual void ReleaseDC(HDC hdc) =0;
        
        /**
         * QueryInterface
         * @brief    提供接口扩展的用的方法
         * @param    REFGUID iid --  待扩展接口ID
         * @param    IObjRef * * ppObj --  接口输出
         * @return   HRESULT -- 成功返回S_OK
         *
         * Describe  具体能获取什么接口依赖于不同的渲染引擎
         */
        virtual HRESULT QueryInterface(REFGUID iid,IObjRef ** ppObj) =0;
    };


}//end of namespace SOUI
