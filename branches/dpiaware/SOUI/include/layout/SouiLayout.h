#pragma once
#include "interface/slayout-i.h"
#include <sobject/sobject-state-impl.hpp>
#include <helper/SplitString.h>

namespace SOUI{

#define POSFLAG_REFCENTER      '|'        //参考父窗口中心
#define POSFLAG_REFPREV_NEAR   '['        //参考前一个兄弟窗口与自己近的边
#define POSFLAG_REFNEXT_NEAR   ']'        //参考下一个兄弟窗口与自己近的边
#define POSFLAG_REFPREV_FAR    '{'        //参考前一个兄弟窗口与自己远的边
#define POSFLAG_REFNEXT_FAR    '}'        //参考下一个兄弟窗口与自己远的边
#define POSFLAG_PERCENT        '%'        //采用在父窗口的百分比定义坐标
#define POSFLAG_SIZE           '@'        //在pos属性中定义窗口的size，只在在定义x2,y2时有效

	enum SWNDPOSMASK//坐标属性
	{
		// Specify by "width" attribute
		SizeX_Mask          = 0x000fUL,
		SizeX_Specify       = 0x0001UL, // width > 0
		SizeX_FitContent    = 0x0002UL, // width <= 0
		SizeX_FitParent     = 0x0004UL, // width = "full" default

		// Specify by "height" attribute
		SizeY_Mask          = 0x00f0UL,
		SizeY_Specify       = 0x0010UL, // height > 0
		SizeY_FitContent    = 0x0020UL, // height <= 0 default
		SizeY_FitParent     = 0x0040UL, // height = "full" default
	};

	//坐标类型
	enum PIT{

		PIT_NULL=0,        //无效定义
		PIT_NORMAL,        //锚点坐标
		PIT_CENTER,        //参考父窗口中心点,以"|"开始
		PIT_PERCENT,       //指定在父窗口坐标的中的百分比,以"%"开始
		PIT_PREV_NEAR,     //参考前一个兄弟窗口与自己近的边,以"["开始
		PIT_NEXT_NEAR,     //参考下一个兄弟窗口与自己近的边,以"]"开始
		PIT_PREV_FAR,      //参考前一个兄弟窗口与自己远的边,以"{"开始
		PIT_NEXT_FAR,      //参考下一个兄弟窗口与自己远的边,以"}"开始
		PIT_SIZE,          //指定窗口的宽或者高,以"@"开始
		PIT_SIB_LEFT=10,       //兄弟结点的left,用于X
		PIT_SIB_TOP=10,        //兄弟结点的top，与left相同，用于Y
		PIT_SIB_RIGHT=11,      //兄弟结点的right,用于X 
		PIT_SIB_BOTTOM=11,      //兄弟结点的bottom,与right相同,用于Y 
	};

	struct POSITION_ITEM
	{
		PIT     pit;        /**<坐标类型 */
		int     nRefID;     /**<根据ID引用兄弟窗口时使用的ID,-1代表不参考特定ID的兄弟,使用ID引用的格式为"sib.left@200:10"类似的格式 */
		char    cMinus;     /**<定义的值包含"-", 由于-0不能直接做nPos表示，需要一个单独的标志位 */
		float   nPos;       /**<坐标值*/
	};


	enum POSINDEX
	{
		PI_LEFT = 0,
		PI_TOP,
		PI_RIGHT,
		PI_BOTTOM,
	};

	class SouiLayoutParam: public SObjectImpl<TObjRefImpl<ILayoutParam>>
	{
		SOUI_CLASS_NAME(SouiLayoutParam,L"SouiLayoutParam")

		friend class SouiLayout;
	public:
		virtual bool IsMatchParent(ORIENTATION orientation) const;

		virtual bool IsSpecifiedSize(ORIENTATION orientation) const;

		virtual bool IsWrapContent(ORIENTATION orientation) const;

		virtual int GetSpecifiedSize(ORIENTATION orientation) const;

		virtual void Clear();

		virtual void SetMatchParent(ORIENTATION orientation);

		virtual void SetWrapContent(ORIENTATION orientation);

		virtual void SetSpecifiedSize(ORIENTATION orientation, int nSize);

	public:
		bool IsOffsetRequired(ORIENTATION orientation) const;
        int  GetExtraSize(ORIENTATION orientation) const;
	protected:
		HRESULT OnAttrWidth(const SStringW & strValue,BOOL bLoading);

		HRESULT OnAttrHeight(const SStringW & strValue,BOOL bLoading);

		HRESULT OnAttrSize(const SStringW & strValue,BOOL bLoading);

		HRESULT OnAttrPos(const SStringW & strValue,BOOL bLoading);

		HRESULT OnAttrOffset(const SStringW & strValue,BOOL bLoading);

		SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"width",OnAttrWidth)
            ATTR_CUSTOM(L"height",OnAttrHeight)
			ATTR_CUSTOM(L"pos",OnAttrPos)
			ATTR_CUSTOM(L"size",OnAttrSize)
			ATTR_CUSTOM(L"offset",OnAttrOffset)
        SOUI_ATTRS_BREAK()

    protected:
        //将字符串描述的坐标转换成POSITION_ITEM
        BOOL StrPos2ItemPos(const SStringW &strPos,POSITION_ITEM & posItem);

        //解析在pos中定义的前两个位置
        BOOL ParsePosition12(const SStringW & pos1, const SStringW &pos2);

        //解析在pos中定义的后两个位置
        BOOL ParsePosition34(const SStringW & pos3, const SStringW &pos4);


	protected:
        int  nCount;                /**< 定义的坐标个数 */
        POSITION_ITEM pos[4];       /**< 由pos属性定义的值, nCount >0 时有效*/

		float fOffsetX,fOffsetY;    /**< 窗口坐标偏移量, x += fOffsetX * width, y += fOffsetY * height  */

		int  m_width;        /**<使用width属性定义的宽 nCount==0 时有效*/
		int  m_height;       /**<使用height属性定义的高 nCount==0 时有效*/
	};

	class SouiLayout: public SObjectImpl<TObjRefImpl<ILayout>>
	{
		SOUI_CLASS_NAME(SouiLayout,L"SouiLayout")

	public:
		SouiLayout(void);
		~SouiLayout(void);

		static HRESULT CreateLayout(IObjRef ** ppObj);

		static HRESULT CreateLayoutParam(IObjRef ** ppObj);

        virtual bool IsParamAcceptable(ILayoutParam *pLayoutParam) const;

        virtual void LayoutChildren(SWindow * pParent);

        virtual ILayoutParam * CreateLayoutParam() const;

        virtual CSize MeasureChildren(SWindow * pParent,int nWidth,int nHeight) const;
    protected:
        struct WndPos{
            SWindow *pWnd;
            CRect    rc;
			bool     bWaitOffsetX;
			bool	 bWaitOffsetY;
        };

        void CalcPositionEx(SList<WndPos> *pListChildren,int nWidth,int nHeight) const;
        int CalcPostion(SList<WndPos> *pListChildren,int nWidth,int nHeight) const;

		int PositionItem2Value(SList<WndPos> *pLstChilds,SPOSITION position,const POSITION_ITEM &pos , int nMax,BOOL bX) const;
        
        int CalcChildLeft(SWindow *pWindow,SouiLayoutParam *pParam);
        int CalcChildRight(SWindow *pWindow,SouiLayoutParam *pParam);
        int CalcChildTop(SWindow *pWindow,SouiLayoutParam *pParam);
        int CalcChildBottom(SWindow *pWindow,SouiLayoutParam *pParam);


        BOOL IsWaitingPos( int nPos ) const;
		SWindow * GetRefSibling(SWindow *pCurWnd,int uCode);
        CRect GetWindowLayoutRect(SWindow *pWindow);
    };


}
